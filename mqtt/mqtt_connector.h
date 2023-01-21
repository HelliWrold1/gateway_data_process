/*
 * Created by HelliWrold1 on 2023/1/21 17:10.
 */

#ifndef GATEWAY_DATA_PROCESS_MQTT_CONNECTOR_H
#define GATEWAY_DATA_PROCESS_MQTT_CONNECTOR_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "MQTTAsync.h"
#include "log.h"

typedef enum {
    MQTT_CONNECTOR_SUCCESS,
    MQTT_CONNECTOR_FAILURE,
    MQTT_CONNECTOR_CREATE_FAILURE,
    MQTT_CONNECTOR_CONNECT_FAILURE
}MQTT_CONNECTOR_STATUS;

typedef struct sMQTTConnector{
    // paho.mqtt.MQTTAsync handle
    MQTTAsync client;

    // 客户端信息
    struct sConnectorInfo {
        const char* brokerUrl;
        const char* clientId;
        const char* userName;
        const char* userPwd;
    }connInfo;
    // 仅第一次连接成功回调
    void (*firstSuccessConnectedCallback)(void* context, MQTTAsync_successData* response);
    // 每次连接成功回调
    void (*eachConnectedCallback)(void* context, char* cause);
    // 连接失败回调
    void (*connectFailureCallback)(void* context, MQTTAsync_failureData* response);
    // 连接断开回调
    void (*connLostCallback)(void* context, char* cause);
    // 订阅成功回调
    void (*successSubscribedCallback)(void* context, MQTTAsync_successData* response);
    // 订阅失败回调
    void (*subscribedFailureCallback)(void* context,  MQTTAsync_failureData* response);
    // 订阅消息收到回调
    int (*msgArrivedCallback)(void* context, char* topicName, int topicLen, MQTTAsync_message *message);
    // 发布的数据成功交付broker回调
    void (*pubDeliveredCallback)(void* context, MQTTAsync_token token);
}MQTTConnector_t;

/**
 * @brief 初始化MQTTConnector_t结构体，除connInfo之外的变量均被初始化为对应零值，
 *        connInfo被初始化为人为规定的默认值详见@ref connectorInit()
 * @param initMQTTConnector 指向被初始化的MQTTConnector_t
 * @return
 */
void connectorInit(MQTTConnector_t *initMQTTConnector);
/**
 * @brief 初始化连接器
 * @param initMQTTConnector 指向一个MQTT连接器结构体 @ref MQTTConnector_t
 * @return
 */
int connectorStart(MQTTConnector_t *initMQTTConnector);

/**
 * @brief 指定服务质量，订阅主题
 * @param topic 主题名
 * @param qos 服务质量
 * @return
 */
int connectorSubscribe(const char* topic, int qos);

/**
 * @brief 发布指定主题、服务质量、载荷的消息
 * @param topic
 * @param payload
 * @param qos
 * @return
 */
int connectorPublish(const char* topic, void* payload, int qos);

/**
 * @brief 连接成功回调，仅在第一次连接成功时回调
 *
 * @param context
 * @param response
 */
void defaultOnSuccessConnected(void* context, MQTTAsync_successData* response);

/**
 * @brief 连接成功回调
 *
 * @param context
 * @param cause
 */
void defaultOnConnectedCallBack(void* context, char* cause);

/**
 * @brief 连接失败回调
 *
 * @param context
 * @param response
 */
void defaultOnConnectFailure(void* context, MQTTAsync_failureData* response);

/**
 * @brief 连接断开回调
 *
 * @param context
 * @param cause
 */
void defaultConnLost(void* context, char* cause);

/**
 * @brief 订阅成功回调
 *
 * @param context
 * @param response
 */
void defaultOnSuccessSubscribe(void* context, MQTTAsync_successData* response);

/**
 * @brief 订阅失败回调
 * @param context
 * @param response
 */
void defaultOnFailureSubscribe(void* context, MQTTAsync_failureData* response);

/**
 * @brief 消息收到回调
 *
 * @param context
 * @param topicName
 * @param topicLen
 * @param message
 * @return int
 */
int defaultMsgArrived(void* context, char* topicName, int topicLen, MQTTAsync_message *message);

/**
 * @brief 消息交付回调
 *
 * @param context
 * @param token
 */
void defaultDelivered(void* context, MQTTAsync_token token);

#endif //GATEWAY_DATA_PROCESS_MQTT_CONNECTOR_H
