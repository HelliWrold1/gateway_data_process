/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#include "json_str_convertor.h"

void GetcJsonParsedDataResult(char* buffer, cJSON* json, cJsonParsedDataResult* cpdr)
{
    cpdr->app = cJSON_GetObjectItem(json, "app")->valuestring;
    cpdr->battery = cJSON_GetObjectItem(json, "battery")->valueint;
    cpdr->codr = cJSON_GetObjectItem(json, "codr")->valuestring;
    cpdr->data = cJSON_GetObjectItem(json, "data")->valuestring;
    cpdr->data1 = cJSON_GetObjectItem(json, "data1")->valueint;
    cpdr->data2 = cJSON_GetObjectItem(json, "data2")->valueint;
    cpdr->data3 = cJSON_GetObjectItem(json, "data3")->valueint;
    cpdr->data4 = cJSON_GetObjectItem(json, "data4")->valueint;
    cpdr->data5 = cJSON_GetObjectItem(json, "data5")->valueint;
    cpdr->data6 = cJSON_GetObjectItem(json, "data6")->valueint;
    cpdr->data7 = cJSON_GetObjectItem(json, "data7")->valueint;
    cpdr->datetime = cJSON_GetObjectItem(json, "datetime")->valuestring;
    cpdr->datr = cJSON_GetObjectItem(json, "datr")->valuestring;
    cpdr->desc = cJSON_GetObjectItem(json, "desc")->valuestring;
    cpdr->devaddr = cJSON_GetObjectItem(json, "devaddr")->valuestring;
    cpdr->fcnt = cJSON_GetObjectItem(json, "fcnt")->valueint;
    cpdr->freq = cJSON_GetObjectItem(json, "freq")->valuedouble;
    cpdr->lsnr = cJSON_GetObjectItem(json, "lsnr")->valueint;
    cpdr->mac = cJSON_GetObjectItem(json, "mac")->valuestring;
    cpdr->port = cJSON_GetObjectItem(json, "port")->valueint;
    cpdr->rssi = cJSON_GetObjectItem(json, "rssi")->valueint;
}

void GetcJsonParsedCtlResult(char *buffer, cJSON *json, cJsonParsedCtlResult *cpcr)
{

}