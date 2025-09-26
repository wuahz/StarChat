//
// Created by 任兀华 on 2025/9/8.
//

#include "MysqlConnectionPool.h"
#include <chrono>
#include <thread>
#include <iostream>

using namespace std::chrono;

MysqlConnectionPool::MysqlConnectionPool(const std::string& host,
                                         const std::string& port,
                                         const std::string& database,
                                         const std::string& user,
                                         const std::string& password,
                                         int poolsize)
        : host_(host), port_(port), database_(database), user_(user), password_(password),
          pool_size_(poolsize), is_stopped_(false), fail_count_(0) {

    try {
        // 初始化连接池
        for (int i = 0; i < pool_size_; ++i) {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            auto*  con = driver->connect(host_ + ":" + port_, user_, password_);
            con->setSchema(database_);
            // 获取当前时间戳
            auto currentTime = std::chrono::system_clock::now().time_since_epoch();
            // 将时间戳转换为秒
            long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
            connection_queue_.push(std::make_unique<MysqlConnection>(con, timestamp));
        }

        // 启动后台线程，定时检测连接健康状态
        check_thread_ = std::thread([this]() {
            while (!is_stopped_) {
                CheckConnection();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        });

    check_thread_.detach();
    }
    catch (sql::SQLException& e) {
        // 处理异常
        std::cout << "mysql pool init failed, error is " << e.what()<< std::endl;
    }
}

MysqlConnectionPool::~MysqlConnectionPool() {
    Close();
}

void MysqlConnectionPool::CheckConnection() {
    std::lock_guard<std::mutex> guard(mutex_);
    size_t pool_size = connection_queue_.size();
    // 获取当前时间戳
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    // 将时间戳转换为秒
    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    // 遍历连接队列
    for (int i = 0; i < pool_size; i++) {
        auto connection = std::move(connection_queue_.front());
        connection_queue_.pop();
        // 使用Defer确保当前连接在作用域结束时（无论是否发生异常）被重新放回连接池
        Defer defer([this, &connection]() {
            connection_queue_.push(std::move(connection));
        });

        if (timestamp - connection->last_time_ < 5) {
            continue;
        }

        try {
            std::unique_ptr<sql::Statement> statement(connection->mysql_connection_->createStatement());
            statement->executeQuery("SELECT 1");
            connection->last_time_ = timestamp;
            std::cout << "execute timer alive query , cur is " << timestamp << std::endl;
        }
        catch (sql::SQLException& e) {
            std::cout << "Error keeping connection alive: " << e.what() << std::endl;
            // 重新创建连接并替换旧的连接
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            auto* new_connection = driver->connect(host_ + ":" + port_, user_, password_);
            new_connection->setSchema(database_);
            connection->mysql_connection_.reset(new_connection);
            connection->last_time_ = timestamp;
        }
    }
}

std::unique_ptr<MysqlConnection> MysqlConnectionPool::AcquireConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待直到有可用连接
    cond_var_.wait(lock, [this]
    {
        return !connection_queue_.empty() || is_stopped_;
    });

    if (is_stopped_) return nullptr;

    auto connection = std::move(connection_queue_.front());
    connection_queue_.pop();
    return connection;
}

void MysqlConnectionPool::ReleaseConnection(std::unique_ptr<MysqlConnection> connection) {
    if (!connection) return;

    std::lock_guard<std::mutex> lock(mutex_);
    connection_queue_.push(std::move(connection));
    cond_var_.notify_one();
}

void MysqlConnectionPool::Close() {
    is_stopped_ = true;
    cond_var_.notify_all();

    if (check_thread_.joinable()) {
        check_thread_.join();
    }

    // 清空连接池
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connection_queue_.empty()) {
        connection_queue_.pop();
    }
}

/// @brief 后台线程：定期检测连接是否健康
//void MysqlConnectionPool::KeepAliveWorker() {
//    while (!is_stopped_) {
//        std::this_thread::sleep_for(std::chrono::seconds(30));
//
//        std::lock_guard<std::mutex> lock(mutex_);
//        if (!connection_queue_.empty()) {
//            auto timestamp = duration_cast<milliseconds>(
//                    system_clock::now().time_since_epoch())
//                    .count();
//            if (!Reconnect(timestamp)) {
//                fail_count_++;
//                if (fail_count_ > 5) {
//                    std::cerr << "Too many MySQL reconnection failures, stopping pool!" << std::endl;
//                    is_stopped_ = true;
//                }
//            }
//        }
//    }
//}
//
///// @brief 尝试重建连接（心跳检测失败时调用）
//bool MysqlConnectionPool::Reconnect(long long timestamp) {
//    try {
//        sql::Driver* driver = sql::mysql::get_driver_instance();
//        auto connection = std::unique_ptr<MysqlConnection>(
//                new MysqlConnection(driver->connect(url_, user_, pass_),
//                                    timestamp)
//        );
//        connection->connection_->setSchema(database_);
//        connection_queue_.push(std::move(connection));
//        return true;
//    } catch (sql::SQLException& e) {
//        std::cerr << "Reconnect failed: " << e.what() << std::endl;
//        return false;
//    }
//}