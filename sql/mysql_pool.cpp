/*
 * Created by HelliWrold1 on 2023/3/20 12:09.
 */

#include "mysql_pool.h"

static auto logger = spdlog::get("logger");

MysqlPool* MysqlPool::mysqlpool_object = NULL;
std::mutex MysqlPool::objectlock;
std::mutex MysqlPool::poollock;

MysqlPool::MysqlPool() {
    if (0 == mysql_library_init(0, NULL, NULL))
        SPDLOG_LOGGER_DEBUG(logger,"mysql_library_init()succeed");
    else
        SPDLOG_LOGGER_ERROR(logger,"mysql_library_init()failed");
    connect_count = 0;
}

/*
 *配置数据库参数
 */
void MysqlPool::setParameter( const char*   mysqlhost,
                              const char*   mysqluser,
                              const char*   mysqlpwd,
                              const char*   databasename,
                              unsigned int  port,
                              const char*   socket,
                              unsigned long client_flag,
                              unsigned int  max_connect ) {
    _mysqlhost    = mysqlhost;
    _mysqluser    = mysqluser;
    _mysqlpwd     = mysqlpwd;
    _databasename = databasename;
    _port         = port;
    _socket       = socket;
    _client_flag  = client_flag;
    MAX_CONNECT   = max_connect;
}

/*
 *有参的单例函数，用于第一次获取连接池对象，初始化数据库信息。
 */
MysqlPool* MysqlPool::getMysqlPoolObject() {
    if (mysqlpool_object == nullptr) {
        objectlock.lock();
        if (mysqlpool_object == nullptr) {
            mysqlpool_object = new MysqlPool();
        }
        objectlock.unlock();
    }
    return mysqlpool_object;
}

/*
 *创建一个连接对象
 */
MYSQL* MysqlPool::createOneConnect() {
    MYSQL* conn = NULL;
    conn = mysql_init(conn);
    if (conn != NULL) {
        if (mysql_real_connect(conn,
                               _mysqlhost,
                               _mysqluser,
                               _mysqlpwd,
                               _databasename,
                               _port,
                               _socket,
                               _client_flag)) {
            connect_count++;
            return conn;
        } else {
            SPDLOG_LOGGER_ERROR(logger,mysql_error(conn));
            return NULL;
        }
    } else {
        SPDLOG_LOGGER_ERROR(logger,"mysql conn init failed");
        return NULL;
    }
}

/*
 *判断当前MySQL连接池的是否空
 */
bool MysqlPool::isEmpty() {
    return mysqlpool.empty();
}
/*
 *获取当前连接池队列的队头
 */
MYSQL* MysqlPool::poolFront() {
    return mysqlpool.front();
}
/*
 *
 */
unsigned int MysqlPool::poolSize() {
    return mysqlpool.size();
}
/*
 *弹出当前连接池队列的队头
 */
void MysqlPool::poolPop() {
    mysqlpool.pop();
}
/*
 *获取连接对象，如果连接池中有连接，就取用;没有，就重新创建一个连接对象。
 *同时注意到MySQL的连接的时效性，即在连接队列中,连接对象在超过一定的时间后没有进行操作，
 *MySQL会自动关闭连接，当然还有其他原因，比如：网络不稳定，带来的连接中断。
 *所以在获取连接对象前，需要先判断连接池中连接对象是否有效。
 *考虑到数据库同时建立的连接数量有限制，在创建新连接需提前判断当前开启的连接数不超过设定值。
 */
MYSQL* MysqlPool::getOneConnect() {
    poollock.lock();
    MYSQL *conn = NULL;
    if (!isEmpty()) {
        while (!isEmpty() && mysql_ping(poolFront())) {
            mysql_close(poolFront());
            poolPop();
            connect_count--;
        }
        if (!isEmpty()) {
            conn = poolFront();
            poolPop();
        } else {
            if (connect_count < MAX_CONNECT)
                conn = createOneConnect();
            else
                SPDLOG_LOGGER_ERROR(logger,"the number of mysql connections is too much!");
        }
    } else {
        if (connect_count < MAX_CONNECT)
            conn = createOneConnect();
        else
            SPDLOG_LOGGER_ERROR(logger,"the number of mysql connections is too much!");
    }
    poollock.unlock();
    return conn;
}
/*
 *将有效的链接对象放回链接池队列中，以待下次的取用。
 */
