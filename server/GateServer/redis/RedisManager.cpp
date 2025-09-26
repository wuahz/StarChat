//
// Created by 任兀华 on 2025/8/19.
//

#include "RedisManager.h"
#include "utils/ConfigManager.h"

RedisManager::RedisManager()
{
    ConfigManager& cfg_mgr = ConfigManager::GetInstance();
    std::string host = cfg_mgr["Redis"]["Host"];
    std::string port = cfg_mgr["Redis"]["Port"];
    std::string password = cfg_mgr["Redis"]["Password"];
    connection_pool_ = std::make_unique<RedisConnectionPool>(5, host, stoi(port), password);
}

RedisManager::~RedisManager()
{
    Close();
}

bool RedisManager::Get(const std::string &key, std::string& value)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }
    
    auto reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    if (reply == nullptr) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        connection_pool_->ReleaseConnection(context);
        return false;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        connection_pool_->ReleaseConnection(context);
        return false;
    }
    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    connection_pool_->ReleaseConnection(context);
    return true;
}

bool RedisManager::Set(const std::string &key, const std::string &value){
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }
    
    //执行redis命令行
    auto reply = (redisReply*)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
    //如果返回NULL则说明执行失败
    if (reply == nullptr)
    {
        std::cout << "Execut command [ SET " << key << "  "<< value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    //如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisManager::Auth(const std::string &password)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "AUTH %s", password.c_str());
    if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "认证失败" << std::endl;
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        return false;
    }
    else {
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        std::cout << "认证成功" << std::endl;
        return true;
    }
}

bool RedisManager::LPush(const std::string &key, const std::string &value)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::LPop(const std::string &key, std::string& value){
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key<<  " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ LPOP " << key <<  " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::RPush(const std::string& key, const std::string& value) {
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::RPop(const std::string& key, std::string& value) {
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ) {
        std::cout << "Execute command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execute command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}
bool RedisManager::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }
    
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    auto reply = (redisReply*)redisCommandArgv(context, 4, argv, argvlen);
    if (reply == nullptr || reply->type != REDIS_REPLY_NIL) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

std::string RedisManager::HGet(const std::string &key, const std::string &hkey)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }
    
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    auto reply = (redisReply*)redisCommandArgv(context, 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        std::cout << "Execute command [ HGet " << key << " "<< hkey <<"  ] failure ! " << std::endl;
        return "";
    }
    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execute command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisManager::Del(const std::string &key)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execute command [ Del " << key <<  " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execute command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::ExistsKey(const std::string &key)
{
    auto context = connection_pool_->AcquireConnection();
    if(context == nullptr)
    {
        return false;
    }

    auto reply = (redisReply*)redisCommand(context, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

void RedisManager::Close()
{
    connection_pool_->Close();
}
