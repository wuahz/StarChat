//
// Created by 任兀华 on 2025/9/17.
//

#include "StatusGrpcClient.h"

//
// Created by 任兀华 on 2025/8/14.
//

#include "StatusGrpcClient.h"
#include "utils/ConfigManager.h"
#include <memory>

StatusGrpcClient::~StatusGrpcClient() = default;

StatusGrpcClient::StatusGrpcClient() {
    auto& cfg_mgr = ConfigManager::GetInstance();
    std::string host = cfg_mgr["StatusServer"]["Host"];
    std::string port = cfg_mgr["StatusServer"]["Port"];
    std::string pool_size = cfg_mgr["StatusServer"]["PoolSize"];
    // 初始化连接池：使用配置的host、port和pool_size
    connection_pool_ = std::make_unique<RpcConnectionPool<StatusService>>(host, port, pool_size);
}


GetChatServerRsp StatusGrpcClient::GetChatServer(int uid) {
    ClientContext context;          // gRPC上下文（可设置超时、元数据等）
    GetChatServerReq request;           // 请求对象
    GetChatServerRsp reply;             // 响应对象
    request.set_uid(uid);       // 设置请求参数

    // 从连接池获取gRPC存根
    auto stub_ = connection_pool_->AcquireConnection();
    std::cout << "[GateServer] 准备通过 gRPC 将用户 uid 发送至状态服务 [StatusServer]: " << std::endl;
    std::cout << "[StatusGrpcClient] GetChatServer uid: " << request.uid() << std::endl;
    // 调用远程方法
    Status status = stub_->GetChatServer(&context, request, &reply);
    // 归还连接
    connection_pool_->ReleaseConnection(std::move(stub_));

    if (status.ok()) {
        std::cout << "[GateServer] 收到 StatusServer 响应: uid=" << uid << std::endl;
    } else {
        reply.set_error(::ErrorCodes::ERROR_RPC_REQUEST);
        std::cout << "[GateServer] 调用 StatusServer 失败: 错误码=" << reply.error() << std::endl;
    }

    return reply;
}