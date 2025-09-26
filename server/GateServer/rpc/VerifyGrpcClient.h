#ifndef GATESERVER_VERIFYGRPCCLIENT_H
#define GATESERVER_VERIFYGRPCCLIENT_H

#include "const.h"
#include "rpc/RpcConnectionPool.h"
#include "protobuf/message.grpc.pb.h"

using message::VerifyService;
using message::GetVerifyCodeReq;
using message::GetVerifyCodeRsp;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    /**
     * @brief 获取验证码
     * @param email 邮箱
     * @return GetVerifyRsp 验证码响应
     */
    GetVerifyCodeRsp GetVerifyCode(const std::string& email);
    ~VerifyGrpcClient();

private:
    VerifyGrpcClient();

    std::unique_ptr<RpcConnectionPool<VerifyService>> connection_pool_; // 连接池
};

#endif //GATESERVER_VERIFYGRPCCLIENT_H
