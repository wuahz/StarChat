//
// Created by 任兀华 on 2025/9/8.
//

#ifndef GATESERVER_USERINFO_H
#define GATESERVER_USERINFO_H

#pragma once
#include <string>

/**
 * @brief 用户信息结构体
 */
struct UserInfo {
    int uid;            ///< 用户 ID
    std::string name;   ///< 用户名
    std::string email;  ///< 邮箱
    std::string password;    ///< 密码
};

#endif //GATESERVER_USERINFO_H
