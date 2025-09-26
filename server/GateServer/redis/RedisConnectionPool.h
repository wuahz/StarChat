//
// Created by 任兀华 on 2025/8/19.
//

#ifndef GATESERVER_REDISCONNECTIONPOOL_H
#define GATESERVER_REDISCONNECTIONPOOL_H

#include "const.h"

class RedisConnectionPool {
public:
    /**
     * @brief 构造函数，初始化连接池
     * @param poolSize 连接池大小（即预先建立的连接数）
     * @param host Redis 服务器地址
     * @param port Redis 服务器端口
     * @param password Redis 认证密码
     */
    RedisConnectionPool(size_t poolSize, const std::string& host, int port, const std::string& password);

    /**
     * @brief 析构函数，停止连接池并释放所有连接
     */
    ~RedisConnectionPool();

    /**
     * @brief 从连接池获取一个 Redis 连接
     * @return redisContext* 返回一个可用的 Redis 连接指针；如果连接池已停止，返回 nullptr
     */
    redisContext* AcquireConnection();

    /**
     * @brief 将 Redis 连接归还到连接池
     * @param context 要归还的连接指针
     */
    void ReleaseConnection(redisContext* context);

    /**
     * @brief 停止连接池，唤醒所有等待获取连接的线程
     */
    void Close();

private:
    // 初始化单个 Redis 连接，并进行认证
    redisContext* CreateAndAuthConnection(const std::string& host, int port, const std::string& password);

private:
    std::atomic<bool> is_stopped_;          // 标记连接池是否已停止
    size_t pool_size_;                      // 连接池大小
    std::string host_;                     // Redis 服务器地址
    int port_;                             // Redis 服务器端口
    std::queue<redisContext*> connection_queue_; // 空闲连接队列
    std::mutex mutex_;                     // 互斥锁，保护共享资源
    std::condition_variable cond_var_;      // 条件变量，用于线程等待连接
};


#endif //GATESERVER_REDISCONNECTIONPOOL_H
