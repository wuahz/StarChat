//
// Created by 任兀华 on 2025/9/8.
//

#ifndef GATESERVER_MYSQLCONNECTION_H
#define GATESERVER_MYSQLCONNECTION_H

#pragma once
#include <memory>
#include <jdbc/cppconn/connection.h>

/**
 * @brief 封装单个 MySQL 连接及其最近使用时间
 */
class MysqlConnection {
public:
    /**
     * @brief 构造函数
     * @param connection MySQL 原始连接指针
     * @param lastTime 最近使用时间戳（毫秒）
     */
    MysqlConnection(sql::Connection* connection, int64_t lastTime)
            : mysql_connection_(connection), last_time_(lastTime) {}

    std::unique_ptr<sql::Connection> mysql_connection_; ///< MySQL 连接对象
    int64_t last_time_;                  ///< 最近一次操作时间戳（毫秒）
};

#endif //GATESERVER_MYSQLCONNECTION_H
