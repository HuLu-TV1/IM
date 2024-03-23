#include "coroutine.h"

void Awaiter::await_suspend(auto hdl) const noexcept {
    CoroTask::promise_type promise = hdl.promise();
    if (promise.cb_) {
        promise.cb_();
    }
}

CoroTask CreateTask() {
    co_await Awaiter{};
}