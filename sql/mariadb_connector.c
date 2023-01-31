/*
 * Created by HelliWrold1 on 2023/1/29 16:58.
 */

#include "mariadb_connector.h"

void initMariadbConnector(MariadbConnector_t* pMariadbConn)
{
    memset(pMariadbConn, 0, sizeof(MariadbConnector_t));
    mysql_init(&pMariadbConn->mysql);
    pMariadbConn->connInfo.ip = "localhost";
    pMariadbConn->connInfo.userName = "paho";
    pMariadbConn->connInfo.userPwd = "process";
    pMariadbConn->connInfo.db = "test";
    pMariadbConn->sql_timeout = 7;
}

void initMariadbQueryResult(MariadbQueryResult_t* pMariadbRes)
{
    memset(pMariadbRes, 0, sizeof(MariadbQueryResult_t));
}

int connectMariadb(MariadbConnector_t * pMariadbConn)
{
    if(mysql_options(&pMariadbConn->mysql,MYSQL_OPT_CONNECT_TIMEOUT,&pMariadbConn->sql_timeout))
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariadbConn->mysql));
//        GW_LOG(LOG_ERROR,"Options Set ERROR!");
        return MARIADB_CONNECTOR_CONNECT_FAILURE;
    }
    if(!mysql_real_connect(&pMariadbConn->mysql, pMariadbConn->connInfo.ip, pMariadbConn->connInfo.userName,
                   pMariadbConn->connInfo.userPwd, pMariadbConn->connInfo.db, 0, NULL, 0))
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariadbConn->mysql));
        mysql_close(&pMariadbConn->mysql);
//        GW_LOG(LOG_ERROR,"Connection Failed!");
        return MARIADB_CONNECTOR_CONNECT_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Mariadb connect database %s successfully.", pMariadbConn->mysql.db);
        return MARIADB_CONNECTOR_SUCCESS;
    }
}

int mariadbExistsTable(MariadbConnector_t* pMariadbConn)
{

}

int mariadbCreateTable(MariadbConnector_t* pMariadbConn)
{
    // 无则创建
    memset(pMariadbConn->sql, 0, sizeof(pMariadbConn->sql));
    sprintf(pMariadbConn->sql,"CREATE TABLE IF NOT EXISTS `node_%s` ("
                                  "`id` int auto_increment,"
                                  "`frame` json DEFAULT NULL CHECK (JSON_VALID(`frame`)),"
                                  "`sendStatus` tinyint(1) DEFAULT 1,"
                                  "PRIMARY KEY (`id`)"
                                  ")ENGINE=InnoDB DEFAULT CHARSET=utf8;",
                                  pMariadbConn->table);
    if(!mysql_query(&pMariadbConn->mysql, pMariadbConn->sql))
    {
        GW_LOG(LOG_DEBUG,"Mariadb create table %s successfully.",pMariadbConn->table);
        return MARIADB_CONNECTOR_SUCCESS;
    }
    else
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariadbConn->mysql));
        return MARIADB_CONNECTOR_CRETE_TABLE_FAILURE;
    }
}

int mariadbInsertRecord(MariadbConnector_t* pMariadbConn, const char *jsonStr, int sendStatus)
{
    memset(pMariadbConn->sql, 0, sizeof(pMariadbConn->sql));
    sprintf(pMariadbConn->sql,"INSERT INTO node_%s(frame, sendStatus) "
                                "VALUES(JSON_REMOVE(JSON_SET('%s','$.localtime',"
                                "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                                "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                                "%d)",
                                pMariadbConn->table, jsonStr, jsonStr,sendStatus);
    GW_LOG(LOG_DEBUG,"%s",pMariadbConn->sql);
    if(!mysql_query(&pMariadbConn->mysql, pMariadbConn->sql))
    {
        GW_LOG(LOG_DEBUG,"Mariadb insert %s successfully!", jsonStr);
        return MARIADB_CONNECTOR_SUCCESS;
    }
    else
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariadbConn->mysql));
        return MARIADB_CONNECTOR_INSERT_RECORD_FAILURE;
    }
}

int mariadbQueryRecord(MariadbConnector_t* pMariaConn, MariadbQueryResult_t* pMariadbRes)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    memset(pMariaConn->sql, 0, sizeof(pMariaConn->sql));
    sprintf(pMariaConn->sql, "SELECT id, frame FROM node_%s WHERE id="
                             "(SELECT MIN(id) FROM node_%s WHERE sendStatus=0 AND JSON_VALUE(frame,'$.localtime')="
                             "(SELECT MIN(JSON_VALUE(frame,'$.localtime')) from node_%s))",
                             pMariaConn->table, pMariaConn->table, pMariaConn->table);
    GW_LOG(LOG_DEBUG, pMariaConn->sql);
    if (!mysql_query(&pMariaConn->mysql, pMariaConn->sql))
    {
        GW_LOG(LOG_DEBUG,"Query Successful.");
        res = mysql_store_result(&pMariaConn->mysql);
        row = mysql_fetch_row(res);
        initMariadbQueryResult(pMariadbRes);
        pMariadbRes->id = atoi(row[0]);
        sprintf(pMariadbRes->frame,"%s",row[1]);
        mysql_free_result(res);
        return MARIADB_CONNECTOR_SUCCESS;
    }
    else
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariaConn->mysql));
        return MARIADB_CONNECTOR_QUERY_FAILURE;
    }
}

int mariadbReplaceTable(MariadbConnector_t* pMariaConn, MariadbQueryResult_t* pMariadbRes)
{
    memset(pMariaConn->sql, 0, sizeof(pMariaConn->sql));
    sprintf(pMariaConn->sql, "UPDATE node_%s set `sendStatus`=1 where id=%d",pMariaConn->table,pMariadbRes->id);

    if (!mysql_query(&pMariaConn->mysql, pMariaConn->sql))
    {
        GW_LOG(LOG_DEBUG, "Replaced id=%d.",pMariadbRes->id);
        return MARIADB_CONNECTOR_SUCCESS;
    }
    else
    {
        GW_LOG(LOG_ERROR, mysql_error(&pMariaConn->mysql));
        return MARIADB_CONNECTOR_REPLACE_FAILURE;
    }
}