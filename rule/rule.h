/*
 * Created by HelliWrold1 on 2023/3/20 21:20.
 */

#ifndef GATEWAY_DATA_PROCESS_RULE_H
#define GATEWAY_DATA_PROCESS_RULE_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define RETRY_TIMES 20
#define RETRY_INSTANCE_METHOD(n, func, this_ptr, ...) \
do { \
    int retry_count = 0; \
    while (retry_count < (n)) { \
        if ((this_ptr->func(__VA_ARGS__))) { \
            break; \
        } \
        retry_count++; \
    } \
} while (0)

#include <spdlog/spdlog.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "json_str_convertor.h"
#include "DB.h"
#include <unistd.h>
#include<mutex>
#include<thread>

class JsonStrConvertor;
class DB;

typedef struct sRulesMap{
public:
    std::string source; // "007E1"
    std::map<const std::string, std::map<const std::string, double>>  conditions; // <"1":<"co2":222>>
    std::map<const std::string, std::vector<std::string>> targets; // <"1":<"007E1","12345">>
    std::map<const std::string,std::map<const std::string, const std::string>> actions; // <"1":<"light":"open">>
}RulesMap_t;

typedef struct sParsedJsonRule{
public:
    std::vector<RulesMap_t> rules;
}ParsedJsonRule_t;

typedef struct sSensorData{
public:
    double co2;
    double co;
    double nh3;
    double h2s;
    double temp;
    double humi;
    double lux;
}SensorData_t;

typedef struct sControlData{
public:
    bool io4;
    bool io5;
    bool io8;
    bool io9;
    bool io11;
    bool io14;
    bool io15;
}ControlData_t;

typedef struct sTimeData {
public:
    int hour;
    int min;
    int sec;
}TimeData_t;

typedef struct sConditions{
public:
    double co2min = -1;
    double co2max = 65535;
    double comin = -1;
    double comax = 65535;
    double h2smin = -1;
    double h2smax = 65535;
    double nh3min = -1;
    double nh3max = 65535;
    double tempmin = -1;
    double tempmax = 65535;
    double humimin = -1;
    double humimax = 65535;
    double luxmin = -1;
    double luxmax = 65535;
}Conditions_t;

typedef struct sActions{
    // 0: close 1: open -1: no action
    int light = -1;
    int fun = -1;
//// 添加规则的动作的示例
//    int curtain = -1;
}Actions_t;

typedef struct sIOExceptStatus{
public:
    bool io4 = false;
    bool io5 = false;
    bool io8 = false;
    bool io9 = false;
    bool io11 = false;
    bool io14 = false;
    bool io15 = false;
}IOExceptStatus_t;

typedef struct sRule{
    std::vector<Conditions_t> conditions;
    std::vector<std::string> targets;
    std::vector<Actions_t> actions;
}Rule_t;

class Rules {
public:
    static Rules* getRules();
    static Rules* getRules(char* jsonFilePath);
    void setSourceData(JsonStrConvertor *pJsonStrConvertor);
    bool genCommands(JsonStrConvertor *pSourceJsonStrConvertor, std::vector<std::string> &commands);
private:
    Rules();
    static bool setRule();
    static void judgeAction(int &device, std::string &action, int &cmd_index);
    bool judgeConditions(std::string &source, int &index);
    bool judgeIOExcepts(std::string &source);
    bool judgeGtRange(double &source, double &gt_range,bool &action_flag);
    bool judgeLtRange(double &source, double &lt_range, bool &action_flag);
    bool isAtomicConditionValid(double &range_num);
    static ParsedJsonRule_t m_json_rules;
    int m_datatype;
    SensorData_t m_sensor_data;
    ControlData_t m_control_data;
    TimeData_t m_time_data;
    static int m_rules_num;
    static int m_rules_index;
    static std::map<const std::string, Rule_t> m_rules;
    static std::map<const std::string, IOExceptStatus_t> m_excepts;
    static DB *m_db;
    static std::recursive_mutex m_rules_lock; // 规则锁,递归锁，加锁多少次就得解锁多少次，递归锁可以被调用的子函数继承
};

#endif //GATEWAY_DATA_PROCESS_RULE_H
