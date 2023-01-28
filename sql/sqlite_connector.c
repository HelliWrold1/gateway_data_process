/*
 * Created by HelliWrold1 on 2023/1/28 0:48.
 */

#include <string.h>
#include <stdlib.h>
#include "sqlite_connector.h"

sqlite3* db;
char* errmsg; // 指向错误信息字符串的指针

int connectDatabase()
{
    // 连接数据库
    if (SQLITE_OK != sqlite3_open(DATABASE, &db))
    {
        GW_LOG(LOG_ERROR, sqlite3_errmsg(db));
        return SQLITE3_CONNECTOR_CONNECT_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Open database successful!");
        return SQLITE3_CONNECTOR_SUCCESS;
    }
}

int createTable(const char* table)
{
    static char sql[256];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "CREATE TABLE node_%s\
                 (id INTEGER PRIMARY KEY AUTOINCREMENT,\
                 json TEXT, sendStatus INTEGER, \
                 CHECK(json_valid(json)));", table);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg))
    {
        GW_LOG(LOG_ERROR,errmsg);
        return SQLITE3_CONNECTOR_CRETE_TABLE_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Create and open %s successfully!",table);
        return SQLITE3_CONNECTOR_SUCCESS;
    }
}

int insertTable(const char* table,const char* json_str)
{
    static char sql[1024];
    memset(sql,0,sizeof (sql));
    sprintf(sql, "INSERT INTO node_%s(json,sendStatus) values('%s',%d);",table,json_str,0);
    if (sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
    {
        GW_LOG(LOG_DEBUG,sql);
        GW_LOG(LOG_ERROR,errmsg);
        return SQLITE3_CONNECTOR_INSERT_TABLE_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Insert %s successfully!", json_str);
        return SQLITE3_CONNECTOR_SUCCESS;
    }
}

int queryTable(const char* table, SQLite3QueryResult_t* p_SQLite3QueryResult)
{
    static char sql[256];
    static char** result;
    static int row, column, index, i, j;
    memset(sql, 0, sizeof (sql));
    sprintf(sql,"SELECT id, json_extract(json,'$.fcnt'), json FROM node_%s WHERE id="
                        "(SELECT MIN(id) FROM node_%s WHERE json_extract(json, '$.fcnt')="
                        "(SELECT MIN(json_extract(json, '$.fcnt')) FROM node_%s WHERE sendStatus=0));",
                        table, table, table);
    if (SQLITE_OK != sqlite3_get_table(db, sql, &result, &row, &column, NULL))
    {
        GW_LOG(LOG_ERROR,"sqlite3_get_table:%s", sqlite3_errmsg(db));
        return SQLITE3_CONNECTOR_FAILURE;
    }
    else
    {
        p_SQLite3QueryResult->id = atoi(result[column]);
        p_SQLite3QueryResult->fcnt = atoi(result[column+1]);
        sprintf(p_SQLite3QueryResult->json,"%s",result[column+2]);
        sqlite3_free_table(result);
        return SQLITE3_CONNECTOR_SUCCESS; // 返回json
    }
}

int initQueryResult(SQLite3QueryResult_t* p_SQLite3QueryResult)
{
    memset(p_SQLite3QueryResult, 0, sizeof(SQLite3QueryResult_t));
}
