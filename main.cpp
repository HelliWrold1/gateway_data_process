/*
 * Created by HelliWrold1 on 2023/1/21 17:08.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define SCAN_DB_SECOND (10)
#include <stdio.h>
#include "mqtt/mqtt_connector.h"
#include "json/json_str_convertor.h"
#include <string.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "DB.h"
#include "PthreadPool.h"

typedef struct sPthreadPoolArgs {
    std::string topicName;
    std::string payload;
}PthreadPoolArgs_t;

static const char* g_topic_rcvdata = "uplinkFromNode/#";  // lorawan-server uplink topic
static const char* g_topic_uplink = "uplinkToCloud";  // uplink to Cloud
static const char* g_topic_downlink = "downlinkToNode"; // downlink to Node
static const char* g_topic_rules_from_cloud = "rulesFromCloud";
static const char* g_topic_bridgeStatus = "$SYS/broker/connection/raspberrypi.raspberry/state"; // $SYS/broker/connection/#
static const int g_qos = 1;
static char jsonfile[] = "../rule/rules.json";

static DB *db;
static PthreadPool pthreadPool;
static int bridgeStatus = 1;
static std::mutex bridge_mutes;
static int preBridgeStatus;
static bool connectToMosquitto;
// 输出到控制台
//auto logger = spdlog::stdout_color_mt( "logger" );
// 输出到滚动日志文件
auto logger = spdlog::rotating_logger_mt("logger",
                 "/home/pi/gateway-data-process/log/gateway.log",1024*1024 * 5,3);

void eachConnectedCallback(void *context, char *cause);
void eachConnectLostCallback(void *context, char *cause);
int msgArrivedCallback(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void processData(void *args); // 处理实时节点数据
void resendUnsentData(void *args); // 断连重发
void sendExecutedCmd(); // 向云端发送已经执行的命令
void resendUnexecutedCmd(void *args); // 向节点重发命令

int main() {
    // set logger
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] [%t] [%!]: %v");
    logger->set_level(spdlog::level::info);
//    logger->set_level(spdlog::level::off);
    char buf[80];
    SPDLOG_LOGGER_INFO(logger, "Current Path: {}", getcwd(buf, sizeof buf));

    MQTTConnector_t mqttConn;
    initMQTTConnector(&mqttConn);
//    取消下面四句注释，可以更改客户端信息
//    mqttConn.connInfo.brokerUrl = "localhost:1883";
//    mqttConn.connInfo.clientId = "mqttConnector";
//    mqttConn.connInfo.userName = "test";
//    mqttConn.connInfo.userPwd = "testPwd";
    mqttConn.eachConnectedCallback = eachConnectedCallback;
    mqttConn.msgArrivedCallback = msgArrivedCallback;
    mqttConn.connLostCallback = eachConnectLostCallback;
    startMQTTConnector();

    db = DB::getDB();

    // 初始化线程池
    pthreadPool.Init(8);

    Rules *rules = Rules::getRules(jsonfile);
    if (rules) {
        delete rules;
        SPDLOG_LOGGER_INFO(logger, "Set Rules successful!");
    }
    pthreadPool.AddTask(resendUnexecutedCmd, nullptr); // 连接成功后开始监视未执行的Command

    while(1){ }
}

// 为MQTTAsync提供每次连接成功后的回调函数
void eachConnectedCallback(void* context, char* cause)
{
    connectToMosquitto = true;
    connectorSubscribe(g_topic_rcvdata, g_qos);
    connectorSubscribe(g_topic_bridgeStatus, g_qos);
    connectorSubscribe(g_topic_rules_from_cloud, g_qos);
    SPDLOG_LOGGER_DEBUG(logger, "Connection successful.");
}

void eachConnectLostCallback(void *context, char* cause){
    connectToMosquitto = false;
    SPDLOG_LOGGER_DEBUG(logger, "Connection Lost");
}


/**
 * 接收数据回调
 * @param context
 * @param topicName
 * @param topicLen
 * @param message
 * @return
 */
