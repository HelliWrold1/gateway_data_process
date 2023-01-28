/*
 * Created by HelliWrold1 on 2023/1/28 0:48.
 */

#ifndef GATEWAY_DATA_PROCESS_SQLITE_CONNECTOR_H
#define GATEWAY_DATA_PROCESS_SQLITE_CONNECTOR_H

#include <sqlite3.h>
#include "log.h"

#define DATABASE "GatewayData.db"

typedef enum {
    SQLITE3_CONNECTOR_SUCCESS,
    SQLITE3_CONNECTOR_FAILURE,
    SQLITE3_CONNECTOR_CRETE_TABLE_FAILURE,
    SQLITE3_CONNECTOR_INSERT_TABLE_FAILURE,
    SQLITE3_CONNECTOR_CONNECT_FAILURE,
}SQLITE3_CONNECTOR_STATUS;


typedef struct sSQLite3QueryResult
{
    int id;
    int fcnt;
    char json[1024];
}SQLite3QueryResult_t;

/**
 * @brief 连接sqlite3数据库
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，错误值@ref SQLITE3_CONNECTOR_STATUS
 */
int connectDatabase();

/**
 * @brief 创建表
 * @param table 表名，只需要提供DevAddr即可
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，错误值@ref SQLITE3_CONNECTOR_STATUS
 */
int createTable(const char* table);

/**
 * @brief 向表中插入json字符串
 * @param table 被插入的表名，只需要提供DevAddr即可
 * @param json_str 被插入的json字符串（必须是符合json格式的字符串，不能多于回车符等奇怪的字符）
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，错误值@ref SQLITE3_CONNECTOR_STATUS
 */
int insertTable(const char* table,const char* json_str);

/**
 * @brief 查询未发送的fcnt最小的数据中id最小的记录，将id、fcnt、json存入sqr指向的结果结构体
 * @param table 被查询的表名，只需要提供DevAddr即可
 * @param sqr 指向结果结构体的指针
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，失败返回SQLITE3_CONNECTOR_FAILURE
 */
int queryTable(const char* table, SQLite3QueryResult_t* sqr);

/**
 * 初始化结果结构体
 * @param p_SQLite3QueryResult 指向结果结构体的指针
 * @return 初始化完毕返回SQLITE3_CONNECTOR_SUCCESS
 */
int initQueryResult(SQLite3QueryResult_t* p_SQLite3QueryResult);

/**
 * 将被发送的数据的发送状态设置为已发送
 * @param table 被更改的表名，只需要提供DevAddr即可
 * @param p_SQLite3QueryResult 指向被发送数据所在的结果结构体
 * @return 更改成功返回SQLITE3_CONNECTOR_SUCCESS，失败返回SQLITE3_CONNECTOR_FAILURE
 */
int replaceTable(const char* table, SQLite3QueryResult_t* p_SQLite3QueryResult);

#endif //GATEWAY_DATA_PROCESS_SQLITE_CONNECTOR_H