void MysqlPool::close(MYSQL* conn) {
    if (conn != NULL) {
        poollock.lock();
        mysqlpool.push(conn);
        poollock.unlock();
    }
}
/*
 * sql语句执行函数，并返回结果，没有结果的SQL语句返回空结果，
 * 每次执行SQL语句都会先去连接队列中去一个连接对象，
 * 执行完SQL语句，就把连接对象放回连接池队列中。
 * 返回对象用map主要考虑，用户可以通过数据库字段，直接获得查询的字。
 * 例如：m["字段"][index]。
 */
std::map< const std::string, std::vector<const char*> >  MysqlPool::readSql(const char* sql) {
    MYSQL* conn = getOneConnect();
    std::map<const std::string,std::vector<const char*> > results;
    if (conn) {
        if (mysql_query(conn,sql) == 0) {
            MYSQL_RES *res = mysql_store_result(conn);
            if (res) {
                MYSQL_FIELD *field;
                while ((field = mysql_fetch_field(res))) {
                    results.insert(make_pair(field->name,std::vector<const char*>()));
                }
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    unsigned int i = 0;
                    for (std::map<const std::string,std::vector<const char*> >::iterator it = results.begin();
                         it != results.end(); ++it) {
                        (it->second).push_back(row[i++]);
                    }
                }
                mysql_free_result(res);
                SPDLOG_LOGGER_DEBUG(logger,"[DB]: {}", sql);
            } else {
                if (mysql_field_count(conn) != 0)
                    SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
            }
        } else {
            SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
            SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", sql);
        }
        close(conn);
    } else {
        SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
    }
    return results;
}

/**
 * 执行插入sql
 * @param sql
 * @return 自增id
 */
int MysqlPool::createSql(const char* sql){
    MYSQL* conn = getOneConnect();
    int auto_id;
    if (conn) {
        if (mysql_query(conn,sql) == 0) {
            SPDLOG_LOGGER_DEBUG(logger,"[DB]: {}", sql);
        }
        else{
            SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger,"[DB]: {}", sql);
        }
        if (mysql_query(conn,"SELECT LAST_INSERT_ID();") == 0){
            SPDLOG_LOGGER_DEBUG(logger,"[DB]: {}", sql);
            MYSQL_RES *res = mysql_store_result(conn);
            MYSQL_ROW row;
            row = mysql_fetch_row(res);
            auto_id = atoi(row[0]);
            close(conn);
            return auto_id;
        }
        else
        {
            SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger,"[DB]: {}", sql);
        }
        close(conn);
        return 0;
    }
    return 0;
}

bool MysqlPool::updateSql(const char *sql) {
    MYSQL* conn = getOneConnect();
    if (conn) {
        if (mysql_query(conn,sql) == 0) {
            SPDLOG_LOGGER_DEBUG(logger,"[DB]: {}", sql);
        }
        else{
            SPDLOG_LOGGER_ERROR(logger,"[DB]: {}", mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger,"[DB]: {}", sql);
        }
        close(conn);
        return 0;
    }
    return 0;
}

bool MysqlPool::migrateSql(const char* sql)
{
    MYSQL* conn = getOneConnect();
    if (conn) {
        if (mysql_query(conn, sql) == 0) {
            SPDLOG_LOGGER_DEBUG(logger, "[DB]:{}", sql);
            return true;
        } else {
            SPDLOG_LOGGER_ERROR(logger, "[DB]: {}", mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger,"[DB]: {}", sql);
            close(conn);
            return false;
        }
    }
    return false;
}

/*
 * 析构函数，将连接池队列中的连接全部关闭
 */
MysqlPool::~MysqlPool() {
    while (poolSize() != 0) {
        mysql_close(poolFront());
        poolPop();
        connect_count--;
    }
    mysql_library_end();
}



