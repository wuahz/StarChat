//
// Created by 任兀华 on 2025/9/8.
//

#ifndef GATESERVER_MYSQLDAO_H
#define GATESERVER_MYSQLDAO_H

#pragma once
#include "MysqlConnectionPool.h"
#include "UserInfo.h"
#include "utils/ConfigManager.h"
#include <memory>
#include <string>

/**
 * @brief MySQL DAO 层，封装用户相关数据库操作
 *
 * 职责：
 *  - 封装常用 SQL（注册、登录、更新密码等）
 *  - 业务逻辑层调用 DAO，而不是直接使用连接池
 */
class MysqlDao {
public:
    MysqlDao();
    ~MysqlDao();

    /**
     * @brief 注册用户
     * @return 插入结果（成功返回 1，失败返回 -1）
     */
    int RegisterUser(const std::string& name,
                     const std::string& email,
                     const std::string& password);

    /**
     * @brief 检查邮箱是否存在
     * @param email 邮箱
     * @return true 邮箱存在
     * @return false 邮箱不存在
     */
    bool CheckEmail(const std::string& name, const std::string& email);

    /**
     * @brief 更新密码
     * @param name 用户名
     * @param new_password 新密码
     * @return true 表示更新成功，false 表示失败
     */
    bool UpdatePassword(const std::string& name, const std::string& new_password);

    /**
     * @brief 校验用户密码是否正确，并填充用户信息
     * @param email 邮箱名
     * @param password  输入的密码
     * @param userInfo 输出的用户信息（仅在成功时有效）
     * @return true 表示密码正确，false 表示失败
     */
    bool CheckPassword(const std::string& email, const std::string& password, UserInfo& userInfo);

private:
    std::unique_ptr<MysqlConnectionPool> connection_pool_; ///< 连接池实例
};

#endif //GATESERVER_MYSQLDAO_H
