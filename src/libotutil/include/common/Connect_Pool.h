#ifndef __CONNECT_SQL_POLL__
#define __CONNECT_SQL_POLL__
#include <mutex>
#include <mysql/mysql.h>
#include <list>
#include <memory>
#include <string>
#include <condition_variable>
#include <atomic>
class ConnectPool {
    public:
    ConnectPool(){};  // Private constructor to enforce singleton pattern
    ConnectPool(const ConnectPool&) = delete; // Disable copy constructor
    ConnectPool& operator=(const ConnectPool&) = delete; // Disable assignment operator
    ~ConnectPool();
    bool Init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn);
    MYSQL* get_sql_handle();
    bool Realease_sql_handle(MYSQL*);

    static ConnectPool* GetInstance();
    private:
    std::string url_;
    std::string user_;
    std::string pass_word_;
    std::string database_name_;
    int port_;
    int max_conn_;
    std::list<MYSQL*> connect_list;
    std::mutex  mtx_;
    int free_connect_;
    int use_connect_;
    std::condition_variable cv_;

};


class ConnectRAII {
     public:
     ConnectRAII(MYSQL**mysql,ConnectPool*connect_pool);
     ~ConnectRAII();
     private:
     MYSQL* mysql_;
     ConnectPool*connect_pool_;
};
#endif