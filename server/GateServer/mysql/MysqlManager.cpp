//
// Created by 任兀华 on 2025/9/9.
//

#include "MysqlManager.h"

MysqlManager::MysqlManager()
{

}

MysqlManager::~MysqlManager()
{

}

int MysqlManager::RegisterUser(const std::string &name, const std::string &email, const std::string &password) {
    return mysql_dao_.RegisterUser(name, email, password);
}

bool MysqlManager::CheckEmail(const std::string &name, const std::string &email) {
    return mysql_dao_.CheckEmail(name, email);
}

bool MysqlManager::UpdatePassword(const std::string &name, const std::string &new_password) {
    return mysql_dao_.UpdatePassword(name, new_password);
}

bool MysqlManager::CheckPassword(const std::string& email, const std::string& password, UserInfo& userInfo) {
    return mysql_dao_.CheckPassword(email, password, userInfo);
}


