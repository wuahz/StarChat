#include "LogicSystem.h"
#include "HttpConnection.h"
#include "rpc/VerifyGrpcClient.h"
#include "rpc/StatusGrpcClient.h"
#include "redis/RedisManager.h"
#include "mysql/MysqlManager.h"

/**
 * @brief 解密密码
 * @param input
 * @return
 */
std::string xorString(const std::string &input) {
    std::string result = input;
    int length = static_cast<int>(result.size());
    unsigned char xor_code = length % 255;

    for (int i = 0; i < length; ++i) {
        result[i] = result[i] ^ xor_code;
    }
    return result; // UTF-8 字符串
}

LogicSystem::LogicSystem() {
    // 简单 GET 测试接口：回显参数
    RegisterGet("/get_test", [](const std::shared_ptr<HttpConnection>& connection) {
        beast::ostream(connection->response_.body()) << "receive get_test request\n";
        int i = 0;
        for (auto& elem : connection->get_params_) {
            beast::ostream(connection->response_.body())
                    << "param" << ++i << " key = " << elem.first
                    << ", value = " << elem.second << "\n";
        }
    });

    // 请求验证码接口：从 VerifyServer 获取验证码
    RegisterPost("/get_verify_code", [](const std::shared_ptr<HttpConnection>& connection) {
        std::string body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "[GateServer] 收到验证码请求数据: " << body_str << std::endl;

        connection->response_.set(http::field::content_type, "application/json");

        Json::Value root, src_root;
        Json::Reader reader;  // 老版本 JSON 解析器（简单用即可）
        if (!reader.parse(body_str, src_root)) {
            root["error"] = ErrorCodes::ERROR_JSON_PARSE;
            beast::ostream(connection->response_.body()) << root.toStyledString();
            return true;
        }

        std::string email = src_root["email"].asString();
        std::cout << "[GateServer] 转发到 VerifyServer 生成验证码, email = " << email << std::endl;

        GetVerifyCodeRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);

        root["error"] = rsp.error();
        root["email"] = email;
        beast::ostream(connection->response_.body()) << root.toStyledString();
        return true;
    });

    // 用户注册接口：验证验证码 + 校验用户是否已存在
    RegisterPost("/register_user", [](const std::shared_ptr<HttpConnection>& connection) {
        std::string body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "LogicSystem.cpp: [GateServer] 收到注册请求: " << body_str << std::endl;

        connection->response_.set(http::field::content_type, "application/json");

        Json::Value root;
        Json::Value src_root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string parse_errors;

        if (!reader->parse(body_str.data(), body_str.data() + body_str.size(), &src_root, &parse_errors)) {
            std::cout << "[GateServer] 注册请求 JSON 解析失败: " << parse_errors << std::endl;
            root["error"] = ErrorCodes::ERROR_JSON_PARSE;
            beast::ostream(connection->response_.body()) << root.toStyledString();
            return true;
        }

        // 1. 校验验证码是否存在/过期
        std::string key = VERIFY_CODE_PREFIX + src_root["email"].asString();
        std::string verify_code;
        if (!RedisManager::GetInstance()->Get(key, verify_code)) {
            root["error"] = ErrorCodes::ERROR_VERIFY_EXPIRED;
            beast::ostream(connection->response_.body()) << root.toStyledString();
            return true;
        }

        // 2. 校验验证码是否正确
        if (verify_code != src_root["verify_code"].asString()) {
            root["error"] = ErrorCodes::ERROR_VERIFY_CODE;
            beast::ostream(connection->response_.body()) << root.toStyledString();
            return true;
        }

        // 3. 校验用户名是否已注册
        if (RedisManager::GetInstance()->ExistsKey(src_root["user"].asString())) {
            root["error"] = ErrorCodes::ERROR_USER_REGISTERED;
            beast::ostream(connection->response_.body()) << root.toStyledString();
            return true;
        }

        // 4. 数据库注册用户
        std::string name = src_root["user"].asString();
        std::string email = src_root["email"].asString();
        std::string password = src_root["password"].asString();
        int uid = MysqlManager::GetInstance()->RegisterUser(name, email, password);
        if (uid == 0 || uid == -1) {
            std::cout << " LogicSystem.cpp: user or email exist" << std::endl;
            root["error"] = ErrorCodes::ERROR_USER_REGISTERED;
            std::string json_str = root.toStyledString();
            beast::ostream(connection->response_.body()) << json_str;
            return true;
        }

        // 5. 所有校验通过，返回成功（⚠️ 示例里直接回显，实际应写入数据库/Redis）
        root["error"] = 0;
        root["uid"] = uid;
        root["email"] = src_root["email"];
        root["user"] = src_root["user"].asString();
        root["password"] = src_root["password"].asString();       // ⚠️ 明文密码仅示例，实际需加密存储
        root["confirm"] = src_root["confirm"].asString();
        root["verify_code"] = src_root["verify_code"].asString();

        beast::ostream(connection->response_.body()) << root.toStyledString();
        return true;
    });

    // 重置密码回调逻辑
    RegisterPost("/reset_password", [](const std::shared_ptr<HttpConnection>& connection) {
        // 读取请求体
        std::string body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "[ResetPassword] Receive body: " << body_str << std::endl;

        connection->response_.set(http::field::content_type, "application/json");

        Json::Value root;
        Json::Value src_root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string parse_errors;

        // 统一的错误响应写入函数
        auto sendError = [&](int errorCode, const std::string& logMsg) {
            if (!logMsg.empty()) {
                std::cout << "[ResetPassword] " << logMsg << std::endl;
            }
            root["error"] = errorCode;
            std::string json_str = root.toStyledString();
            beast::ostream(connection->response_.body()) << json_str;
            return true;
        };

        // 1. 解析 JSON
        if (!reader->parse(body_str.data(), body_str.data() + body_str.size(), &src_root, &parse_errors)) {
            return sendError(ErrorCodes::ERROR_JSON_PARSE, "Failed to parse JSON data!");
        }

        auto name  = src_root["user"].asString();
        auto email = src_root["email"].asString();
        auto new_password   = src_root["password"].asString();

        // 2. 校验 Redis 中的验证码
        std::string verify_code;
        bool is_get_verify_code = RedisManager::GetInstance()->Get(VERIFY_CODE_PREFIX + email, verify_code);
        if (!is_get_verify_code) {
            return sendError(ErrorCodes::ERROR_VERIFY_EXPIRED, "Verification code expired");
        }
        if (verify_code != src_root["verify_code"].asString()) {
            return sendError(ErrorCodes::ERROR_VERIFY_CODE, "Verification code mismatch");
        }

        // 3. 校验数据库中用户名和邮箱是否匹配
        if (!MysqlManager::GetInstance()->CheckEmail(name, email)) {
            return sendError(ErrorCodes::ERROR_EMAIL_MISMATCH, "User and email not match");
        }

        // 4. 更新密码
        if (!MysqlManager::GetInstance()->UpdatePassword(name, new_password)) {
            return sendError(ErrorCodes::ERROR_PASSWORD_UPDATE, "Failed to update password");
        }

        // 5. 成功响应
        std::cout << "[ResetPassword] Password updated successfully for user: " << name << std::endl;
        root["error"] = 0;
        root["email"] = email;
        root["user"] = name;
        root["password"] = new_password;   // ⚠️ 生产环境最好不要明文返回密码
        root["verify_code"] = src_root["verify_code"].asString();

        std::string json_str = root.toStyledString();
        beast::ostream(connection->response_.body()) << json_str;
        return true;
    });

    // 用户登录逻辑

    RegisterPost("/login_user", [](const std::shared_ptr<HttpConnection>& connection) {
        // 获取请求体字符串
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;

        // 设置返回内容类型为 JSON
        connection->response_.set(http::field::content_type, "application/json");

        Json::Value root;       // 返回 JSON
        Json::Value src_root;   // 请求 JSON
        Json::Reader reader;

        // 统一错误返回函数，减少重复代码
        auto sendError = [&](int errCode, const std::string& logMsg = "") {
            if (!logMsg.empty()) {
                std::cout << logMsg << std::endl;
            }
            root["error"] = errCode;
            beast::ostream(connection->response_.body()) << root.toStyledString();
//            return true;
        };

        // 解析 JSON 请求
        if (!reader.parse(body_str, src_root)) {
            sendError(ErrorCodes::ERROR_JSON_PARSE, "Failed to parse JSON data!");
            return;
        }

        // 获取用户名和密码
        std::string email = src_root["email"].asString();
        std::string password  = src_root["password"].asString();
//        password = xorString(password);

        // 查询数据库校验密码
        UserInfo userInfo;
        if (!MysqlManager::GetInstance()->CheckPassword(email, password, userInfo)) {
            sendError(ErrorCodes::ERROR_EMAIL_MISMATCH, "user password not match");
            return;
        }

        // 向 StatusServer 请求合适的 ChatServer
        auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
        if (reply.error()) {
            sendError(ErrorCodes::ERROR_RPC_REQUEST,
                      "[LogicSystem.cpp]: grpc get chat server failed, error is " + std::to_string(reply.error()));
            return;
        }

        // 登录成功，返回用户信息和服务器信息
        std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
        root["error"] = 0;
        root["uid"]   = userInfo.uid;
        root["name"]  = userInfo.name;
        root["email"] = userInfo.email;
        root["token"] = reply.token();
        root["host"]  = reply.host();
        root["port"]  = reply.port();

        std::cout << "[LogicSystem.cpp]: login success, uid is " << userInfo.uid << ", token is " << reply.token() <<
            ", host is " << reply.host() << std::endl;
        beast::ostream(connection->response_.body()) << root.toStyledString();
        return;
    });


}

