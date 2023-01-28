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
 * @param table 表名
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，错误值@ref SQLITE3_CONNECTOR_STATUS
 */
int createTable(const char* table);
/**
 * @brief 向表中插入json字符串
 * @param table
 * @param json_str
 * @return 成功返回SQLITE3_CONNECTOR_SUCCESS，错误值@ref SQLITE3_CONNECTOR_STATUS
 */
int insertTable(const char* table,const char* json_str);

int queryTable(const char* table, SQLite3QueryResult_t* sqr);

int initQueryResult(SQLite3QueryResult_t* p_SQLite3QueryResult);

#endif //GATEWAY_DATA_PROCESS_SQLITE_CONNECTOR_H
