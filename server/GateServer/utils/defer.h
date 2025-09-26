//
// Created by 任兀华 on 2025/9/8.
//

#ifndef GATESERVER_DEFER_H
#define GATESERVER_DEFER_H

#pragma once
#include <functional>

// RAII 辅助类：在作用域结束时执行清理函数
class Defer {
public:
    explicit Defer(std::function<void()> func) : func_(std::move(func)) {}
    ~Defer() { if (func_) func_(); }

    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

private:
    std::function<void()> func_;
};

#endif //GATESERVER_DEFER_H
