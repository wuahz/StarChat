#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <cassert>
#include <format>
#include "utils/Singleton.h"
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <hiredis/hiredis.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <grpcpp/grpcpp.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
    SUCCESS = 0,                  // 成功（无 ERROR_，因为是“非错误”）
    ERROR_JSON_PARSE = 1001,      // JSON 解析错误
    ERROR_RPC_REQUEST = 1002,     // RPC 请求错误
    ERROR_VERIFY_EXPIRED = 1003,  // 验证码过期
    ERROR_VERIFY_CODE = 1004,     // 验证码错误
    ERROR_USER_REGISTERED = 1005, // 用户已注册
    ERROR_PASSWORD = 1006,        // 密码错误
    ERROR_EMAIL_MISMATCH = 1007,  // 邮箱不匹配
    ERROR_PASSWORD_UPDATE = 1008, // 密码更新失败
    ERROR_PASSWORD_INVALID = 1009 // 密码无效
};

// gRPC server address configuration
const std::string VERIFY_GRPC_SERVER_ADDRESS = "127.0.0.1:50051";

#define VERIFY_CODE_PREFIX "verify_code_"

