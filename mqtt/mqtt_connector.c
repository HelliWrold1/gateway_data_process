/*
 * Created by HelliWrold1 on 2023/1/21 17:10.
 */
#include "mqtt_connector.h"

static MQTTConnector_t *p_MQTTConnector;

void connectorInit(MQTTConnector_t *initMQTTConnector)
{
    p_MQTTConnector->client = 0;
    p_MQTTConnector->connInfo.brokerUrl = "localhost:1883";
    p_MQTTConnector->connInfo.clientId = "mqttConnector";
    p_MQTTConnector->connInfo.userName = "HelliWrold1";
    p_MQTTConnector->connInfo.userPwd = "HelloWorld!";
    p_MQTTConnector->firstSuccessConnectedCallback = NULL;
    p_MQTTConnector->eachConnectedCallback = NULL;
    p_MQTTConnector->connectFailureCallback = NULL;
    p_MQTTConnector->connLostCallback = NULL;
    p_MQTTConnector->successSubscribedCallback = NULL;
    p_MQTTConnector->subscribedFailureCallback = NULL;
    p_MQTTConnector->msgArrivedCallback = NULL;
    p_MQTTConnector->pubDeliveredCallback = NULL;

}

int connectorStart(MQTTConnector_t *initMQTTConnector)
{
    int rc;
    p_MQTTConnector = initMQTTConnector;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.automaticReconnect = 1; // 开启自动重连
    conn_opts.minRetryInterval = 1; // 1s，最小的重连间隔，每次失败间隔时间都会加倍
    conn_opts.maxRetryInterval = 5; // 20s，最大重连间隔

    // 创建客户端
    rc = MQTTAsync_create(&p_MQTTConnector->client, p_MQTTConnector->connInfo.brokerUrl,
                p_MQTTConnector->connInfo.clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if(rc != MQTTASYNC_SUCCESS)
    {
        return MQTT_CONNECTOR_CREATE_FAILURE;
    }
    // 设置回调函数
    if(p_MQTTConnector->eachConnectedCallback)
        MQTTAsync_setConnected(p_MQTTConnector->client,NULL,p_MQTTConnector->eachConnectedCallback);
    else
        MQTTAsync_setConnected(p_MQTTConnector->client,NULL,defaultOnConnectedCallBack);
    if(p_MQTTConnector->connLostCallback)
        MQTTAsync_setConnectionLostCallback(p_MQTTConnector->client,NULL,p_MQTTConnector->connLostCallback);
    else
        MQTTAsync_setConnectionLostCallback(p_MQTTConnector->client,NULL,defaultConnLost);
    if(p_MQTTConnector->firstSuccessConnectedCallback)
        conn_opts.onSuccess = p_MQTTConnector->firstSuccessConnectedCallback;
    else
        conn_opts.onSuccess = defaultOnSuccessConnected;
    if(p_MQTTConnector->connectFailureCallback)
        conn_opts.onFailure = p_MQTTConnector->connectFailureCallback;
    else
        conn_opts.onFailure = defaultOnConnectFailure;
    if(p_MQTTConnector->pubDeliveredCallback)
        MQTTAsync_setDeliveryCompleteCallback(p_MQTTConnector->client,NULL,p_MQTTConnector->pubDeliveredCallback);
    else
        MQTTAsync_setDeliveryCompleteCallback(p_MQTTConnector->client,NULL,defaultDelivered);
    if(p_MQTTConnector->msgArrivedCallback)
        MQTTAsync_setMessageArrivedCallback(p_MQTTConnector->client,NULL,p_MQTTConnector->msgArrivedCallback);
    else
        MQTTAsync_setMessageArrivedCallback(p_MQTTConnector->client,NULL,defaultMsgArrived);
    if( (rc = MQTTAsync_connect(p_MQTTConnector->client, &conn_opts)) != MQTTASYNC_SUCCESS )
    {
        printf("First connection failed, error code:%d\n", rc);
        return MQTT_CONNECTOR_CONNECT_FAILURE;
    }
}

int connectorSubscribe(const char* topic, int qos)
{
    int rc;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.context = p_MQTTConnector->client;
    // 设置订阅成功回调
    if(p_MQTTConnector->successSubscribedCallback)
        opts.onSuccess = p_MQTTConnector->successSubscribedCallback;
    else
        opts.onSuccess = defaultOnSuccessSubscribe;
    //opts.onFailure = NULL; // 订阅失败回调
    if(p_MQTTConnector->subscribedFailureCallback)
        opts.onFailure = p_MQTTConnector->subscribedFailureCallback;
    else
        opts.onFailure = defaultOnFailureSubscribe;
    if ((rc = MQTTAsync_subscribe(p_MQTTConnector->client, topic, qos, &opts)) != MQTTASYNC_SUCCESS) //尝试订阅主题
    {
        GW_LOG(LOG_ERROR,"Failed to subscribe topic '%s' , error code:%d\n",topic, rc);
        return MQTT_CONNECTOR_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Succeed in subscribing topic '%s' , error code:%d\n",topic, rc);
        return MQTT_CONNECTOR_SUCCESS;
    }
}

int connectorPublish(const char* topic, void* payload, int qos)
{
    int rc;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.context = p_MQTTConnector->client;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = qos;
    GW_LOG(LOG_DEBUG,"Sending:'%s\n",(char*)pubmsg.payload);
    if ((rc = MQTTAsync_sendMessage(p_MQTTConnector->client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        GW_LOG(LOG_ERROR,"Failed to send message: '%s' , error code:%d\n", (char *)pubmsg.payload, rc);
        return MQTT_CONNECTOR_FAILURE;
    }
    else
    {
        GW_LOG(LOG_DEBUG,"Succeed in sending message: '%s' , error code: %d\n", (char *)pubmsg.payload, rc);
        return MQTT_CONNECTOR_SUCCESS;
    }
}

void defaultOnSuccessConnected(void* context, MQTTAsync_successData* response)
{
    GW_LOG(LOG_DEBUG,"successConnected Message with token value %d delivery confirmed\n", response->token);
}

void defaultOnConnectedCallBack(void* context, char* cause)
{
    GW_LOG(LOG_DEBUG,"Connection successful.\n");
}

void defaultOnConnectFailure(void* context, MQTTAsync_failureData* response)
{
    GW_LOG(LOG_DEBUG,"Connection fail, error code: %d, error message:%s\n", response ? response->code : 0, response ? response->message : 0);
}

void defaultConnLost(void* context, char* cause)
{
    GW_LOG(LOG_DEBUG,"Disconnected, cause: %s\n", cause);
}

void defaultOnSuccessSubscribe(void* context, MQTTAsync_successData* response)
{
    GW_LOG(LOG_DEBUG,"Subscribe successful. Message with token value %d delivery confirmed\n", response->token);
}

void defaultOnFailureSubscribe(void* context, MQTTAsync_failureData* response)
{
    GW_LOG(LOG_DEBUG,"Subscribe failed. Message with token value %d delivery confirmed\n", response->token);
}

int defaultMsgArrived(void* context, char* topicName, int topicLen, MQTTAsync_message *message) //接收数据回调
{
    GW_LOG(LOG_INFO,"Message arrived:\n");
    GW_LOG(LOG_INFO,"topic: %s\tpayload: '%s'\t payloadlength:%d\n\n", topicName, (char *) message->payload,
           message->payloadlen);
}

void defaultDelivered(void* context, MQTTAsync_token token)
{
    GW_LOG(LOG_DEBUG,"Message with token value %d delivery confirmed\n", token);
}

