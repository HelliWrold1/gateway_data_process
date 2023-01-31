/*
 * Created by HelliWrold1 on 2023/1/29 16:58.
 */

#ifndef GATEWAY_DATA_PROCESS_MARIADB_CONNECTOR_H
#define GATEWAY_DATA_PROCESS_MARIADB_CONNECTOR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include "mysql/mysql.h"
#include "log.h"
#include <time.h>

typedef enum {
    MARIADB_CONNECTOR_SUCCESS,
    MARIADB_CONNECTOR_FAILURE,
    MARIADB_CONNECTOR_CONNECT_FAILURE,
    MARIADB_CONNECTOR_CRETE_TABLE_FAILURE,
    MARIADB_CONNECTOR_INSERT_RECORD_FAILURE,
    MARIADB_CONNECTOR_QUERY_FAILURE,
    MARIADB_CONNECTOR_REPLACE_FAILURE,
} MARIADB_CONNECTOR_STATUS;

typedef struct sMariadbConnector {
    MYSQL mysql;
    struct sMariadbConnectorInfo {
        const char *ip;
        const char *userName;
        const char *userPwd;
        const char *db;
    } connInfo;
    unsigned int sql_timeout;
    const char *table;
    char sql[1024];
} MariadbConnector_t;

typedef struct sMariadbQueryResult {
    int id;
    char frame[1024];
} MariadbQueryResult_t;

void initMariadbConnector(MariadbConnector_t *pMariadbConn);

void initMariadbQueryResult(MariadbQueryResult_t *pMariadbRes);

int connectMariadb(MariadbConnector_t *pMariadbConn);

int mariadbCreateTable(MariadbConnector_t *pMariadbConn);

int mariadbInsertRecord(MariadbConnector_t* pMariadbConn, const char *jsonStr, int sendStatus);

int mariadbQueryRecord(MariadbConnector_t *pMariaConn, MariadbQueryResult_t *pMariadbRes);

int mariadbReplaceTable(MariadbConnector_t *pMariaConn, MariadbQueryResult_t *pMariadbRes);

#if defined(__cplusplus)
}
#endif

#endif //GATEWAY_DATA_PROCESS_MARIADB_CONNECTOR_H
