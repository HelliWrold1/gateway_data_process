/*
 * Created by HelliWrold1 on 2023/3/22 13:42.
 */

#include "doctest.h"
#include "rule.h"
#include "json_str_convertor.h"

TEST_CASE("Class Rule")
{
    char jsonfile[] = "../../rule/rules.json";
    Rules* rules = Rules::getRules(jsonfile);
    char sensor_str[] =
            R"({"app":"Raspiber-handler","battery":0,"codr":"4/5","data":"0012345678901234","data1":18,"data2":52,"data3":86,"data4":120,"data5":144,"data6":18,"data7":52,"datatype":0,"datetime":"2023-01-27T18:13:05Z","datr":"SF10BW125","desc":"ABP-Ra-08-Control","devaddr":"67678D5E","fcnt":1,"freq":474.7,"lsnr":-10.2,"mac":"B827EBFFFE2114B5","port":2,"rssi":-115})";
    JsonStrConvertor_t jsonStrConvertor;
    memset(&jsonStrConvertor, 0, sizeof(jsonStrConvertor));
    parseNodeUplink(sensor_str, &jsonStrConvertor); // 解析源数据
    rules->setSourceData(&jsonStrConvertor); // TODO 将setSource和genCommand做到一起
    std::vector<std::string> commands;
    SUBCASE("get rules")
    {
        rules->genCommands(&jsonStrConvertor,commands); // 将源数据读入到rule对象中，生成指令
        for (int j = 0; j < commands.size(); ++j) {
            std::cout<<commands[j]<<std::endl;
        }
    }
}