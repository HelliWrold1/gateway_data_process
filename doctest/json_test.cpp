/*
 * Created by HelliWrold1 on 2023/1/31 21:53.
 */
#include "doctest.h"
#include "json_str_convertor.h"
#include "unistd.h"

TEST_CASE("parseNodeUplink")
{
    char sensor_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":0,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    char ctl_str[] =
            R"({"app":"Raspier-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":1,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    char time_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":30,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    JsonStrConvertor *jsonStrConvertor = new JsonStrConvertor(sensor_str);
    SUBCASE("sensor str")
    {
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.lux != 0);
        CHECK(jsonStrConvertor->parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor->str != NULL);
    }
    SUBCASE("control str")
    {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseNodeUplink(ctl_str));
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.io15 != 0);
        CHECK(jsonStrConvertor->parsedData.devaddr != 0);
        CHECK(jsonStrConvertor->str != NULL);
    }

    SUBCASE("time str")
    {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseNodeUplink(time_str));
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.intervaltime != NULL);
        CHECK(jsonStrConvertor->parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor->str != NULL);
    }

    SUBCASE("parseNodeUplinkFailure")
    {
        CHECK(JSON_PARSE_FAILURE == jsonStrConvertor->parseNodeUplink("{,}"));
    }
    delete jsonStrConvertor;
}

TEST_CASE("test DB json data") {
    JsonStrConvertor *jsonStrConvertor = new JsonStrConvertor();
    char sensor_data[] = "{\"app\": \"Raspiber-handler\", \"battery\": 0, \"data\": \"0000210020001402EF014000C9001D9F\", \"datatype\": 0, \"datetime\": \"2023-03-26T06:15:29Z\", \"devaddr\": \"3EB4A376\", \"fcnt\": 108, \"mac\": \"B827EBFFFE2114B5\", \"nh3\": 33, \"h2s\": 32, \"co\": 20, \"co2\": 751, \"humi\": 320, \"temp\": 201, \"lux\": 7583, \"localtime\": \"2023-03-26 14:15:29\"}";
    char ctl_data[] = "{\"app\": \"Raspiber-handler\", \"battery\": 0, \"data\": \"01000000000000000100000000000000\", \"datatype\": 1, \"datetime\": \"2023-03-26T06:17:26Z\", \"devaddr\": \"B54E453C\", \"fcnt\": 0, \"mac\": \"B827EBFFFE2114B5\", \"io4\": 0, \"io5\": 0, \"io8\": 0, \"io9\": 1, \"io11\": 0, \"io14\": 0, \"io15\": 0, \"localtime\": \"2023-03-26 14:17:26\"}";
    char time_data[] = "{\"app\": \"Raspiber-handler\", \"battery\": 0, \"data\": \"0100000000001E000000000000000000\", \"datatype\": 30, \"datetime\": \"2023-03-26T06:17:26Z\", \"devaddr\": \"B54E453C\", \"fcnt\": 0, \"mac\": \"B827EBFFFE2114B5\", \"intervaltime\": \"00:00:30\", \"localtime\": \"2023-03-26 14:17:26\"}";
    SUBCASE("sensor data") {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseNodeUplink(sensor_data));
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.lux != 0);
        CHECK(jsonStrConvertor->parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor->str != NULL);
    }
    SUBCASE("control data") {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseNodeUplink(ctl_data));
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.io9 != 0);
        CHECK(jsonStrConvertor->parsedData.devaddr != 0);
        CHECK(jsonStrConvertor->str != NULL);
    }
    SUBCASE("time data")
    {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseNodeUplink(time_data));
        CHECK(jsonStrConvertor->json != (cJSON*)NULL);
        CHECK(jsonStrConvertor->parsedData.intervaltime != NULL);
        CHECK(jsonStrConvertor->parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor->str != NULL);
    }
}

TEST_CASE("test json file parser")
{
    JsonStrConvertor *jsonStrConvertor = new JsonStrConvertor();
    char buf[80];
    printf("%s\n",getcwd(buf,sizeof buf));
    char jsonfile[] = "../../rule/rules.json";
    ParsedJsonRule_t parsedJsonRule;
    SUBCASE("parse json file")
    {
        CHECK(JSON_SUCCESS == jsonStrConvertor->parseRuleFile(jsonfile,&parsedJsonRule));
        std::cout<<parsedJsonRule.rules[0].source<<std::endl;
        std::cout<<parsedJsonRule.rules[0].conditions["1"]["co2max"]<<std::endl;
    }
}