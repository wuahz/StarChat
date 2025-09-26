//
// Created by 任兀华 on 2025/9/8.
//

#include "MysqlDao.h"
#include "ConnectionGuard.h"
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <iostream>

MysqlDao::MysqlDao() {
    ConfigManager& config_manager = ConfigManager::GetInstance();
    // 初始化连接池，参数应从配置文件读取
    connection_pool_ = std::make_unique<MysqlConnectionPool>(
            config_manager["Mysql"]["Host"],
            config_manager["Mysql"]["Port"],
            config_manager["Mysql"]["Database"],
            config_manager["Mysql"]["User"],
            config_manager["Mysql"]["Password"],
            std::stoi(config_manager["Mysql"]["PoolSize"])
    );
}

MysqlDao::~MysqlDao() {
    connection_pool_->Close();
}

int MysqlDao::RegisterUser(const std::string& name,
                                      const std::string& email,
                                      const std::string& password) {
    try {
        // 开始事务
//        connection->connection_->setAutoCommit(false);

        // RAII：作用域结束时自动将连接归还给连接池
        ConnectionGuard connection_guard(*connection_pool_, connection_pool_->AcquireConnection());
        sql::Connection * raw_connection = connection_guard.get();
        if (!raw_connection) {
            std::cerr << "[RegisterUser] Failed to get MySQL connection from pool" << std::endl;
            return false;
        }

        // 准备调用存储过程
        std::unique_ptr<sql::PreparedStatement> preparedStatement(
                raw_connection->prepareStatement("CALL register_user(?,?,?,@result)"));

        // 设置输入参数
        std::cout << "MysqlDao.cpp: name:" << name << " email:" << email << " password:" << password << std::endl;
        preparedStatement->setString(1, name);
        preparedStatement->setString(2, email);
        preparedStatement->setString(3, password);

        // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

        // 执行存储过程
        preparedStatement->execute();
        // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
        // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
        std::unique_ptr<sql::Statement> statement(raw_connection->createStatement());
        std::unique_ptr<sql::ResultSet> resultSet(statement->executeQuery("SELECT @result AS result"));
        std::cout << "MysqlDao.cpp: resultSet:" << resultSet << std::endl;
        if (resultSet->next()) {
            int result = resultSet->getInt("result");
            std::cout << "Result: " << result << std::endl;
            return result;
        }
        return -1;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
    try {
        // RAII：作用域结束时自动将连接归还给连接池
        ConnectionGuard connection_guard(*connection_pool_, connection_pool_->AcquireConnection());
        sql::Connection * raw_connection = connection_guard.get();
        if (!raw_connection) {
            std::cerr << "[CheckEmail] Failed to get MySQL connection from pool" << std::endl;
            return false;
        }

        // 1. 准备查询语句
        std::unique_ptr<sql::PreparedStatement> preparedStatement(
                raw_connection->prepareStatement("SELECT email FROM user WHERE name = ?")
        );

        // 2. 绑定参数
        preparedStatement->setString(1, name);

        // 3. 执行查询
        std::unique_ptr<sql::ResultSet> resultSet(preparedStatement->executeQuery());

        // 4. 遍历结果集（理论上 name 唯一，结果只有一行）
        if (resultSet->next()) {
            std::string result = resultSet->getString("email");
            std::cout << "[CheckEmail] DB email = " << result << ", Input email = " << email << std::endl;
            if (result != email) {
                return false;
            }
            return true;
        }

        // 用户不存在
        std::cout << "[CheckEmail] User not found: " << name << std::endl;
        return false;
    }
    catch (sql::SQLException& e) {
        std::cerr << "[CheckEmail] SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        return false;
    }
}

bool MysqlDao::UpdatePassword(const std::string& name, const std::string& new_password) {
    try {
        // RAII：作用域结束时自动将连接归还给连接池
        ConnectionGuard connection_guard(*connection_pool_, connection_pool_->AcquireConnection());
        sql::Connection * raw_connection = connection_guard.get();
        if (!raw_connection) {
            std::cerr << "[UpdatePassword] Failed to get MySQL connection from pool" << std::endl;
            return false;
        }

        // 3. 准备更新语句
        std::unique_ptr<sql::PreparedStatement> preparedStatement(
                raw_connection->prepareStatement("UPDATE user SET password = ? WHERE name = ?")
        );

        // 4. 绑定参数
        preparedStatement->setString(1, new_password);
        preparedStatement->setString(2, name);

        // 5. 执行更新
        int updateCount = preparedStatement->executeUpdate();
        std::cout << "[UpdatePassword] Updated rows: " << updateCount << std::endl;

        // 如果没有更新行，说明用户不存在
        return (updateCount > 0);
    }
    catch (sql::SQLException& e) {
        std::cerr << "[UpdatePassword] SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        return false;
    }
}

bool MysqlDao::CheckPassword(const std::string& email, const std::string& password, UserInfo& userInfo) {
   try{
       // RAII：作用域结束时自动将连接归还给连接池
       ConnectionGuard connection_guard(*connection_pool_, connection_pool_->AcquireConnection());
       sql::Connection * raw_connection = connection_guard.get();
       if (!raw_connection) {
           std::cerr << "[CheckPassword] Failed to get MySQL connection from pool" << std::endl;
           return false;
       }

       // 准备 SQL 查询（只取一行即可）
       auto preparedStatement = std::unique_ptr<sql::PreparedStatement>(
               raw_connection->prepareStatement("SELECT uid, name, password FROM user WHERE email = ? LIMIT 1")
       );
       preparedStatement->setString(1, email);

       // 执行查询
       auto res = std::unique_ptr<sql::ResultSet>(preparedStatement->executeQuery());

       if (!res->next()) {  // 用户不存在
           std::cout << "[CheckPassword] Password not found for email: " << email << std::endl;
           return false;
       }

       // 获取数据库中的密码
       const auto original_password = res->getString("password");
       std::cout << "[CheckPassword] Password in DB: " << original_password << std::endl;

       // 校验密码
       if (password != original_password) {
           return false;
       }

       // 填充用户信息
       userInfo.uid   = res->getInt("uid");
       userInfo.name  = res->getString("name");
       userInfo.email = email;
       userInfo.password   = original_password;

       return true;
    }
    catch (const sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        return false;
    }
}

