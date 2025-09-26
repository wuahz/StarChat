//
// Created by 任兀华 on 2025/8/14.
//

#ifndef GATESERVER_ASIOIOCONTEXTPOOL_H
#define GATESERVER_ASIOIOCONTEXTPOOL_H


#include <vector>
#include "third_party/boost_1_88_0/include/boost/asio.hpp"
#include "utils/Singleton.h"

class AsioIOContextPool : public Singleton<AsioIOContextPool>
{
    friend Singleton<AsioIOContextPool>;
public:
    // 简化 boost::asio::io_context 的类型名（I/O 事件循环核心）
    using IOContext = boost::asio::io_context;
    // 定义工作守卫类型，防止 io_context 在没有任务时退出
    using WorkGuard = boost::asio::executor_work_guard<IOContext::executor_type>;
    // 工作守卫的智能指针类型，用于自动管理生命周期
    using WorkGuardPtr = std::unique_ptr<WorkGuard>;

    ~AsioIOContextPool();
    AsioIOContextPool(const AsioIOContextPool&) = delete;
    AsioIOContextPool& operator=(const AsioIOContextPool&) = delete;
    // 使用 round-robin 的方式返回一个 io_service
    IOContext& GetIOContext();
    void Stop();
private:
    AsioIOContextPool(std::size_t size = 2); // std::thread::hardware_concurrency()
    std::vector<IOContext> _ioContexts;      // 多个 io_context 对象
    std::vector<WorkGuardPtr> _workGuards;   // 每个 io_context 关联一个 WorkGuard
    std::vector<std::thread> _threads;       // 运行 io_context 的线程池
    std::size_t _nextIOContextIndex; // 下一个 io_context 索引
};


#endif //GATESERVER_ASIOIOCONTEXTPOOL_H
