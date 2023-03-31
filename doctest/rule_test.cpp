/*
 * Created by HelliWrold1 on 2023/3/22 13:42.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "doctest.h"
#include "rule.h"
#include "json_str_convertor.h"
#include <spdlog/sinks/stdout_color_sinks.h>

TEST_CASE("Class Rule")
{
    auto logger  = spdlog::get("logger");
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] [%!]: %v");
    logger->set_level(spdlog::level::debug);
    char jsonfile[] = "../../rule/rules.json";
    Rules* rules = Rules::getRules(jsonfile);
    char sensor_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":0,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"3EB4A376","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    char ctl_data[] = "{\"app\": \"Raspiber-handler\", \"battery\": 0, \"data\": \"01000000000000000100000000000000\", \"datatype\": 1, \"datetime\": \"2023-03-26T06:17:26Z\", \"devaddr\": \"B54E453C\", \"fcnt\": 0, \"mac\": \"B827EBFFFE2114B5\", \"io4\": 0, \"io5\": 0, \"io8\": 0, \"io9\": 1, \"io11\": 0, \"io14\": 0, \"io15\": 0, \"localtime\": \"2023-03-26 14:17:26\"}";
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

    SUBCASE("get rules")
    {
        JsonStrConvertor *pJsonStrConvertor = new JsonStrConvertor();
        std::vector<std::string> commands;
        pJsonStrConvertor->parseNodeUplink(sensor_str); // 解析源数据
        rules->setSourceData(pJsonStrConvertor);
        rules->genCommands(pJsonStrConvertor, commands); // 将源数据读入到rule对象中，生成指令
        for (int j = 0; j < commands.size(); ++j) {
            SPDLOG_LOGGER_DEBUG(logger, commands[j]);
        }

        pJsonStrConvertor->parseNodeUplink(ctl_data);
        rules->setSourceData(pJsonStrConvertor);
        rules->genCommands(pJsonStrConvertor, commands); // 将源数据读入到rule对象中，生成指令
        for (int j = 0; j < commands.size(); ++j) {
            SPDLOG_LOGGER_DEBUG(logger, commands[j]);
        }

        pJsonStrConvertor->parseNodeUplink(time_data);
        rules->setSourceData(pJsonStrConvertor);
        rules->genCommands(pJsonStrConvertor, commands); // 将源数据读入到rule对象中，生成指令
        for (int j = 0; j < commands.size(); ++j) {
            SPDLOG_LOGGER_DEBUG(logger, commands[j]);
        }
        delete pJsonStrConvertor;
    }
}