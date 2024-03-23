#pragma once
#include <coroutine>
#include <exception>
#include "common.h"

class [[nodiscard]] CoroTask {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;

public:
    CoroTask(auto h): hdl{h} {}
    ~CoroTask() {
        if (hdl) {
            hdl.destroy();
        }
    }
    DISALLOW_COPY_AND_ASSIGN(CoroTask)

    bool resume() const {
        if (!hdl || hdl.done()) {
            return false;
        }
        hdl.resume();
        return !hdl.done();
    }

    void setCallback(auto cb) {
        promise_type& promise = hdl.promise();
        promise.cb_ = cb;
}

private:
    CoroHdl hdl;
};

struct CoroTask::promise_type {
    auto get_return_object() {
        return CoroTask{CoroHdl::from_promise(*this)};
    }

    auto initial_suspend() {
        return std::suspend_always{};
    }

    void unhandled_exception() {
        std::terminate();
    }

    void return_void() {}

    auto final_suspend() noexcept {
        return std::suspend_always{};
    }

public:
    std::function<void()> cb_;
};

class Awaiter {
public:
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(auto hdl) const noexcept;

    void await_resume() const noexcept {}
};

CoroTask CreateTask();