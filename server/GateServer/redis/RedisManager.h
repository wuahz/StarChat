//
// Created by 任兀华 on 2025/8/19.
//

#ifndef GATESERVER_REDISMANAGER_H
#define GATESERVER_REDISMANAGER_H

#include "const.h"
#include "RedisConnectionPool.h"

class RedisManager: public Singleton<RedisManager>,
                public std::enable_shared_from_this<RedisManager>
{
    // 声明 Singleton 模板类为友元，以便其可以访问私有构造函数
    friend class Singleton<RedisManager>;

public:
    // 析构函数，用于清理资源
    ~RedisManager();

    // 根据 key 获取对应的 string 类型的 value
    bool Get(const std::string &key, std::string& value);

    // 设置 key 对应的 string 值
    bool Set(const std::string &key, const std::string &value);

    // 使用密码进行 Redis 身份验证
    bool Auth(const std::string &password);

    // 将值推入列表头部 (LPUSH)
    bool LPush(const std::string &key, const std::string &value);

    // 从列表头部弹出一个值 (LPOP)，并将结果存入 value
    bool LPop(const std::string &key, std::string& value);

    // 将值推入列表尾部 (RPUSH)
    bool RPush(const std::string& key, const std::string& value);

    // 从列表尾部弹出一个值 (RPOP)，并将结果存入 value
    bool RPop(const std::string& key, std::string& value);

    // 设置哈希表中字段的值 (HSET，string版本)
    bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);

    // 设置哈希表中字段的值 (HSET，C风格字符串版本，带长度参数)
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);

    // 获取哈希表中字段的值 (HGET)，返回 string 类型
    std::string HGet(const std::string &key, const std::string &hkey);

    // 删除指定的 key
    bool Del(const std::string &key);

    // 检查某个 key 是否存在
    bool ExistsKey(const std::string &key);

    // 关闭 Redis 连接
    void Close();

private:
    // 私有构造函数，确保只能通过 Singleton 创建实例
    RedisManager();

    std::unique_ptr<RedisConnectionPool> connection_pool_; // 连接池
};


#endif //GATESERVER_REDISMANAGER_H
