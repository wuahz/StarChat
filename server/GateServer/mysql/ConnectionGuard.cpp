#include "ConnectionGuard.h"

ConnectionGuard::ConnectionGuard(MysqlConnectionPool& pool, std::unique_ptr<MysqlConnection> conn)
        : pool_(pool), connection_(std::move(conn)) {}

ConnectionGuard::~ConnectionGuard() {
    if (connection_) {
        pool_.ReleaseConnection(std::move(connection_));
    }
}

sql::Connection* ConnectionGuard::get() const {
    return connection_ ? connection_->mysql_connection_.get() : nullptr;
}
