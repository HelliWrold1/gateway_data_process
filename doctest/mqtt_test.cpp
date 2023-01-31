/*
 * Created by HelliWrold1 on 2023/1/31 19:20.
 */
#include "doctest.h"
#include "mqtt_connector.h"

TEST_CASE("testMQTTConnector")
{
    MQTTConnector_t mqttConn;
    initMQTTConnector(&mqttConn);
    CHECK(MQTT_CONNECTOR_SUCCESS == startMQTTConnector());
    sleep(5);
    CHECK(MQTT_CONNECTOR_SUCCESS == connectorSubscribe("test",1));
    CHECK(MQTT_CONNECTOR_SUCCESS == connectorPublish("test","This is a mqtt test payload",1));
}

