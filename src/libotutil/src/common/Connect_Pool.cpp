#include "Connect_Pool.h"
#include <vector>
#include <log.h>


ConnectPool*ConnectPool::GetInstance()
{
    static ConnectPool connect_pool;
    return &connect_pool;
}

ConnectPool::~ConnectPool()
{
     LOG_DEBUG("size = %d",connect_list.size());
     for(auto i:connect_list) {
          mysql_close(i);
          i == nullptr;
     }
    free_connect_ = 0;
    use_connect_ = 0;
    connect_list.clear();
}
bool ConnectPool::Init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn)
{
    url_ = url, user_ = User, pass_word_ = PassWord, database_name_ = DataBaseName, port_ = Port, max_conn_ = MaxConn;
    for (int i = 0; i < max_conn_; i++)
    {

        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (sql == nullptr)
        {
            LOG_ERROR("MySQL Error");
            exit(1);
        }
       
        sql = mysql_real_connect(sql, url_.c_str(), user_.c_str(), pass_word_.c_str(), database_name_.c_str(), port_, nullptr, 0);

        if (sql == nullptr)
        {
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        connect_list.push_back(sql);
        ++free_connect_;
    }
    return true;
}

MYSQL *ConnectPool::get_sql_handle()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]()
             { return (!connect_list.empty()); });

    MYSQL *sql_ptr = connect_list.front();
    connect_list.pop_front();
    use_connect_++;
    free_connect_--;
    LOG_DEBUG("use_connect_ = %d,free_connect_ = %d",use_connect_,free_connect_);

    return sql_ptr;
}

bool ConnectPool::Realease_sql_handle(MYSQL *sql_ptr)
{

    if (sql_ptr == nullptr)
    {
        return false;
    }
    {
    std::lock_guard<std::mutex> lck(mtx_);
    connect_list.push_back(sql_ptr);
    use_connect_--;
    free_connect_++;
        LOG_DEBUG("use_connect_ = %d,free_connect_ = %d",use_connect_,free_connect_);
    }

    cv_.notify_one();
    return true;
}

static ConnectPool &GetInstance()
{
    static ConnectPool connect_pool;
    return connect_pool;
}

ConnectRAII::ConnectRAII(MYSQL **mysql, ConnectPool *connect_pool)
{
    *mysql = connect_pool->get_sql_handle();
    mysql_ = *mysql;
    connect_pool_ = connect_pool;
   
}
ConnectRAII::~ConnectRAII()
{
    connect_pool_->Realease_sql_handle(mysql_);
}