// 注册GET请求处理函数 - 将URL与对应的处理函数添加到映射表中
// 这里使用insert而非[]是为了避免当URL已存在时意外覆盖原有处理函数
void LogicSystem::RegisterGet(const std::string& url, const HttpHandler& handler) {
    // insert方法只会在键不存在时插入新值，若键已存在则不执行任何操作
    get_handlers_.insert(make_pair(url, handler));
}

// 注册POST请求处理函数 - 与RegisterGet实现逻辑相同
void LogicSystem::RegisterPost(const std::string& url, const HttpHandler& handler) {
    post_handlers_.insert(make_pair(url, handler));
}

// 处理GET请求 - 根据URL路径查找并调用对应的处理函数
bool LogicSystem::HandleGet(const std::string& path, const std::shared_ptr<HttpConnection>& connection) {
    // 1. 首先检查URL路径是否已注册对应的处理函数
    if (get_handlers_.find(path) == get_handlers_.end()) {
        return false;  // 未找到对应的处理函数
    }

    // 2. 由于前面已经确认路径存在，这里使用[]操作符只是访问已存在的处理函数，不会插入新元素
    //    []获取到函数对象后，立即调用该函数并传入connection参数
    get_handlers_[path](connection);
    return true;
}

// 处理POST请求 - 与HandleGet实现逻辑相同
bool LogicSystem::HandlePost(const std::string& path, const std::shared_ptr<HttpConnection>& connection) {
    if (post_handlers_.find(path) == post_handlers_.end()) {
        return false;
    }

    post_handlers_[path](connection);
    return true;
}
