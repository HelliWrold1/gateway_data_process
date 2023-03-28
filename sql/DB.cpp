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
    mysql->createSql(R"(CREATE TABLE IF NOT EXISTS `data`  (
                      `id` int(11) NOT NULL AUTO_INCREMENT,
                      `devaddr` varchar(255) CHARACTER SET utf8 NOT NULL,
                      `frame` json DEFAULT NULL CHECK (JSON_VALID(`frame`)),
                      `send_status` tinyint(1) NULL DEFAULT 1,
                      PRIMARY KEY (`id`) USING BTREE,
                      INDEX `devaddr`(`devaddr`) USING BTREE
                    ) ENGINE = InnoDB  CHARACTER SET = utf8;)");

    mysql->migrateSql("CREATE TABLE IF NOT EXISTS `cmd` ("
                      "`id` int auto_increment,"
                      "`data_id` int,"
                      "`cmd` json DEFAULT NULL CHECK (JSON_VALID(`cmd`)),"
                      "`datetime` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,"
                      "PRIMARY KEY (`id`)"
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8;");
}

/**
 * 存储数据
 * @param frame
 * @param send_status
 * @return 自增id
 */
int DB::insertData(const char* json_frame, int send_status)
{ // TODO 必须保证control node 数据唯一性，用于指示状态
    int auto_id;
    char sql[4096];
    sprintf(sql,"INSERT INTO data(devaddr, frame, send_status) "
                            "VALUES("
                            "JSON_VALUE('%s', '$.devaddr'),"
                            "JSON_REMOVE(JSON_SET('%s','$.localtime',"
                            "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                            "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                            "%d"
                            ")",
                            json_frame, json_frame, json_frame, send_status);
    auto_id = mysql->createSql(sql);
    SPDLOG_LOGGER_DEBUG(logger,"[DB]: auto increment id:{}", auto_id);
    return auto_id;
}
/**
 * 存储网关下发的指令，与数据绑定
 * @param data_id
 * @param cmd
 */
void DB::insertCmd(int data_id, char* cmd)
{
    char sql[4096];
    sprintf(sql,"INSERT INTO cmd(data_id, cmd) VALUES(%d, %s)", data_id, cmd);
    mysql->createSql(sql);
}

bool DB::queryIOStatus(std::string devAddr, std::map<const std::string,std::vector<const char*> > &records) {
    char sql[4096];
    sprintf(sql, "SELECT frame FROM data WHERE devaddr=\"%s\"", devAddr.data());
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
