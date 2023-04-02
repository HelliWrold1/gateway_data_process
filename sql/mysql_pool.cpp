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
/**
 * 读取数据库的记录，放到一个map中
 * @param sql
 * @return 存储记录的map 类型：std::map< const std::string, std::vector<const char*>, ReadSqlMapCompare >
 */
std::unordered_map<std::string, std::vector<std::string>, sHash>  MysqlPool::readSql(const char* sql) {
    MYSQL* conn = getOneConnect();
    std::unordered_map<std::string, std::vector<std::string>, sHash> results;
    if (conn) {
        if (mysql_query(conn,sql) == 0) {
            MYSQL_RES *res = mysql_store_result(conn);
            if (res) {
                MYSQL_FIELD *field;
                std::vector<std::string> fields;
                while ((field = mysql_fetch_field(res))) {
                    fields.push_back(std::string(field->name));
                    results.insert(make_pair(field->name,std::vector<std::string>()));
                }
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    unsigned int i = 0;
                    for (int j = 0; j < fields.size(); ++j) {
                        results[fields[i]].push_back(std::string(row[i++]));
                    }
                }
                mysql_free_result(res);
            } else {
                if (mysql_field_count(conn) != 0)
                    SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            }
        } else {
            SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            SPDLOG_LOGGER_ERROR(logger, sql);
        }
        close(conn);
    } else {
        SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
    }
    return results;
}

/**
 * 执行插入sql
 * @param sql
 * @return 0: 插入失败 >0: 自增id
 */
int MysqlPool::createSql(const char* sql){
    MYSQL* conn = getOneConnect();
    int auto_id;
    if (conn) {
        if (mysql_query(conn,sql)) {
            SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger, sql);
            close(conn);
            return 0;
        }
        if (mysql_query(conn,"SELECT LAST_INSERT_ID();") == 0){
            SPDLOG_LOGGER_DEBUG(logger, sql);
            MYSQL_RES *res = mysql_store_result(conn);
            MYSQL_ROW row;
            row = mysql_fetch_row(res);
            auto_id = atoi(row[0]);
            close(conn);
            return auto_id;
        }
        else
        {
            SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger, sql);
            close(conn);
            return 0;
        }
    }
    return 0;
}

/**
 * 更新记录
 * @param sql
 * @return 0: 更新成功 1: 更新失败
 */
bool MysqlPool::updateSql(const char *sql) {
    MYSQL* conn = getOneConnect();
    if (conn) {
        if (mysql_query(conn,sql)) {
            SPDLOG_LOGGER_INFO(logger, sql);
            SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            close(conn);
            return 1;
        }
        close(conn);
        return 0;
    }
    return 1;
}

/**
 * 自动迁移
 * @param sql
 * @return true: 成功 false: 失败
 */
bool MysqlPool::migrateSql(const char* sql)
{
    MYSQL* conn = getOneConnect();
    if (conn) {
        if (mysql_query(conn, sql) == 0) {
            SPDLOG_LOGGER_DEBUG(logger, sql);
            close(conn);
            return true;
        } else {
            SPDLOG_LOGGER_ERROR(logger, mysql_error(conn));
            SPDLOG_LOGGER_INFO(logger, sql);
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



