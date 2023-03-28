/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */

#ifndef GATEWAY_DATA_PROCESS_DB_H
#define GATEWAY_DATA_PROCESS_DB_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "mysql_pool.h"
#include <spdlog/spdlog.h>

typedef struct sFactualIOStatus {
    bool io4 = false;
    bool io5 = false;
    bool io8 = false;
    bool io9 = false;
    bool io11 = false;
    bool io14 = false;
    bool io15 = false;
}FactualIOStatus_t;

class DB {
public:
    ~DB();
    int insertData(const char *frame, int send_status);
    void insertCmd(int data_id, char *cmd);
    bool queryIOStatus(std::string devAddr,  std::map<const std::string,std::vector<const char*> > &records);
    static DB *getDB();
private:
    DB();
    void migrate();
};


#endif //GATEWAY_DATA_PROCESS_DB_H
