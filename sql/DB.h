/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */

#ifndef GATEWAY_DATA_PROCESS_DB_H
#define GATEWAY_DATA_PROCESS_DB_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "json_str_convertor.h"
#include "mysql_pool.h"
#include <spdlog/spdlog.h>

class JsonStrConvertor;

class DB {
public:
    ~DB();
    int insertData(JsonStrConvertor *pJsonStrConvertor, int send_status);
    void insertCmd(int data_id, const char *cmd);
//// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//    void insertCmdFromCloud(const char *cmd);
    void updateCmdStatus(const char *cmd, int status_type);
    bool queryIOStatus(std::string devAddr,  std::map<const std::string,std::vector<const char*> > &records);
    static DB *getDB();
private:
    DB();
    void migrate();
};


#endif //GATEWAY_DATA_PROCESS_DB_H
