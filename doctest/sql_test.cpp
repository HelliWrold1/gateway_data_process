/*
 * Created by HelliWrold1 on 2023/1/30 21:24.
 */
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
//    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] [%!]: %v");
    logger->set_level(spdlog::level::debug);
    // get db
    DB* db = DB::getDB();
    int id;
    bool queryFlag;
    std::string devaddr("B54E453C");
    std::map<const std::string,std::vector<const char*> > frame;

    SUBCASE("test insert data")
    {
        id = db->insertData(R"({
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
    })",1);
        CHECK(id != 0);
        db->insertCmd(id,R"('{"devaddr":"B54E453C", "data":"FA5F",  "confirmed":true, "port":2, "time":"immediately" }')");
    }

    SUBCASE("test query data")
    {
        queryFlag = db->queryIOStatus("B54E453C",frame);
        CHECK(queryFlag == false);
        db->insertData(R"({
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
        })",1);
        queryFlag = db->queryIOStatus("B54E453C",frame);
        CHECK(queryFlag == true);
    }
}