/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */


#include "DB.h"

MysqlPool *mysql;
DB * db;
static auto logger = spdlog::get("logger");
std::mutex DB::m_object_lock;

DB::DB() {
    mysql = MysqlPool::getMysqlPoolObject();
    this->migrate();
}

DB::~DB() {
    delete mysql;
}

DB* DB::getDB(void) {
    if (db== nullptr) {
        m_object_lock.lock();
        if(db == nullptr)
            db = new DB();
        m_object_lock.unlock();
    }
    return db;
}

void DB::migrate()
{
    mysql->setParameter("localhost","paho","process","test",0,NULL,0,30);
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
 * 存储节点数据
 * @param pJsonStrConvertor
 * @param send_status MQTT发送状态 0: 未发送 1: 已发送
 * @return 返回自增id
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
                                ")",
                                pJsonStrConvertor->parsedData.devaddr, pJsonStrConvertor->parsedData.datatype,
                                pJsonStrConvertor->str, pJsonStrConvertor->str, send_status);
    if (pJsonStrConvertor->parsedData.datatype == 0x1E)
        sprintf(sql, "INSERT INTO data(dev_addr, data_type, frame, send_status) "
                                 "VALUES("
                                 "\"%s\","
                                 "%d,"
                                 "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                                 "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                                 "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                                 "%d"
                                 ")",
                                pJsonStrConvertor->parsedData.devaddr,pJsonStrConvertor->parsedData.datatype,
                                pJsonStrConvertor->str, pJsonStrConvertor->str, send_status);
    SPDLOG_LOGGER_DEBUG(logger, sql);
    auto_id = mysql->createSql(sql);
    SPDLOG_LOGGER_DEBUG(logger,"Auto increment id:{}", auto_id);
    return auto_id;
}
/**
 * 插入和更新未执行且未发送的Command
 * @param data_id 记录ID
 * @param cmd 未执行且未发送的Command
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
    SPDLOG_LOGGER_DEBUG(logger, sql);
    mysql->createSql(sql);
}

/**
 * 查询某个控制节点的IO状态
 * @param devAddr 节点ID
 * @param records 接收记录的map
 * @return true: 查询成功 false: 查询失败
 */
bool DB::queryIOStatus(std::string devAddr, std::unordered_map<std::string, std::vector<std::string>, sHash> &records) {
    char sql[4096];
    sprintf(sql, "SELECT frame FROM data WHERE dev_addr=\"%s\" AND data_type=1 ORDER BY id DESC LIMIT 1", devAddr.data());
    SPDLOG_LOGGER_DEBUG(logger, sql);
    records = mysql->readSql(sql);
    if (records["frame"].empty()){
        SPDLOG_LOGGER_DEBUG(logger, "false");
        return false;
    }
    else{
        SPDLOG_LOGGER_DEBUG(logger, "{}", records["frame"][0]);
        SPDLOG_LOGGER_DEBUG(logger, "true");
        return true;
    }
}

/**
 * 查询未发送的数据
 * @param records 接收查询到的传感器数据
 * @return true: 查询成功 false: 查询失败
 */
bool DB::queryUnsentData(std::unordered_map<std::string, std::vector<std::string>, sHash> &records) {
    char sql[4096];
    sprintf(sql, "SELECT id, frame FROM data WHERE send_status=0 AND datatype<2");
    SPDLOG_LOGGER_DEBUG(logger, sql);
    records = mysql->readSql(sql);
    if (records["frame"].empty()) {
        SPDLOG_LOGGER_DEBUG(logger, "false");
        return false;
    } else {
        SPDLOG_LOGGER_DEBUG(logger, "{}", records["frame"][0]);
        SPDLOG_LOGGER_DEBUG(logger, "true");
        return true;
    }
}

/**
 * 查询超时30s未执行的Command
 * @param records 接收查询到的未执行命令数据
 * @return true: 查询成功 false : 查询失败
 */
bool DB::queryUnexecutedCmd(std::unordered_map<std::string, std::vector<std::string>, sHash> &records) {
    char sql[4096];
    sprintf(sql, "SELECT id, cmd FROM cmd WHERE exe_status=0 AND datetime <= DATE_ADD(NOW(), INTERVAL -30 SECOND)");
    SPDLOG_LOGGER_DEBUG(logger, sql);
    records = mysql->readSql(sql);
    if (records["cmd"].empty()) {
        SPDLOG_LOGGER_DEBUG(logger, "false");
        return false;
    } else {
        SPDLOG_LOGGER_DEBUG(logger, "{}", records["cmd"][0]);
        SPDLOG_LOGGER_DEBUG(logger, "true");
        return true;
    }
}

/**
 * 查询已执行，但是未通过MQTT发送的Command
 * @param records 接收Commands
 * @return true: 查询成功 false : 查询失败
 */
bool DB::queryUnSentCmd(std::unordered_map<std::string, std::vector<std::string>, sHash> &records) {
    char sql[4096];
    sprintf(sql, "SELECT id, cmd FROM cmd WHERE exe_status=1 AND send_status=0");
    SPDLOG_LOGGER_DEBUG(logger, sql);
    records = mysql->readSql(sql);
    if (records["cmd"].empty()) {
        SPDLOG_LOGGER_DEBUG(logger, "false");
        return false;
    } else {
        SPDLOG_LOGGER_DEBUG(logger, "{}", records["cmd"][0]);
        SPDLOG_LOGGER_DEBUG(logger, "true");
        return true;
    }
}

/**
 * 将已经通过MQTT发送的数据标记为已发送
 * @param id
 * @return true: 更新成功 false: 更新失败
 */
bool DB::updateDataSendStatus(int id) {
    char sql[4096];
    sprintf(sql, "UPDATE data SET send_status=1 WHERE id=%d AND send_status=0", id);
    SPDLOG_LOGGER_DEBUG(logger, sql);
    if (mysql->updateSql(sql))
        return false;
    else
        return true;
}

/**
 * 改变执行状态或发送状态-->已执行或已发送
 * @param cmd 被更新的指令记录的指令内容
 * @param status_type 0: 更改执行状态 1: 更改发送状态
 * @return true: 更改成功 false: 更改失败
 */
bool DB::updateCmdStatus(const char *cmd, int status_type) {
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
    SPDLOG_LOGGER_DEBUG(logger, sql);
    if (mysql->updateSql(sql))
        return false;
    else
        return true;
}

/**
 * 更新指令的时间戳，用于重发判断
 * @param id
 * @return true: 更新成功 false: 更新失败
 */
bool DB::updateCmdDatetime(int id) {
    char sql[4096];
    sprintf(sql, "UPDATE cmd SET datetime=NOW() WHERE id=%d", id);
    SPDLOG_LOGGER_DEBUG(logger, sql);
    if (mysql->updateSql(sql))
        return false;
    else
        return true;
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