int msgArrivedCallback(void* context, char* topicName, int topicLen, MQTTAsync_message *message) {
    PthreadPoolArgs_t *pthreadPoolArgs = new PthreadPoolArgs_t;
    pthreadPoolArgs->topicName.assign(topicName);
    pthreadPoolArgs->payload.assign((char*)message->payload);
    pthreadPool.AddTask(processData,(void*)pthreadPoolArgs);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

/**
 * 线程池任务，用于处理数据
 * @param args topicName、MQTTAsync_message
 */
void processData(void *args) {
    PthreadPoolArgs_t *poolArgs = (PthreadPoolArgs_t*)args;
    const char *topicName = poolArgs->topicName.data();
    const char *payload = poolArgs->payload.data();
    SPDLOG_LOGGER_DEBUG(logger, "Message arrived:");
    SPDLOG_LOGGER_DEBUG(logger, "topic: '{}'\tpayload: '{}'", poolArgs->topicName, poolArgs->payload);

    if (strstr(topicName,"uplinkFromNode")) {
        JsonStrConvertor *pJsonStrConvertor = new JsonStrConvertor(payload);

        int auto_id;
        // 如果是传感器数据或时间间隔数据，则上传到云端
        if (pJsonStrConvertor->parsedData.datatype == TYPE_SENSOR_DATA ||
            pJsonStrConvertor->parsedData.datatype == TYPE_INTERVAL_TIME_DATA ||
            pJsonStrConvertor->parsedData.datatype == TYPE_CONTROL_DATA) // 控制节点数据是否上传
            connectorPublish(g_topic_uplink, pJsonStrConvertor->str, g_qos);
        auto_id = db->insertData(pJsonStrConvertor,bridgeStatus);

        std::vector<std::string> commands;
        Rules* rules = Rules::getRules();
        rules->setSourceData(pJsonStrConvertor);
        bool genFlag = rules->genCommands(pJsonStrConvertor,commands); // 将源数据读入到rule对象中，生成指令
        if (genFlag == true) {
            for (int i = 0; i < commands.size(); ++i) {
                connectorPublish(g_topic_downlink, commands[i].data(), g_qos); // 下发控制指令
                db->insertCmd(auto_id, commands[i].data());
            }
            sleep(SCAN_DB_SECOND); // 间隔SCAN_DB_SECOND扫描一次数据库
            SPDLOG_LOGGER_DEBUG(logger,"down {} commands",commands.size());
        } else {
            SPDLOG_LOGGER_DEBUG(logger, "gen {} 's command false",pJsonStrConvertor->parsedData.devaddr);
        }

        delete pJsonStrConvertor;
        delete rules;
    }

    // 规则下发后，重新向Rules类中读取规则
    if (strstr(topicName, g_topic_rules_from_cloud)) {
        // 将文件写入到json文件中
        if (cJSON_Parse(payload) != nullptr) {
            FILE *fp = fopen(jsonfile, "w");
            if (fp)
                fprintf(fp, "%s",payload);
            fclose(fp);
        }
        Rules *rules = Rules::getRules(jsonfile);
        if (rules) {
            delete rules;
            SPDLOG_LOGGER_INFO(logger, "Reset Rules successful!");
        }
    }

    // 桥连接状态的改变
    if (strstr(topicName, g_topic_bridgeStatus)) {
        int nowStatus = atoi(payload);
        if (nowStatus == 0)
            preBridgeStatus = 1; // 曾经断连
        if (bridgeStatus != nowStatus) {
            bridge_mutes.lock();
            if (bridgeStatus != nowStatus)
                bridgeStatus = nowStatus;
            bridge_mutes.unlock();
        }
        // 如果曾经断连，并且现在已经重新连接
        if (preBridgeStatus && bridgeStatus) {
            pthreadPool.AddTask(resendUnsentData, nullptr);
        }
    }
    delete poolArgs;
//// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//    // 将云端下发的时间间隔指令存入数据库
//    if (strstr(topicName, g_topic_from_cloud)) {
//        db->insertCmdFromCloud(payload);
//    }
}

/**
 * 向云端发送断连期间未发送的数据
 * @param args
 */
void resendUnsentData(void *args) {
    if (preBridgeStatus == 1) // 曾经断连才重发
        if (bridgeStatus == 0) {
            return;
        } else {
            std::unordered_map<std::string, std::vector<std::string>, sHash> records;
            while (db->queryUnsentData(records)) { // 只要有记录就一直查询和发送
                int frame_num = records["frame"].size();
                for (int i = 0; i < frame_num; ++i) {
                    // 如果此时的桥仍连接，则将数据发送到云端
                    if (bridgeStatus == 1) {
                        if (connectorPublish(g_topic_uplink, records["frame"][i].data(), g_qos) == MQTT_CONNECTOR_SUCCESS) {
                            RETRY_INSTANCE_METHOD(RETRY_TIMES, updateDataSendStatus, db, atoi(records["id"][i].data()));
                        }
                    } else {
                        return; // 桥接断开就结束本线程
                    }
                } // end for (int i = 0; i < frame_num; ++i)
            } // end while (db->queryUnsentData(records))
            // 无记录则结束线程
            preBridgeStatus = 0; // 重置曾经断连的状态
            return;
        } // end if (bridgeStatus == 0)
}

/**
 * 向云端发送已经执行的命令
 */
void sendExecutedCmd() {
    if (bridgeStatus == 0) {
        return;
    } else {
        std::unordered_map<std::string, std::vector<std::string>, sHash> records;
        while (db->queryUnSentCmd(records)) { // 只要有记录就一直查询和发送
            int cmd_num = records["cmd"].size();
            for (int i = 0; i < cmd_num; ++i) {
                // 如果此时的桥仍连接，则将数据发送到云端
                if (bridgeStatus == 1) {
                    if (connectorPublish(g_topic_uplink, records["cmd"][i].data(), g_qos) == MQTT_CONNECTOR_SUCCESS) {
                        RETRY_INSTANCE_METHOD(RETRY_TIMES, updateCmdStatus, db, records["cmd"][i].data(), 1);
                    }
                } else {
                    return; // 桥接断开就结束本线程
                }
            } // end for (int i = 0; i < cmd_num; ++i)
        } // end while (db->queryUnSentCmd(records))
        return; // 无记录则结束线程
    } // end if (bridgeStatus == 0)
}

/**
 * 将未发送的指令下发
 * @param args
 */
void resendUnexecutedCmd(void *args) {
    std::unordered_map<std::string, std::vector<std::string>, sHash> records;
    while (true) {
        while (db->queryUnexecutedCmd(records)) { // 只要有记录就一直查询和发送
            int cmd_num = records["cmd"].size();
            for (int i = 0; i < cmd_num; ++i) {
                if(connectToMosquitto)
                    if (connectorPublish(g_topic_downlink, records["cmd"][i].data(), g_qos) == MQTT_CONNECTOR_SUCCESS) {
                    RETRY_INSTANCE_METHOD(RETRY_TIMES, updateCmdDatetime, db, atoi( records["id"][i].data()));
                    }
            }
        }
//        sendExecutedCmd();
        SPDLOG_LOGGER_INFO(logger, "Sleep for {} sec.", SCAN_DB_SECOND);
        sleep(SCAN_DB_SECOND); // 间隔SCAN_DB_SECOND扫描一次数据库
    }
}
