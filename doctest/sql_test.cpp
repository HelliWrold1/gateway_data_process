/*
 * Created by HelliWrold1 on 2023/1/30 21:24.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/sinks/stdout_color_sinks.h>
#include "doctest.h"
#include "DB.h"

/**
 * Before run this example, create database test and drop table test.cmd and test.data. mysql username=root password=root
 */
TEST_CASE("TEST DB")
{
    // set log
    auto logger  = spdlog::get("logger");
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] [%!]: %v");
    logger->set_level(spdlog::level::debug);
    // get db
    DB* db = DB::getDB();
    int id;
    bool queryFlag;
    std::string devaddr("B54E453C");
    std::map<const std::string,std::vector<const char*> > frame;
    char sensor_data[] = R"({
      "app": "Raspiber-handler",
      "battery": 0,
      "codr": "4/5",
      "data": "0000210020001402EF014000C9001D9F",
      "data1": 33,
      "data2": 32,
      "data3": 20,
      "data4": 751,
      "data5": 320,
      "data6": 201,
      "data7": 7583,
      "datatype": 0,
      "datetime": "2023-03-26T06:15:29Z",
      "datr": "SF10BW125",
      "desc": "Collection",
      "devaddr": "3EB4A376",
      "fcnt": 108,
      "freq": 474.5,
      "lsnr": -7.5,
      "mac": "B827EBFFFE2114B5",
      "port": 2,
      "rssi": -107
    })";

    char time_data[] = {"{\n"
                        "  \"app\": \"Raspiber-handler\",\n"
                        "  \"battery\": 0,\n"
                        "  \"codr\": \"4/5\",\n"
                        "  \"data\": \"01000000000000000100000000000000\",\n"
                        "  \"data1\": 0,\n"
                        "  \"data2\": 0,\n"
                        "  \"data3\": 30,\n"
                        "  \"data4\": 0,\n"
                        "  \"data5\": 0,\n"
                        "  \"data6\": 0,\n"
                        "  \"data7\": 0,\n"
                        "  \"datatype\": 30,\n"
                        "  \"datetime\": \"2023-03-26T09:12:02Z\",\n"
                        "  \"datr\": \"SF12BW125\",\n"
                        "  \"desc\": \"Control\",\n"
                        "  \"devaddr\": \"B54E453C\",\n"
                        "  \"fcnt\": 8,\n"
                        "  \"freq\": 475.7,\n"
                        "  \"lsnr\": -18.5,\n"
                        "  \"mac\": \"B827EBFFFE2114B5\",\n"
                        "  \"port\": 2,\n"
                        "  \"rssi\": -101\n"
                        "}"};

    char cmd_data[] = {R"({ "devaddr":"B54E453C", "data":"FA5F", "confirmed":true, "port":2, "time":"immediately" })"};
    char time_cmd_data[] = {R"({ "devaddr":"B54E453C", "data":"1E00001E", "confirmed":true, "port":2, "time":"immediately" })"};
    JsonStrConvertor *pJsonStrConvertor = new JsonStrConvertor(sensor_data);
    SUBCASE("test insert data")
    {
        id = db->insertData(pJsonStrConvertor,1);
        CHECK(id != 0);
        pJsonStrConvertor->parseNodeUplink(time_data);
        id = db->insertData(pJsonStrConvertor,1);
        db->insertCmd(id, cmd_data);
//// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//        db->insertCmdFromCloud(time_cmd_data);
    }

    SUBCASE("test query data")
    {
        char ctl_data[] = R"({
          "app": "Raspiber-handler",
          "battery": 0,
          "codr": "4/5",
          "data": "01000000000000000000000000000000",
          "data1": 0,
          "data2": 0,
          "data3": 0,
          "data4": 0,
          "data5": 0,
          "data6": 0,
          "data7": 0,
          "datatype": 1,
          "datetime": "2023-03-26T06:17:26Z",
          "datr": "SF12BW125",
          "desc": "Control",
          "devaddr": "B54E453C",
          "fcnt": 0,
          "freq": 474.9,
          "lsnr": -12.2,
          "mac": "B827EBFFFE2114B5",
          "port": 2,
          "rssi": -109
        })";
        pJsonStrConvertor->parseNodeUplink(ctl_data);
        queryFlag = db->queryIOStatus("B54E453C",frame);
        CHECK(queryFlag == false);
        db->insertData(pJsonStrConvertor,1);
        queryFlag = db->queryIOStatus("B54E453C",frame);
        CHECK(queryFlag == true);
    }

    SUBCASE("test update data") {
        db->updateCmdStatus(cmd_data,0);
        db->updateCmdStatus(cmd_data,1);
        db->updateCmdStatus(time_cmd_data,0);
    }
}