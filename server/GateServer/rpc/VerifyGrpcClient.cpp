//
// Created by 任兀华 on 2025/8/14.
//

#include "VerifyGrpcClient.h"
#include "utils/ConfigManager.h"
#include <memory>

VerifyGrpcClient::~VerifyGrpcClient() = default;

VerifyGrpcClient::VerifyGrpcClient() {
    auto& cfg_mgr = ConfigManager::GetInstance();
    std::string host = cfg_mgr["VerifyServer"]["Host"];
    std::string port = cfg_mgr["VerifyServer"]["Port"];
    std::string pool_size = cfg_mgr["VerifyServer"]["PoolSize"];
    // 初始化连接池：使用配置的host、port和pool_size
    connection_pool_ = std::make_unique<RpcConnectionPool<VerifyService>>(host, port, pool_size);
}

GetVerifyCodeRsp VerifyGrpcClient::GetVerifyCode(const std::string& email) {
    ClientContext context;          // gRPC上下文（可设置超时、元数据等）
    GetVerifyCodeReq request;           // 请求对象
    GetVerifyCodeRsp reply;             // 响应对象
    request.set_email(email);       // 设置请求参数

    // 从连接池获取gRPC存根
    auto stub_ = connection_pool_->AcquireConnection();
    std::cout << "[GateServer] 准备通过 gRPC 将用户邮箱发送至验证码服务 [VerifyServer]: " << request.email() << std::endl;
    // 调用远程方法
    Status status = stub_->GetVerifyCode(&context, request, &reply);
    // 归还连接
    connection_pool_->ReleaseConnection(std::move(stub_));

    if (status.ok()) {
        std::cout << "[GateServer] 收到 VerifyServer 响应: 邮箱=" << email << std::endl;
    } else {
        reply.set_error(::ErrorCodes::ERROR_RPC_REQUEST);
        std::cout << "[GateServer] 调用 VerifyServer 失败: 错误码=" << reply.error() << std::endl;
    }

    return reply;
}