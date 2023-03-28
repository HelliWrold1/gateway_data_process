/*
 * Created by HelliWrold1 on 2023/1/27 10:35.
 */

#include "json_str_convertor.h"

static auto logger = spdlog::get("logger");

int parseNodeUplink(const char *buffer, JsonStrConvertor_t *pJsonConvertor) {
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
            {"nh3",  "h2s",  "co",   "co2",    "humi",   "temp",   "lux"},
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
    SPDLOG_LOGGER_DEBUG(logger, cJSON_GetObjectItem(json, "datetime")->valuestring);
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

    SPDLOG_LOGGER_DEBUG(logger, pJsonConvertor->str);

    return JSON_SUCCESS;
}

int parseRuleFile(const char *filename, struct sParsedJsonRule *pJsonRule)
{
    // 读取json文件
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return JSON_FILE_READ_FAILURE;
    size_t file_size;
    long pos;
    char *json_str;
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return JSON_FILE_READ_FAILURE;
    }
    file_size = pos;
    rewind(fp);
    json_str = (char *) malloc(sizeof(char) * (file_size + 1));
    if (!json_str) {
        fclose(fp);
        return JSON_FILE_READ_FAILURE;
    }
    if (fread(json_str, file_size, 1, fp) < 1) {
        if (ferror(fp)) {
            fclose(fp);
            free(json_str);
            return JSON_FILE_READ_FAILURE;
        }
    }
    fclose(fp);
    json_str[file_size] = '\0';

    SPDLOG_LOGGER_DEBUG(logger, json_str);

    // 解析文件内容
    cJSON *json;

    if ((json = cJSON_Parse(json_str)) == NULL) {
        return JSON_PARSE_FAILURE;
    }

    // 提取rules
    int rule_num = cJSON_GetArraySize(json);
    cJSON *rules[rule_num]; // 根据规则数量定义
    char num_str[10];
    for (int i = 0; i < rule_num; ++i) {
        memset(num_str, 0, 10);
        sprintf(num_str, "%d",i);
        rules[i] = cJSON_GetObjectItem(json, num_str);
    }

    // 提取rule
    for (int i = 0; i < rule_num; ++i) {
        RulesMap_t rule;

        // 提取source
        rule.source.assign(cJSON_GetObjectItem(rules[i], "source")->valuestring);
        // 提取conditions
        cJSON *conditions = cJSON_GetObjectItem(rules[i], "conditions");
        int condition_num = cJSON_GetArraySize(conditions);
        for (int j = 0; j < condition_num; ++j) { // 遍历某规则内的所有条件组
            // 将单个condition构造出来一个map，与序号构成一个嵌套map
            memset(num_str, 0, 10);
            sprintf(num_str, "%d", j);
            cJSON *condition = cJSON_GetObjectItem(conditions, num_str)->child;
            std::map<const std::string, double> factors;
            while (condition!= nullptr) {
                std::string factor_str(condition->string);
                factors.insert({factor_str, cJSON_GetNumberValue(condition)});
                condition = condition->next;
            }
            rule.conditions.insert({std::to_string(i), factors}); // 插入一个condition
        }

        // 提取targets
        cJSON *targets = cJSON_GetObjectItem(rules[i], "targets");
        int target_num = cJSON_GetArraySize(targets);
        for (int j = 0; j < target_num; ++j) {
            memset(num_str, 0, 10);
            sprintf(num_str, "%d", j);
            cJSON *target = cJSON_GetObjectItem(targets, num_str);
            std::vector<std::string> nodes;
            int node_num = cJSON_GetArraySize(target);
            for (int k = 0; k < node_num; ++k) {
                std::string node_str(cJSON_GetArrayItem(target, k)->valuestring);
                nodes.push_back(node_str);
            }
            rule.targets.insert({std::to_string(i), nodes});

        }

        // 提取action
        cJSON *actions = cJSON_GetObjectItem(rules[i], "actions");
        int act_num = cJSON_GetArraySize(actions);
        for (int j = 0; j < target_num; ++j) {
            memset(num_str, 0, 10);
            sprintf(num_str, "%d", j);
            cJSON *action = cJSON_GetObjectItem(actions, num_str)->child;
            std::map<const std::string, const std::string> acts;
            while (action!= nullptr) {
                std::string device_str(action->string);
                std::string action_str(cJSON_GetStringValue(action));
                acts.insert({device_str, action_str});
                action = action->next;
            }
            rule.actions.insert({std::to_string(j), acts});
        }
        pJsonRule->rules.push_back(rule);
    }

    return JSON_SUCCESS;
}

void deleteParsedNodeUplink(JsonStrConvertor_t *pJsonConvertor) {
    cJSON_Delete((cJSON *) pJsonConvertor->json);
    cJSON_free(pJsonConvertor->str);
    memset(pJsonConvertor, 0, sizeof(*pJsonConvertor));
}

void fillParsedSensorData(JsonStrConvertor_t *pJsonConvertor, cJSON *json, char *key[KEY_LINE][KEY_RANK]) {
    pJsonConvertor->parsedData.nh3 = cJSON_GetObjectItem(json, key[DATA_LINE][0])->valuedouble;
    pJsonConvertor->parsedData.h2s = cJSON_GetObjectItem(json, key[DATA_LINE][1])->valuedouble;
    pJsonConvertor->parsedData.co = cJSON_GetObjectItem(json, key[DATA_LINE][2])->valuedouble;
    pJsonConvertor->parsedData.co2 = cJSON_GetObjectItem(json, key[DATA_LINE][3])->valuedouble;
    pJsonConvertor->parsedData.humi = cJSON_GetObjectItem(json, key[DATA_LINE][4])->valuedouble / 10;
    pJsonConvertor->parsedData.temp = cJSON_GetObjectItem(json, key[DATA_LINE][5])->valuedouble / 10;
    pJsonConvertor->parsedData.lux = cJSON_GetObjectItem(json, key[DATA_LINE][6])->valuedouble / 100;
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