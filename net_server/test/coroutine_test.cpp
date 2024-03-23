#include "coroutine.h"
#include <iostream>
#include <thread>

int main() {
    CoroTask task = CreateTask();
    std::cout << "This is main coroutine: " << std::this_thread::get_id() << std::endl;
    task.setCallback([&](){
        std::cout << "This is sub coroutine: " << std::this_thread::get_id() << std::endl;
    });
    task.resume();
    std::cout << "This is main coroutine: " << std::this_thread::get_id() << std::endl;
}