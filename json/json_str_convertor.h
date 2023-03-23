/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#ifndef GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
#define GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H

#define _XOPEN_SOURCE
#include <time.h>
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "log.h"
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

typedef struct sJsonStrConvertor{
    const cJSON * json;
    struct
    {
        const char * app;
        int battery;
        const char *data;
        int datatype;
        const char *datetime;
        const char *devaddr;
        int fcnt;
        const char *mac;

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

        const char * intervaltime;

        const char* localtime;
    }parsedData;
    char * str;
}JsonStrConvertor_t;

int parseNodeUplink(char* buffer, JsonStrConvertor_t* pJsonConvertor);

void deleteParsedNodeUplink(JsonStrConvertor_t* pJsonConvertor);

void fillParsedSensorData(JsonStrConvertor_t *pJsonConvertor, cJSON *json,  char *key[KEY_LINE][KEY_RANK]);

void fillParsedControlData(JsonStrConvertor_t *pJsonConvertor, cJSON *json,  char *key[KEY_LINE][KEY_RANK]);

void fillParsedCommonData(JsonStrConvertor_t *pJsonConvertor, cJSON *json);

int parseRuleFile(const char *filename, struct sParsedJsonRule *pJsonRule);

#endif //GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
