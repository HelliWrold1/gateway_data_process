/*
 * Created by HelliWrold1 on 2023/3/20 14:37.
 */

#include "DB.h"
MysqlPool *mysql;
DB * db;

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
    mysql->migrateSql("CREATE TABLE IF NOT EXISTS `data` ("
                      "`id` int auto_increment,"
                      "`frame` json DEFAULT NULL CHECK (JSON_VALID(`frame`)),"
                      "`send_status` tinyint(1) DEFAULT 1,"
                      "PRIMARY KEY (`id`)"
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8;");
    mysql->migrateSql("CREATE TABLE IF NOT EXISTS `cmd` ("
                      "`id` int auto_increment,"
                      "`data_id` int,"
                      "`cmd` json DEFAULT NULL CHECK (JSON_VALID(`cmd`)),"
                      "`datetime` datetime,"
                      "PRIMARY KEY (`id`)"
                      ")ENGINE=InnoDB DEFAULT CHARSET=utf8;");
}

/**
 * 存储数据
 * @param frame
 * @param send_status
 * @return 自增id
 */
int DB::insertData(char* json_frame, int send_status)
{
    int auto_id;
    char sql[1024];
    sprintf(sql,"INSERT INTO data(frame, send_status) "
                "VALUES(JSON_REMOVE(JSON_SET('%s','$.localtime',"
                "DATE_ADD(STR_TO_DATE(JSON_VALUE('%s','$.datetime'),'%%Y-%%m-%%dT%%H:%%i:%%sZ'),"
                "INTERVAL 8 HOUR)),'$.codr','$.datr','$.desc','$.freq','$.lsnr','$.port','$.rssi'),"
                "%d)",json_frame,json_frame,send_status);
    auto_id = mysql->createSql(sql);
    GW_LOG(LOG_DEBUG,"[DB]: auto increment id:%d",auto_id);
    return auto_id;
}
/**
 * 存储网关下发的指令，与数据绑定
 * @param data_id
 * @param cmd
 */
void DB::insertCmd(int data_id, char* cmd)
{
    char sql[512];
    sprintf(sql,"INSERT INTO cmd(data_id, cmd) VALUES(%d,%s)",data_id,cmd);
    mysql->createSql(sql);
}