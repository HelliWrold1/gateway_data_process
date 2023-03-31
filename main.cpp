/*
 * Created by HelliWrold1 on 2023/1/21 17:08.
 */
#include <stdio.h>
#include "mqtt/mqtt_connector.h"
#include "json/json_str_convertor.h"
#include <string.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "DB.h"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

static const char* g_topic_rcvdata = "uplinkFromNode/#";  // lorawan-server uplink topic
static const char* g_topic_uplink = "uplinkToCloud";  // uplink to Cloud
static const char* g_topic_downlink = "downlinkToNode"; // downlink to Node
static const char* g_topic_from_cloud = "downlinkFromCloud";
static const char* g_topic_bridgeStatus = "$SYS/broker/connection/raspberrypi.raspberry/state"; // $SYS/broker/connection/#
static const int g_qos = 1;

static DB *db;

auto logger = spdlog::stdout_color_mt( "logger" );

void eachConnectedCallback(void* context, char* cause);
int msgArrivedCallback(void* context, char* topicName, int topicLen, MQTTAsync_message *message);

int main() {
    // set logger
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] [%!]: %v");
    logger->set_level(spdlog::level::debug);

    printf("Hello, World!\n");
    MQTTConnector_t mqttConn;
    initMQTTConnector(&mqttConn);
//    取消下面四句注释，可以更改客户端信息
//    mqttConn.connInfo.brokerUrl = "localhost:1883";
//    mqttConn.connInfo.clientId = "mqttConnector";
//    mqttConn.connInfo.userName = "test";
//    mqttConn.connInfo.userPwd = "testPwd";
    mqttConn.eachConnectedCallback = eachConnectedCallback;
    mqttConn.msgArrivedCallback = msgArrivedCallback;
    startMQTTConnector();

    db = DB::getDB();

    char jsonfile[] = "../rule/rules.json";
    std::vector<std::string> commands;
    Rules* rules = Rules::getRules(jsonfile);

    while(1){ }

    return 0;
}

// 为MQTTAsync提供每次连接成功后的回调函数
void eachConnectedCallback(void* context, char* cause)
{
    connectorSubscribe(g_topic_rcvdata,g_qos);
    connectorSubscribe(g_topic_bridgeStatus,g_qos);
    SPDLOG_LOGGER_DEBUG(logger, "Connection successful.");
}

int msgArrivedCallback(void* context, char* topicName, int topicLen, MQTTAsync_message *message) //接收数据回调
{
    char* payload = (char*)message->payload;
    SPDLOG_LOGGER_DEBUG(logger, "Message arrived:");
    SPDLOG_LOGGER_DEBUG(logger, "topic: '{}'\tpayload: '{}'\t payloadlength:{}\n\n", topicName, (char *) message->payload,
           message->payloadlen);
    if (strstr(topicName,"uplinkFromNode"))
    {
        // TODO 此处应为一个线程池添加任务
        JsonStrConvertor *pJsonStrConvertor = new JsonStrConvertor(payload);

        int auto_id;
        auto_id = db->insertData(pJsonStrConvertor,1);
        // TODO 根据配置文件判断规则
        char buf[80];
        SPDLOG_LOGGER_INFO(logger, "Current Path: {}", getcwd(buf, sizeof buf));

        std::vector<std::string> commands;
        Rules* rules = Rules::getRules();
        rules->setSourceData(pJsonStrConvertor);
        bool genFlag = rules->genCommands(pJsonStrConvertor,commands); // 将源数据读入到rule对象中，生成指令
        if (genFlag == true) {
            for (int i = 0; i < commands.size(); ++i) {
                connectorPublish(g_topic_downlink, commands[i].data(), g_qos); // 下发控制指令
                db->insertCmd(auto_id, commands[i].data());
            }
        } else {
            SPDLOG_LOGGER_DEBUG(logger, "gen {} 's command false",pJsonStrConvertor->parsedData.devaddr);
        }
        delete pJsonStrConvertor;
    }

    //// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//    // 将云端下发的时间间隔指令存入数据库
//    if (strstr(topicName, g_topic_from_cloud)) {
//        db->insertCmdFromCloud(payload);
//    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}
