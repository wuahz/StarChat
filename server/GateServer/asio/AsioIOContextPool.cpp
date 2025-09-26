//
// Created by 任兀华 on 2025/8/14.
//

#include "AsioIOContextPool.h"

AsioIOContextPool::AsioIOContextPool(std::size_t size)
        : _ioContexts(size),
          _workGuards(size),
          _nextIOContextIndex(0)
{
    // 初始化工作守卫
    for (std::size_t i = 0; i < size; ++i) {
        _workGuards[i] = std::make_unique<WorkGuard>(
                boost::asio::make_work_guard(_ioContexts[i])
        );
    }

    // 启动工作线程
    for (std::size_t i = 0; i < size; ++i) {
        _threads.emplace_back([this, i] {
            _ioContexts[i].run();
        });
    }
}

AsioIOContextPool::~AsioIOContextPool() {
    Stop();
    std::cout << "AsioIOContextPool destructed" << std::endl;
}

AsioIOContextPool::IOContext& AsioIOContextPool::GetIOContext() {
    auto& context = _ioContexts[_nextIOContextIndex++];
    if (_nextIOContextIndex == _ioContexts.size()) {
        _nextIOContextIndex = 0;
    }
    return context;
}

void AsioIOContextPool::Stop() {
    // 1. 先停止所有IO上下文
    for (auto& context : _ioContexts) {
        context.stop();
    }

    // 2. 释放工作守卫（允许线程退出）
    _workGuards.clear();

    // 3. 等待所有线程结束
    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    _threads.clear();
}
