/*
 * Created by HelliWrold1 on 2023/1/31 21:53.
 */
#include "doctest.h"
#include "json_str_convertor.h"
#include "unistd.h"

TEST_CASE("parseNodeUplink")
{
    JsonStrConvertor_t jsonStrConvertor;
    memset(&jsonStrConvertor, 0, sizeof(jsonStrConvertor));
    char sensor_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":0,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    char ctl_str[] =
            R"({"app":"Raspier-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":1,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    char time_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":30,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";

    SUBCASE("sensor str")
    {
        CHECK(JSON_SUCCESS == parseNodeUplink(sensor_str, &jsonStrConvertor));
        CHECK(jsonStrConvertor.json != (cJSON*)NULL);
        CHECK(jsonStrConvertor.parsedData.lux != 0);
        CHECK(jsonStrConvertor.parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor.str != NULL);
    }
    SUBCASE("control str")
    {
        CHECK(JSON_SUCCESS == parseNodeUplink(ctl_str,&jsonStrConvertor));
        CHECK(jsonStrConvertor.json != (cJSON*)NULL);
        CHECK(jsonStrConvertor.parsedData.io15 != 0);
        CHECK(jsonStrConvertor.parsedData.devaddr != 0);
        CHECK(jsonStrConvertor.str != NULL);
    }

    SUBCASE("time str")
    {
        CHECK(JSON_SUCCESS == parseNodeUplink(time_str, &jsonStrConvertor));
        CHECK(jsonStrConvertor.json != (cJSON*)NULL);
        CHECK(jsonStrConvertor.parsedData.intervaltime != NULL);
        CHECK(jsonStrConvertor.parsedData.devaddr != NULL);
        CHECK(jsonStrConvertor.str != NULL);
    }

    SUBCASE("parseNodeUplinkFailure")
    {
        CHECK(JSON_PARSE_FAILURE == parseNodeUplink("{,}", &jsonStrConvertor));
    }
    deleteParsedNodeUplink(&jsonStrConvertor);
}

TEST_CASE("test json file parser")
{
    char buf[80];
    printf("%s\n",getcwd(buf,sizeof buf));
    char jsonfile[] = "../../rule/rules.json";
    ParsedJsonRule_t parsedJsonRule;
    SUBCASE("parse json file")
    {
        CHECK(JSON_SUCCESS == parseRuleFile(jsonfile,&parsedJsonRule));
        std::cout<<parsedJsonRule.rules[0].source<<std::endl;
        std::cout<<parsedJsonRule.rules[0].conditions["1"]["co2max"]<<std::endl;
    }
}