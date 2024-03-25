#include <iostream>
#include <Connect_Pool.h>
#include <log.h>
using namespace std;
void test_connect(ConnectPool *pool) {
    LOG_DEBUG("pool = %p tid = %d",pool,pthread_self());
    MYSQL *mysql = nullptr;
    ConnectRAII(&mysql, pool);
    if (mysql == nullptr)
    {
        cout << "allocate connect failed" << std::endl;
    }
}
int main()
{
    CommonLog::Logger::GetInstance()->Init(CommonLog::LogType::LOG_PRINT, 0);
    ConnectPool *pool = ConnectPool::GetInstance();
    bool ret = false;
    //debian-sys-maint为本地sql的账户 3QTSj9lgwCgpOqqU本地密码 testdb本地数据库名字
    ret = pool->Init("localhost", "debian-sys-maint", "3QTSj9lgwCgpOqqU", "testdb", 3306, 10);
    if (ret == false)
    {
        cout << "Init  failed" << std::endl;
    }
    std::vector<std::thread> threads;
    for(int i= 0; i<20;i++) {
        threads.push_back(std::thread(test_connect,pool));
    }
    for(auto &i:threads) {
        i.join();
    }
    return 0;
}