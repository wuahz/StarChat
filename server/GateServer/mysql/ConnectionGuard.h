#ifndef CONNECTION_GUARD_H
#define CONNECTION_GUARD_H

#include "MysqlConnectionPool.h"

/**
 * @class ConnectionGuard
 * @brief RAII 封装的 MySQL 连接管理器
 *
 * 在作用域内获取连接，析构时自动归还。
 */
class ConnectionGuard {
public:
    /**
     * @brief 构造函数
     * @param pool 连接池引用
     * @param conn 从连接池获取的连接对象
     */
    ConnectionGuard(MysqlConnectionPool& pool, std::unique_ptr<MysqlConnection> conn);

    /// 析构函数，自动归还连接
    ~ConnectionGuard();

    /// 获取底层 MySQL 连接指针（只借用，不负责释放）
    [[nodiscard]] sql::Connection* get() const;

private:
    MysqlConnectionPool& pool_;                    ///< 连接池引用
    std::unique_ptr<MysqlConnection> connection_;  ///< 当前持有的连接
};

#endif // CONNECTION_GUARD_H
