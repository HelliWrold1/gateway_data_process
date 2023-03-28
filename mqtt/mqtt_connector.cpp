/*
 * Created by HelliWrold1 on 2023/1/21 17:10.
 */
#include "mqtt_connector.h"

static auto logger = spdlog::get("logger");
static MQTTConnector_t *pMQTTConnector;

void initMQTTConnector(MQTTConnector_t *initMQTTConnector)
{
    pMQTTConnector = initMQTTConnector;
    memset(pMQTTConnector, 0, sizeof(MQTTConnector_t));
    pMQTTConnector->connInfo.brokerUrl = "localhost:1883";
    pMQTTConnector->connInfo.clientId = "mqttConnector";
    pMQTTConnector->connInfo.userName = "HelliWrold1";
    pMQTTConnector->connInfo.userPwd = "HelloWorld!";
}

int startMQTTConnector()
{
    // 只有一个连接器/客户端
    if (pMQTTConnector->client != 0)
        return MQTT_CONNECTOR_ONLY_ON_CONNECTOR;
    int rc;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.automaticReconnect = 1; // 开启自动重连
    conn_opts.minRetryInterval = 1; // 1s，最小的重连间隔，每次失败间隔时间都会加倍
    conn_opts.maxRetryInterval = 5; // 20s，最大重连间隔

    // 创建客户端
    rc = MQTTAsync_create(&pMQTTConnector->client, pMQTTConnector->connInfo.brokerUrl,
                pMQTTConnector->connInfo.clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if(rc != MQTTASYNC_SUCCESS)
    {
        return MQTT_CONNECTOR_CREATE_FAILURE;
    }
    // 设置回调函数
    if(pMQTTConnector->eachConnectedCallback)
        MQTTAsync_setConnected(pMQTTConnector->client,NULL,pMQTTConnector->eachConnectedCallback);
    else
        MQTTAsync_setConnected(pMQTTConnector->client,NULL,defaultOnConnectedCallBack);
    if(pMQTTConnector->connLostCallback)
        MQTTAsync_setConnectionLostCallback(pMQTTConnector->client,NULL,pMQTTConnector->connLostCallback);
    else
        MQTTAsync_setConnectionLostCallback(pMQTTConnector->client,NULL,defaultConnLost);
    if(pMQTTConnector->firstSuccessConnectedCallback)
        conn_opts.onSuccess = pMQTTConnector->firstSuccessConnectedCallback;
    else
        conn_opts.onSuccess = defaultOnSuccessConnected;
    if(pMQTTConnector->connectFailureCallback)
        conn_opts.onFailure = pMQTTConnector->connectFailureCallback;
    else
        conn_opts.onFailure = defaultOnConnectFailure;
    if(pMQTTConnector->pubDeliveredCallback)
        MQTTAsync_setDeliveryCompleteCallback(pMQTTConnector->client,NULL,pMQTTConnector->pubDeliveredCallback);
    else
        MQTTAsync_setDeliveryCompleteCallback(pMQTTConnector->client,NULL,defaultDelivered);
    if(pMQTTConnector->msgArrivedCallback)
        MQTTAsync_setMessageArrivedCallback(pMQTTConnector->client,NULL,pMQTTConnector->msgArrivedCallback);
    else
        MQTTAsync_setMessageArrivedCallback(pMQTTConnector->client,NULL,defaultMsgArrived);

    // 连接Broker
    if( (rc = MQTTAsync_connect(pMQTTConnector->client, &conn_opts)) != MQTTASYNC_SUCCESS )
    {
        printf("First connection failed, error code:%d\n", rc);
        return MQTT_CONNECTOR_CONNECT_FAILURE;
    }
    else
    {
        return MQTT_CONNECTOR_SUCCESS;
    }
}

int connectorSubscribe(const char* topic, int qos)
{
    int rc;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.context = pMQTTConnector->client;
    // 设置订阅成功回调
    if(pMQTTConnector->successSubscribedCallback)
        opts.onSuccess = pMQTTConnector->successSubscribedCallback;
    else
        opts.onSuccess = defaultOnSuccessSubscribe;
    //opts.onFailure = NULL; // 订阅失败回调
    if(pMQTTConnector->subscribedFailureCallback)
        opts.onFailure = pMQTTConnector->subscribedFailureCallback;
    else
        opts.onFailure = defaultOnFailureSubscribe;
    if ((rc = MQTTAsync_subscribe(pMQTTConnector->client, topic, qos, &opts)) != MQTTASYNC_SUCCESS) //尝试订阅主题
    {
        SPDLOG_LOGGER_ERROR(logger,"Failed to subscribe topic '{}' , error code:{}\n",topic, rc);
        return MQTT_CONNECTOR_SUBSCRIBE_FAILURE;
    }
    else
    {
        SPDLOG_LOGGER_DEBUG(logger,"Succeed in subscribing topic '{}' , error code:{}\n",topic, rc);
        return MQTT_CONNECTOR_SUCCESS;
    }
}

int connectorPublish(const char* topic, const char *payload, int qos)
{
    int rc;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.context = pMQTTConnector->client;
    pubmsg.payload = (void*)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = qos;
    SPDLOG_LOGGER_DEBUG(logger,"Sending:'%s'\n",(char*)pubmsg.payload);
    if ((rc = MQTTAsync_sendMessage(pMQTTConnector->client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        SPDLOG_LOGGER_ERROR(logger,"Failed to send message: '{}' , error code:{}\n", (char *)pubmsg.payload, rc);
        return MQTT_CONNECTOR_PUBLISH_FAILURE;
    }
    else
    {
        SPDLOG_LOGGER_DEBUG(logger,"Succeed in sending message: '{}' , error code: {}\n", (char *)pubmsg.payload, rc);
        return MQTT_CONNECTOR_SUCCESS;
    }
}

void defaultOnSuccessConnected(void* context, MQTTAsync_successData* response)
{
    SPDLOG_LOGGER_DEBUG(logger,"successConnected Message with token value {} delivery confirmed\n", response->token);
}

void defaultOnConnectedCallBack(void* context, char* cause)
{
    SPDLOG_LOGGER_DEBUG(logger,"Connection successful.");
}

void defaultOnConnectFailure(void* context, MQTTAsync_failureData* response)
{
    SPDLOG_LOGGER_DEBUG(logger,"Connection fail, error code: %d, error message:{}", response ? response->code : 0, response ? response->message : 0);
}

void defaultConnLost(void* context, char* cause)
{
    SPDLOG_LOGGER_DEBUG(logger,"Disconnected, cause: {}", cause);
}

void defaultOnSuccessSubscribe(void* context, MQTTAsync_successData* response)
{
    SPDLOG_LOGGER_DEBUG(logger,"Subscribe successful. Message with token value {} delivery confirmed", response->token);
}

void defaultOnFailureSubscribe(void* context, MQTTAsync_failureData* response)
{
    SPDLOG_LOGGER_DEBUG(logger,"Subscribe failed. Message with token value {} delivery confirmed", response->token);
}

int defaultMsgArrived(void* context, char* topicName, int topicLen, MQTTAsync_message *message) //接收数据回调
{
    SPDLOG_LOGGER_DEBUG(logger,"Message arrived:\n");
    SPDLOG_LOGGER_DEBUG(logger,"topic: {}\tpayload: '{}'\t payloadlength:{}\n\n", topicName, (char *) message->payload,
           message->payloadlen);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}

void defaultDelivered(void* context, MQTTAsync_token token)
{
    SPDLOG_LOGGER_DEBUG(logger,"Message with token value {} delivery confirmed\n", token);
}

