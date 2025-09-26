//
// Created by 任兀华 on 2025/9/9.
//

#ifndef GATESERVER_MYSQLMANAGER_H
#define GATESERVER_MYSQLMANAGER_H

#include "utils/Singleton.h"
#include "MysqlDao.h"

class MysqlManager : public Singleton<MysqlManager> {
    friend class Singleton<MysqlManager>;

public:
    ~MysqlManager();

    int RegisterUser(const std::string& name,
                     const std::string& email,
                     const std::string& password);

    /**
     * @brief 检查用户名和邮箱是否匹配
     * @param name  用户名
     * @param email 邮箱
     * @return true  用户存在且邮箱匹配
     * @return false 用户不存在或邮箱不匹配
     */
    bool CheckEmail(const std::string& name, const std::string& email);

    /**
     * @brief 更新用户密码
     * @param name   用户名
     * @param new_password 新密码
     * @return true  更新成功（至少一行被更新）
     * @return false 更新失败或异常
     */
    bool UpdatePassword(const std::string &name, const std::string &new_password);

    /**@brief 校验用户密码是否正确，并填充用户信息
     * @param email 邮箱名
     * @param password  输入的密码
     * @param userInfo 输出的用户信息（仅在成功时有效）
     * @return true 表示密码正确，false 表示失败
     */
    bool CheckPassword(const std::string& email, const std::string& password, UserInfo& userInfo);

private:
    MysqlManager();

    MysqlDao mysql_dao_;
};

#endif //GATESERVER_MYSQLMANAGER_H
