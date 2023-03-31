/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */


#include "DB.h"

MysqlPool *mysql;
DB * db;
static auto logger = spdlog::get("logger");

DB::DB()
{
    mysql = MysqlPool::getMysqlPoolObject();
    mysql->setParameter("localhost","paho","process","test",0,NULL,0,100);
    this->migrate();
}

DB::~DB()
{
    delete mysql;
}

DB* DB::getDB(void)
{
    if (db== nullptr) {
        db = new DB();
    }
    return db;
}

void DB::migrate()
{
    mysql->migrateSql(R"(CREATE TABLE IF NOT EXISTS `data`  (
                            `id` int(11) NOT NULL AUTO_INCREMENT,
                            `dev_addr` varchar(255) CHARACTER SET utf8 NOT NULL,
                            `data_type` int(11) NOT NULL,
                            `frame` json DEFAULT NULL CHECK (JSON_VALID(`frame`)),
                            `send_status` tinyint(1) NULL DEFAULT 1,
                            PRIMARY KEY (`id`) USING BTREE,
                            INDEX `dev_addr`(`dev_addr`) USING BTREE,
                            INDEX `data_type`(`data_type`) USING BTREE,
                            UNIQUE INDEX `frame`(`frame`(512)) USING BTREE
                            ) ENGINE = InnoDB  CHARACTER SET = utf8;)");

    mysql->migrateSql("CREATE TABLE IF NOT EXISTS `cmd` ("
                      "`id` int auto_increment,"
                      "`data_id` int,"
                      "`cmd` json DEFAULT NULL CHECK (JSON_VALID(`cmd`)), "
                      "`datetime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
                      "`exe_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0, "
                      "`send_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0,"
                      "PRIMARY KEY (`id`) USING BTREE, "
                      "INDEX `exe_status`(`exe_status`) USING BTREE, "
                      "INDEX `send_status`(`send_status`) USING BTREE,"
                      "INDEX `datetime`(`datetime`) USING BTREE, "
                      "UNIQUE INDEX `cmd`(`cmd`(100)) USING BTREE"
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8;");
}

/**
 * 存储数据
 * @param frame
 * @param send_status
 * @return 自增id
 */
int DB::insertData(JsonStrConvertor *pJsonStrConvertor, int send_status)
{
    int auto_id;
    char sql[4096];
    // 判断数据类型，选择插入的方式
    if (pJsonStrConvertor->parsedData.datatype == 0x00)
        sprintf(sql,"INSERT INTO data(dev_addr, data_type, frame, send_status) "
                                "VALUES("
                                "\"%s\","
                                "%d,"
                                "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                                "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                                "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                                "%d"
                                ")",
                                pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                                pJsonStrConvertor->str, pJsonStrConvertor->str, send_status);
    // 查询当前IO或当前下位机上传间隔是否存在，存在则更新，不存在则插入
    if (pJsonStrConvertor->parsedData.datatype == 0x01)
        sprintf(sql,"INSERT INTO data(dev_addr, data_type, frame, send_status) "
                    "VALUES("
                    "\"%s\","
                    "%d,"
                    "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                    "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                    "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                    "%d"
                    ")"
                    "ON DUPLICATE KEY UPDATE "
                    "frame ="
                    "IF("
                    "dev_addr=\"%s\" "
                    "AND "
                    "data_type=%d, "
                    "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                    "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                    "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                    "frame"
                    "), "
                    "send_status = "
                    "IF("
                    "dev_addr=\"%s\" "
                    "AND "
                    "data_type=%d, "
                    "%d, "
                    "send_status"
                    ")",
                    pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                    pJsonStrConvertor->str, pJsonStrConvertor->str, send_status,
                    pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                    pJsonStrConvertor->str, pJsonStrConvertor->str, pJsonStrConvertor->parsedData.devaddr,
                    pJsonStrConvertor->parsedData.datatype, send_status);
    if (pJsonStrConvertor->parsedData.datatype == 0x1E)
        sprintf(sql, "INSERT INTO data(dev_addr, data_type, frame, send_status) "
                     "VALUES("
                     "\"%s\","
                     "%d,"
                     "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                     "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                     "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                     "%d"
                     ")"
                     "ON DUPLICATE KEY UPDATE "
                     "frame ="
                     "IF("
                     "dev_addr=\"%s\" "
                     "AND "
                     "data_type=%d, "
                     "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                     "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                     "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                     "frame"
                     "), "
                     "send_status = "
                     "IF("
                     "dev_addr=\"%s\" "
                     "AND "
                     "data_type=%d, "
                     "%d, "
                     "send_status"
                     ")",
                    pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                    pJsonStrConvertor->str, pJsonStrConvertor->str, send_status,
                    pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                    pJsonStrConvertor->str, pJsonStrConvertor->str, pJsonStrConvertor->parsedData.devaddr,
                    pJsonStrConvertor->parsedData.datatype, send_status);
    auto_id = mysql->createSql(sql);
    SPDLOG_LOGGER_DEBUG(logger,"[DB]: auto increment id:{}", auto_id);
    return auto_id;
}
/**
 * 插入和更新未执行且未发送的cmd
 * @param data_id
 * @param cmd
 * @param exe_status
 * @param send_status
 */
void DB::insertCmd(int data_id, const char *cmd)
{
    char sql[4096];
    sprintf(sql,"INSERT INTO cmd(data_id, cmd, exe_status, send_status) "
                "VALUES(%d, '%s', 0, 0) "
                "ON DUPLICATE KEY UPDATE"
                "`datetime` = "
                "IF("
                "datetime<NOW(), "
                "NOW(), "
                "datetime"
                ")"
            , data_id, cmd);
    mysql->createSql(sql);
}

/**
 * 改变执行状态或发送状态-->已执行或已发送
 * @param cmd 被更新的指令记录的指令内容
 * @param status_type 0:更改执行状态 1:更改发送状态
 */
void DB::updateCmdStatus(const char *cmd, int status_type) {
    char sql[4096];
    if (status_type == 0) { // 将执行状态改为已执行
        sprintf(sql, "UPDATE cmd "
                        "SET exe_status = %d WHERE cmd='%s' AND exe_status=0",
                        1, cmd);
    } else { // 将已经执行的cmd的发送状态改为已发送
        sprintf(sql, "UPDATE cmd "
                        "SET send_status = %d WHERE cmd='%s' AND exe_status=1",
                        1, cmd);
    }
    mysql->updateSql(sql);
}

bool DB::queryIOStatus(std::string devAddr, std::map<const std::string,std::vector<const char*> > &records) {
    char sql[4096];
    sprintf(sql, "SELECT frame FROM data WHERE dev_addr=\"%s\" AND data_type=1", devAddr.data());
    records = mysql->readSql(sql);
    if (records["frame"].empty()){
        SPDLOG_LOGGER_DEBUG(logger, "false");
        return false;
    }
    else{
        SPDLOG_LOGGER_DEBUG(logger, "true");
        return true;
    }
}

//// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//void DB::insertCmdFromCloud(const char *cmd) {
//    char sql[4096];
//    sprintf(sql, "INSERT INTO cmd(data_id, cmd, exe_status, send_status) "
//            "VALUES(%d, '%s', %d, %d) "
//            "ON DUPLICATE KEY UPDATE "
//            "`datetime` = "
//            "IF("
//            "datetime<NOW(), "
//            "NOW(),"
//            "datetime "
//            ")", 0, cmd, 0, 1);
//    mysql->createSql(sql);
//}
