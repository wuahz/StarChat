//
// RpcConnectionPool.h
// Created by 任兀华 on 2025/8/19.
//

#ifndef GATESERVER_RPCCONNECTIONPOOL_H
#define GATESERVER_RPCCONNECTIONPOOL_H

#include <grpcpp/grpcpp.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <string>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

/**
 * @brief RPC连接池模板类，用于管理和复用不同类型的gRPC连接
 * @tparam ServiceType gRPC服务类型，例如 message::VerifyService
 */
template <typename ServiceType>
class RpcConnectionPool {
public:
    /**
     * @brief 构造函数，初始化连接池
     * @param host 服务器主机地址
     * @param port 服务器端口号
     * @param pool_size 连接池大小
     */
    RpcConnectionPool(const std::string& host, const std::string& port, const std::string& pool_size);

    /**
     * @brief 析构函数，关闭连接池并清理所有连接
     */
    ~RpcConnectionPool();

    /**
     * @brief 从连接池获取一个可用连接
     * @return std::unique_ptr<typename ServiceType::Stub> 返回的连接对象指针
     * @note 如果连接池已关闭则返回 nullptr
     */
    std::unique_ptr<typename ServiceType::Stub> AcquireConnection();

    /**
     * @brief 归还连接到连接池
     * @param connection 要归还的连接对象指针
     */
    void ReleaseConnection(std::unique_ptr<typename ServiceType::Stub> connection);

    /**
     * @brief 关闭连接池，通知所有等待线程并停止服务
     */
    void Close();

private:
    std::atomic<bool> is_stopped_;   ///< 连接池停止标志
    std::string host_;               ///< 服务器主机地址
    std::string port_;               ///< 服务器端口号
    std::string pool_size_;               ///< 连接池大小

    std::queue<std::unique_ptr<typename ServiceType::Stub>> connection_queue_; ///< 可用连接队列
    std::mutex mutex_;              ///< 互斥锁
    std::condition_variable cond_var_; ///< 条件变量
};

#include "RpcConnectionPool.tpp"  // 模板实现放在 tpp 文件

#endif //GATESERVER_RPCCONNECTIONPOOL_H
