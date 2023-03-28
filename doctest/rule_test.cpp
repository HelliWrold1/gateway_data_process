/*
 * Created by HelliWrold1 on 2023/3/22 13:42.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
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
    SUBCASE("get rules")
    {
        JsonStrConvertor_t jsonStrConvertor;
        std::vector<std::string> commands;
        memset(&jsonStrConvertor, 0, sizeof(jsonStrConvertor));
        parseNodeUplink(sensor_str, &jsonStrConvertor); // 解析源数据
        rules->setSourceData(&jsonStrConvertor);
        rules->genCommands(&jsonStrConvertor,commands); // 将源数据读入到rule对象中，生成指令
        for (int j = 0; j < commands.size(); ++j) {
            SPDLOG_LOGGER_DEBUG(logger, commands[j]);
        }
    }
}