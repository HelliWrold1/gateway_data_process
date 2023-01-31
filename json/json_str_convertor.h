/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#ifndef GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
#define GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "cjson/cJSON.h"

/**
 * @brief Parsed cJSON data result
 *
 */
typedef struct {
    char *app;
    int battery;
    char *codr;
    char *data;
    int data1;
    int data2;
    int data3;
    int data4;
    int data5;
    int data6;
    int data7;
    char *datetime;
    char *datr;
    char *desc;
    char *devaddr;
    int fcnt;
    double freq;
    int lsnr;
    char *mac;
    int port;
    int rssi;
} cJsonParsedDataResult;

/**
 * @brief Parsed cJSON control result
 *
 */
typedef struct {

} cJsonParsedCtlResult;

void GetcJsonParsedDataResult(char *buffer, cJSON *json, cJsonParsedDataResult *cpdr);
void GetcJsonParsedCtlResult(char *buffer, cJSON *json, cJsonParsedCtlResult *cpcr);

#if defined(__cplusplus)
}
#endif

#endif //GATEWAY_DATA_PROCESS_JSON_STR_CONVERTOR_H
