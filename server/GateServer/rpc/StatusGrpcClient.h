//
// Created by 任兀华 on 2025/9/17.
//

#ifndef GATESERVER_STATUSGRPCCLIENT_H
#define GATESERVER_STATUSGRPCCLIENT_H

#include "const.h"
#include "rpc/RpcConnectionPool.h"
#include "protobuf/message.grpc.pb.h"

using message::StatusService;
using message::GetChatServerReq;
using message::GetChatServerRsp;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    GetChatServerRsp GetChatServer(int uid);
    ~StatusGrpcClient();

private:
    StatusGrpcClient();

    std::unique_ptr<RpcConnectionPool<StatusService>> connection_pool_; // 连接池
};


#endif //GATESERVER_STATUSGRPCCLIENT_H
