//
// Created by 任兀华 on 2025/8/19.
//

#include "RedisConnectionPool.h"

RedisConnectionPool::RedisConnectionPool(size_t poolSize, const std::string& host, int port, const std::string& password)
        : is_stopped_(false), pool_size_(poolSize), host_(host), port_(port) {

    for (size_t i = 0; i < pool_size_; ++i) {
        redisContext* ctx = CreateAndAuthConnection(host_, port_, password);
        if (ctx) {
            connection_queue_.push(ctx);
        }
    }
}

RedisConnectionPool::~RedisConnectionPool() {
    // 停止连接池
    Close();

    // 释放所有空闲连接
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connection_queue_.empty()) {
        redisContext* ctx = connection_queue_.front();
        connection_queue_.pop();
        if (ctx) {
            redisFree(ctx);  // 重要！释放 Redis 连接资源
        }
    }
}

redisContext* RedisConnectionPool::AcquireConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待直到有可用连接 或 连接池已停止
    cond_var_.wait(lock, [this]() {
        return is_stopped_.load() || !connection_queue_.empty();
    });

    // 如果已停止，返回空指针
    if (is_stopped_) {
        return nullptr;
    }

    // 取出一个空闲连接
    redisContext* ctx = connection_queue_.front();
    connection_queue_.pop();
    return ctx;
}

void RedisConnectionPool::ReleaseConnection(redisContext* context) {
    if (!context) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (is_stopped_) {
        redisFree(context);  // 如果已停止，直接释放连接
        return;
    }

    // 将连接放回空闲队列
    connection_queue_.push(context);
    cond_var_.notify_one();  // 通知一个等待的线程
}

void RedisConnectionPool::Close() {
    is_stopped_ = true;
    cond_var_.notify_all();  // 唤醒所有等待获取连接的线程
}

redisContext* RedisConnectionPool::CreateAndAuthConnection(const std::string& host, int port, const std::string& password) {
    redisContext* ctx = redisConnect(host.c_str(), port);
    if (ctx == nullptr || ctx->err != 0) {
        if (ctx) {
            std::cerr << "Redis连接失败: " << ctx->errstr << std::endl;
            redisFree(ctx);
        } else {
            std::cerr << "Redis连接失败: 无法分配上下文" << std::endl;
        }
        return nullptr;
    }

    // 认证
    redisReply* reply = (redisReply*)redisCommand(ctx, "AUTH %s", password.c_str());
    if (reply == nullptr) {
        std::cerr << "Redis认证命令执行失败" << std::endl;
        redisFree(ctx);
        return nullptr;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Redis认证失败: " << reply->str << std::endl;
        freeReplyObject(reply);
        redisFree(ctx);
        return nullptr;
    } else {
        std::cout << "Redis认证成功" << std::endl;
    }

    freeReplyObject(reply);
    return ctx;
}
