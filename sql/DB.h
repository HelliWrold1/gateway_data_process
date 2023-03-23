/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */

#ifndef GATEWAY_DATA_PROCESS_DB_H
#define GATEWAY_DATA_PROCESS_DB_H

#include "mysql_pool.h"

class DB {
public:
    ~DB();
    int insertData(char *frame, int send_status);
    void insertCmd(int data_id, char *cmd);
    static DB *getDB();
private:
    DB();
    void migrate();
};


#endif //GATEWAY_DATA_PROCESS_DB_H
