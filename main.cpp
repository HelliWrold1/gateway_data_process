/*
 * Created by HelliWrold1 on 2023/1/21 17:08.
 */
#include <stdio.h>
#include "mqtt/mqtt_connector.h"
#include "json/json_str_convertor.h"
#include "sql/mariadb_connector.h"
#include <string.h>
#include "DB.h"

static const char* g_topic_rcvdata = "uplinkFromNode/#";  // lorawan-server uplink topic
static const char* g_topic_uplink = "uplinkToCloud";  // uplink to Cloud
static const char* g_topic_downlink = "downlinkToNode"; // downlink to Node
static const char* g_topic_bridgeStatus = "$SYS/broker/connection/raspberrypi.raspberry/state"; // $SYS/broker/connection/#
static const int g_qos = 1;

static DB *db;
static MariadbConnector_t mariadbConnRecv;
static MariadbConnector_t mariadbConnSend;

void eachConnectedCallback(void* context, char* cause);
int msgArrivedCallback(void* context, char* topicName, int topicLen, MQTTAsync_message *message);

int main() {
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
    initMariadbConnector(&mariadbConnRecv);
    initMariadbConnector(&mariadbConnSend);
    if (connectMariadb(&mariadbConnRecv) != MARIADB_CONNECTOR_SUCCESS)
    {
        GW_LOG(LOG_ERROR, "Mariadb connector Recv connection failed");
        return 1;
    }

    if (connectMariadb(&mariadbConnSend) != MARIADB_CONNECTOR_SUCCESS)
    {
        GW_LOG(LOG_ERROR, "Mariadb connector Send connection failed");
        return 1;
    }

    while(1){ }

    return 0;
}

// 为MQTTAsync提供每次连接成功后的回调函数
void eachConnectedCallback(void* context, char* cause)
{
    connectorSubscribe(g_topic_rcvdata,g_qos);
    connectorSubscribe(g_topic_bridgeStatus,g_qos);
    GW_LOG(LOG_DEBUG,"Connection successful.\n");
}

int msgArrivedCallback(void* context, char* topicName, int topicLen, MQTTAsync_message *message) //接收数据回调
{
    char* payload = (char*)message->payload;
    GW_LOG(LOG_DEBUG,"Message arrived:\n");
    GW_LOG(LOG_DEBUG,"topic: %s\tpayload: '%s'\t payloadlength:%d\n\n", topicName, (char *) message->payload,
           message->payloadlen);
    if (strstr(topicName,"uplinkFromNode/B827EBFFFE2114B5"))
    {
        // TODO 此处应为一个线程池添加任务
        JsonStrConvertor_t jsonStrConvertor;
        parseNodeUplink(payload, &jsonStrConvertor);

        int auto_id;
        auto_id = db->insertData(jsonStrConvertor.str,1);
        // TODO 根据配置文件判断规则
        char buf[80];
        printf("pwd: %s\n",getcwd(buf,sizeof buf));
        char jsonfile[] = "../rule/rules.json";
        std::vector<std::string> commands;
        Rules* rules = Rules::getRules(jsonfile);
        rules->setSourceData(&jsonStrConvertor);
        bool genFlag = rules->genCommands(&jsonStrConvertor,commands); // 将源数据读入到rule对象中，生成指令
        if (genFlag == false)
            GW_LOG(LOG_DEBUG,"gen %s 's command false",jsonStrConvertor.parsedData.devaddr);
        for (int j = 0; j < commands.size(); ++j) {
            std::cout<<commands[j]<<std::endl;
        }
//        db->insertCmd(auto_id,"");
//        mariadbConnRecv.table = jsonStrConvertor.parsedData.devaddr;
//        mariadbCreateTable(&mariadbConnRecv);
//        mariadbInsertRecord(&mariadbConnRecv,jsonStrConvertor.str,0);

        deleteParsedNodeUplink(&jsonStrConvertor);
    }
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}
