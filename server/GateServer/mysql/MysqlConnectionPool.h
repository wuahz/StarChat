//
// Created by 任兀华 on 2025/9/8.
//

#ifndef GATESERVER_MYSQLCONNECTIONPOOL_H
#define GATESERVER_MYSQLCONNECTIONPOOL_H

#pragma once
#include "MysqlConnection.h"
#include "utils/defer.h"
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/statement.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <string>

/**
 * @brief MySQL 连接池类
 *
 * 职责：
 *  - 维护一组数据库连接
 *  - 提供 Acquire/Release 接口实现复用
 *  - 后台定时检测连接健康状态
 */
class MysqlConnectionPool {
public:
    /**
     * @brief 构造函数
     * @param url 数据库地址，例如 "tcp://127.0.0.1:3306"
     * @param user 用户名
     * @param pass 密码
     * @param schema 数据库名称
     * @param poolSize 连接池大小
     */
    MysqlConnectionPool(const std::string& host,
                        const std::string& port,
                        const std::string& database,
                        const std::string& user,
                        const std::string& password,
                        int poolSize);

    /// 禁止拷贝
    MysqlConnectionPool(const MysqlConnectionPool&) = delete;
    MysqlConnectionPool& operator=(const MysqlConnectionPool&) = delete;

    ~MysqlConnectionPool();

    /**
     * @brief 检查连接是否存活
     */
    void CheckConnection();

    /**
     * @brief 获取一个数据库连接
     * @return 可用的 MysqlConnection 智能指针
     * @note 如果连接池关闭，返回 nullptr
     */
    std::unique_ptr<MysqlConnection> AcquireConnection();

    /**
     * @brief 归还连接
     * @param conn 需要归还的连接
     */
    void ReleaseConnection(std::unique_ptr<MysqlConnection> conn);

    /**
     * @brief 主动关闭连接池
     */
    void Close();

private:
    /// @brief 后台线程函数，定时检测连接是否存活
//    void KeepAliveWorker();

    /// @brief 重新建立连接（心跳检测失败时调用）
//    bool Reconnect(long long timestamp);

private:
    std::string host_;    ///< 数据库主机地址
    std::string port_;    ///< 数据库端口
    std::string database_; ///< 数据库名
    std::string user_;   ///< 用户名
    std::string password_;   ///< 密码
    int pool_size_;       ///< 最大连接数

    std::queue<std::unique_ptr<MysqlConnection>> connection_queue_; ///< 空闲连接队列
    std::mutex mutex_;          ///< 互斥锁
    std::condition_variable cond_var_; ///< 条件变量（等待可用连接）
    std::thread check_thread_; ///< 心跳检测线程
    std::atomic<bool> is_stopped_;  ///< 停止标志
    std::atomic<int> fail_count_; ///< 连续失败次数
};

#endif //GATESERVER_MYSQLCONNECTIONPOOL_H
