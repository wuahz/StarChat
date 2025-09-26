#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(boost::asio::io_context& ioc)
    : socket_(ioc) {
}

tcp::socket& HttpConnection::GetSocket() {
    return socket_;
}

void HttpConnection::Start()
{
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, request_, [self](beast::error_code ec,
        std::size_t bytes_transferred) {
            try {
                if (ec) {
                    std::cout << "http read err is " << ec.what() << std::endl;
                    return;
                }

                //处理读到的数据
                boost::ignore_unused(bytes_transferred);
                self->HandleRequest();
                self->CheckDeadline();
            }
            catch (std::exception& exp) {
                std::cout << "exception is " << exp.what() << std::endl;
            }
        }
    );
}

unsigned char ToHex(unsigned char x) {
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //判断是否仅有数字和字母构成
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') //为空字符
            strTemp += "+";
        else
        {
            //其他字符需要提前加%并且高四位和低四位分别转为16进制
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //还原+为空
        if (str[i] == '+') strTemp += ' ';
        //遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam() {
    // 提取 URI  
    auto uri = request_.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）  
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        get_url_ = uri;
        return;
    }

    get_url_ = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
            value = UrlDecode(pair.substr(eq_pos + 1));
            get_params_[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）  
    if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            get_params_[key] = value;
        }
    }
}

void HttpConnection::HandleRequest()
{
    //设置版本
    response_.version(request_.version());
    //设置为短链接
    response_.keep_alive(false);

    if (request_.method() == http::verb::get) {
        PreParseGetParam();
        bool success = LogicSystem::GetInstance()->HandleGet(get_url_, shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }

    if (request_.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance()->HandlePost(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}

void HttpConnection::CheckDeadline() {
    // 这里添加检查截止时间的逻辑
    // 示例：仅打印日志
    std::cout << "Checking deadline..." << std::endl;
}

void HttpConnection::WriteResponse() {
    auto self = shared_from_this();
    http::async_write(socket_, response_, [self](beast::error_code ec, std::size_t) {
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};
    });
}