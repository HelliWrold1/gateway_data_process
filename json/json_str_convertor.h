/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#ifndef GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
#define GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H

#define _XOPEN_SOURCE
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <time.h>
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "spdlog/spdlog.h"
#include "rule.h"

typedef enum
{
    SENSOR_LINE,
    CTL_LINE,
    DATA_LINE,
    REMOVE_LINE,
    KEY_LINE,
    KEY_RANK = 7,
}KEY_ARRAY;

typedef enum
{
    JSON_SUCCESS = 0,
    JSON_PARSE_FAILURE = 1,
    JSON_FILE_READ_FAILURE = 2,
}JSON_STATUS;

typedef enum
{
    TYPE_SENSOR_DATA,
    TYPE_CONTROL_DATA,
    TYPE_INTERVAL_TIME_DATA = 0x1E,
}DATA_TYPE;

class JsonStrConvertor{

public:
    JsonStrConvertor();
    JsonStrConvertor(const char* buffer);
    ~JsonStrConvertor();
    int parseNodeUplink();
    int parseNodeUplink(const char *buffer);
    int parseRuleFile(const char *filename, struct sParsedJsonRule *pJsonRule);
private:
    void fillParsedSensorData(char *key[KEY_LINE][KEY_RANK]);
    void fillParsedControlData(char *key[KEY_LINE][KEY_RANK]);
    void fillParsedCommonData();
    void fillParsedDBSensorData(char *(*key)[7]);
    void fillParsedDBControlData(char *(*key)[7]);
    void fillParsedDBIntervalTimeData();
public:
    cJSON * json = nullptr;
    struct
    {
        const char * app = nullptr;
        int battery;
        const char *data = nullptr;
        int datatype;
        const char *datetime = nullptr;
        const char *devaddr = nullptr;
        int fcnt;
        const char *mac = nullptr;

        double temp;
        double humi;
        double lux;
        double co;
        double co2;
        double h2s;
        double nh3;

        int io4;
        int io5;
        int io8;
        int io9;
        int io11;
        int io14;
        int io15;

        const char * intervaltime = nullptr;
        const char* localtime = nullptr;
    }parsedData;
    char * str = nullptr;
};

#endif //GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
