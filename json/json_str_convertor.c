/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#include "json_str_convertor.h"

int parseNodeUplink(char *buffer, JsonStrConvertor_t *pJsonConvertor) {
    cJSON *json;

    if ((json = cJSON_Parse(buffer)) == NULL) {
        return JSON_PARSE_FAILURE;
    }

    int datatype;
    int i, value;
    struct tm time; // 用于封装和解析时间相关字符串
    time_t timestamp = 8 * 60 * 60; // 8小时时间戳，以s为单位，用于转换UTC时间
    cJSON *addJsonFlag; // 标志添加键值对是否成功，影响到解析成功与否

    datatype = cJSON_GetObjectItem(json, "datatype")->valueint;

    char *key[KEY_LINE][KEY_RANK] = {
            {"temp",  "humi",  "lux",   "co",    "co2",   "h2s",   "nh3"},
            {"io4",   "io5",   "io8",   "io9",   "io11",  "io14",  "io15"},
            {"data1", "data2", "data3", "data4", "data5", "data6", "data7"},
            {"desc",  "codr",  "datr",  "freq",  "lsnr",  "port",  "rssi"},
    };

    if (datatype == TYPE_SENSOR_DATA) // Collection node.
    {
        for (i = 0; i < KEY_RANK; ++i) {
            value = cJSON_GetObjectItem(json, key[DATA_LINE][i])->valueint;
            addJsonFlag = cJSON_AddNumberToObject(json, key[SENSOR_LINE][i], value);
        }
        fillParsedSensorData(pJsonConvertor, json, key);
    }

    if (datatype == TYPE_CONTROL_DATA) {
        for (int i = 0; i < KEY_RANK; ++i) {
            value = cJSON_GetObjectItem(json, key[DATA_LINE][i])->valueint;
            addJsonFlag = cJSON_AddNumberToObject(json, key[CTL_LINE][i], value);
        }
        fillParsedControlData(pJsonConvertor, json, key);
    }

    if (datatype == TYPE_INTERVAL_TIME_DATA) // Interval time.
    {
        int hour, min, sec;
        char intervalTime[15];

        hour = cJSON_GetObjectItem(json, "data1")->valueint;
        min = cJSON_GetObjectItem(json, "data2")->valueint;
        sec = cJSON_GetObjectItem(json, "data3")->valueint;
        sprintf(intervalTime, "%02d:%02d:%02d", hour, min, sec);
        strptime(intervalTime, "%H:%M:%S", &time);
        sprintf(intervalTime, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
        addJsonFlag = cJSON_AddStringToObject(json, "intervaltime", intervalTime);
        pJsonConvertor->parsedData.intervaltime = cJSON_GetObjectItem(json, "intervaltime")->valuestring;
    }

    // 转换UTC时间，将localtime加入到json对象中
    char localTimeStr[20];
    GW_LOG(LOG_DEBUG, cJSON_GetObjectItem(json, "datetime")->valuestring);
    strptime(cJSON_GetObjectItem(json, "datetime")->valuestring, "%Y-%m-%dT%H:%M:%SZ", &time);
    timestamp += mktime(&time);
    if (!localtime_r(&timestamp, &time))
        perror("localtime convert failed");
    sprintf(localTimeStr, "%d-%02d-%02d %02d:%02d:%02d",
            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
    addJsonFlag = cJSON_AddStringToObject(json, "localtime", localTimeStr);
    fillParsedCommonData(pJsonConvertor, json);

    // 删除暂时无用的键值对
    for (i = 0; i < KEY_RANK; ++i) {
        cJSON_DeleteItemFromObject(json, key[DATA_LINE][i]);
        cJSON_DeleteItemFromObject(json, key[REMOVE_LINE][i]);
    }

    if (addJsonFlag == NULL) {
        cJSON_Delete(json);
        memset(pJsonConvertor, 0, sizeof(*pJsonConvertor));
        return JSON_PARSE_FAILURE;
    }
    pJsonConvertor->json = json;
    pJsonConvertor->str = cJSON_Print(json);

    GW_LOG(LOG_DEBUG, pJsonConvertor->str);

    return JSON_SUCCESS;
}

void deleteParsedNodeUplink(JsonStrConvertor_t *pJsonConvertor) {
    cJSON_Delete((cJSON *) pJsonConvertor->json);
    cJSON_free(pJsonConvertor->str);
    memset(pJsonConvertor, 0, sizeof(*pJsonConvertor));
}

void fillParsedSensorData(JsonStrConvertor_t *pJsonConvertor, cJSON *json, char *key[KEY_LINE][KEY_RANK]) {
    pJsonConvertor->parsedData.temp = cJSON_GetObjectItem(json, key[DATA_LINE][0])->valueint;
    pJsonConvertor->parsedData.humi = cJSON_GetObjectItem(json, key[DATA_LINE][1])->valueint;
    pJsonConvertor->parsedData.lux = cJSON_GetObjectItem(json, key[DATA_LINE][2])->valueint;
    pJsonConvertor->parsedData.co = cJSON_GetObjectItem(json, key[DATA_LINE][3])->valueint;
    pJsonConvertor->parsedData.co2 = cJSON_GetObjectItem(json, key[DATA_LINE][4])->valueint;
    pJsonConvertor->parsedData.h2s = cJSON_GetObjectItem(json, key[DATA_LINE][5])->valueint;
    pJsonConvertor->parsedData.nh3 = cJSON_GetObjectItem(json, key[DATA_LINE][6])->valueint;
}

void fillParsedControlData(JsonStrConvertor_t *pJsonConvertor, cJSON *json, char *key[KEY_LINE][KEY_RANK]) {
    pJsonConvertor->parsedData.io4 = cJSON_GetObjectItem(json, key[DATA_LINE][0])->valueint;
    pJsonConvertor->parsedData.io5 = cJSON_GetObjectItem(json, key[DATA_LINE][1])->valueint;
    pJsonConvertor->parsedData.io8 = cJSON_GetObjectItem(json, key[DATA_LINE][2])->valueint;
    pJsonConvertor->parsedData.io9 = cJSON_GetObjectItem(json, key[DATA_LINE][3])->valueint;
    pJsonConvertor->parsedData.io11 = cJSON_GetObjectItem(json, key[DATA_LINE][4])->valueint;
    pJsonConvertor->parsedData.io14 = cJSON_GetObjectItem(json, key[DATA_LINE][5])->valueint;
    pJsonConvertor->parsedData.io15 = cJSON_GetObjectItem(json, key[DATA_LINE][6])->valueint;
}

void fillParsedCommonData(JsonStrConvertor_t *pJsonConvertor, cJSON *json) {
    pJsonConvertor->parsedData.localtime = cJSON_GetObjectItem(json, "localtime")->valuestring;
    pJsonConvertor->parsedData.devaddr = cJSON_GetObjectItem(json, "devaddr")->valuestring;
    pJsonConvertor->parsedData.battery = cJSON_GetObjectItem(json, "battery")->valueint;
    pJsonConvertor->parsedData.app = cJSON_GetObjectItem(json, "app")->valuestring;
}