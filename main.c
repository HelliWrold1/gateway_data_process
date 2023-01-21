/*
 * Created by HelliWrold1 on 2023/1/21 17:08.
 */
#include <stdio.h>
#include "mqtt/mqtt_connector.h"

void eachConnectedCallback(void* context, char* cause);

int main() {
    printf("Hello, World!\n");
    MQTTConnector_t mqttConn;
    connectorInit(&mqttConn);
//    取消下面四句注释，可以更改客户端信息
//    mqttConn.connInfo.brokerUrl = "localhost:1883";
//    mqttConn.connInfo.clientId = "mqttConnector";
//    mqttConn.connInfo.userName = "test";
//    mqttConn.connInfo.userPwd = "testPwd";
    connectorStart(&mqttConn);
    printf("%d\n",mqttConn.client);
    connectorStart(&mqttConn);
    printf("%d\n",mqttConn.client);

    while(1)
    {
        connectorPublish("test","hello,hi,hey",1);
        sleep(10);
    }
    return 0;
}

void eachConnectedCallback(void* context, char* cause)
{
    connectorSubscribe("test",1);
    GW_LOG(LOG_DEBUG,"Connection successful.\n");
}