/*
 * Created by HelliWrold1 on 2023/3/20 21:20.
 */

#include "rule.h"

Rules* Rules::g_rules = nullptr;
int Rules::m_rules_num = 0;
int Rules::m_rules_index = 0;
std::map<const std::string ,Rule_t> Rules::m_rules;
ParsedJsonRule_t Rules::m_json_rules;

Rules::Rules() {
    m_rules_index = 0;
}

/**
 * 从解析后的条件结构体中取出源节点（传感器节点）、条件（范围要求）、目标（关联的控制节点）、命令（控制节点的设备动作）
 */
void Rules::setRule() {
    // 递归读取规则
    if (m_rules_index < m_rules_num){

        std::string source;
        // 获取一个rule的源节点
        if (!m_json_rules.rules[m_rules_index].source.empty()){
            source = m_json_rules.rules[m_rules_index].source;
        }

        Rule_t rule;
        // 获取一组条件
        if (!m_json_rules.rules[m_rules_index].conditions.empty()){
            int condition_num = m_json_rules.rules[m_rules_index].conditions.size();
            for (int i = 0; i < condition_num; ++i) {
                Conditions_t condition;
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("co2min"))
                    condition.co2min = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["co2min"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("co2max"))
                    condition.co2max = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["co2max"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("comin"))
                    condition.comin = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["comin"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("comax"))
                    condition.comax = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["comax"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("h2smin"))
                    condition.h2smin = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["h2smin"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("h2smax"))
                    condition.h2smax = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["h2smax"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("nh3min"))
                    condition.nh3min = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["nh3min"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("nh3max"))
                    condition.nh3max = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["nh3max"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("tempmin"))
                    condition.tempmin = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["tempmin"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("tempmax"))
                    condition.tempmax = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["tempmax"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("humimin"))
                    condition.humimin = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["humimin"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("humimax"))
                    condition.humimax = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["humimax"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("luxmin"))
                    condition.luxmin = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["luxmin"];
                if (m_json_rules.rules[m_rules_index].conditions[std::to_string(i)].count("luxmax"))
                    condition.luxmax = m_json_rules.rules[m_rules_index].conditions[std::to_string(i)]["luxmax"];
                rule.conditions.push_back(condition);
            }
        }

        // 获取一个目标组
        rule.targets = m_json_rules.rules[m_rules_index].targets[std::to_string(m_rules_index)];

        if (!m_json_rules.rules[m_rules_index].actions.empty()){
            std::string device_str;
            int act_num = m_json_rules.rules[m_rules_index].actions.size();
            for (int i = 0; i < act_num; ++i) {
                Actions_t actions;
                device_str = "light";
                judgeAction(actions.light,device_str,i);
                device_str = "fun";
                judgeAction(actions.fun,device_str,i);
                device_str = "curtain";
                judgeAction(actions.curtain,device_str,i);
                rule.actions.push_back(actions);
            }
        }

        m_rules.insert({source,rule});

        // 读入规则的游标移动
        m_rules_index += 1;
        setRule(); // 递归读取
    }
}

/**
 * 将动作读入到rule对象中
 * @param device 引用设备成员变量，分别为light、fun、curtain
 * @param device_str 引用设备字符串，分别为 "light"、"fun"、"curtain"
 */
void Rules::judgeAction(int &device,std::string &device_str, int &cmd_index)
{
    std::string action;
    if (m_json_rules.rules[m_rules_index].actions[std::to_string(cmd_index)].count(device_str)){
        action = m_json_rules.rules[m_rules_index].actions[std::to_string(cmd_index)][device_str];
    }
    if (action == "open")
        device = 1;
    else if (action == "close")
        device = 0;
    else
        device = -1; // no action
}

/**
 * 将cJSON解析的rules读入到rule对象中
 * @param rules
 * @return 返回单例对象
 */
Rules* Rules::getRules(char* jsonFilePath) {
    ParsedJsonRule_t parsedJsonRule;
    if (!parseRuleFile(jsonFilePath,&parsedJsonRule)){
        m_json_rules = parsedJsonRule;
    }
//    m_json_rules = rules;
    m_rules_num = m_json_rules.rules.size();
    setRule();
    return getRules();
}

Rules* Rules::getRules() {
    if (g_rules == nullptr)
        g_rules = new Rules();
    return g_rules;
}

/**
 * 判断条件是否符合，返回符合与否
 * @note 本方法的结果关系到action执行与否
 * @return true:符合 false:不符合
 */
bool Rules::judgeConditions(std::string &source, int &index) {
    bool action_flag;
    judgeGtRange(m_sensor_data.co2,m_rules[source].conditions[index].co2min,action_flag);
    judgeLtRange(m_sensor_data.co2,m_rules[source].conditions[index].co2max,action_flag);
    judgeGtRange(m_sensor_data.co,m_rules[source].conditions[index].comin,action_flag);
    judgeLtRange(m_sensor_data.co,m_rules[source].conditions[index].comax,action_flag);
    judgeGtRange(m_sensor_data.nh3,m_rules[source].conditions[index].nh3min,action_flag);
    judgeLtRange(m_sensor_data.nh3,m_rules[source].conditions[index].nh3max,action_flag);
    judgeGtRange(m_sensor_data.h2s,m_rules[source].conditions[index].h2smin,action_flag);
    judgeLtRange(m_sensor_data.h2s,m_rules[source].conditions[index].h2smax,action_flag);
    judgeGtRange(m_sensor_data.temp,m_rules[source].conditions[index].tempmin,action_flag);
    judgeLtRange(m_sensor_data.temp,m_rules[source].conditions[index].tempmax,action_flag);
    judgeGtRange(m_sensor_data.humi,m_rules[source].conditions[index].humimin,action_flag);
    judgeLtRange(m_sensor_data.humi,m_rules[source].conditions[index].humimax,action_flag);
    judgeGtRange(m_sensor_data.lux,m_rules[source].conditions[index].luxmin,action_flag);
    judgeLtRange(m_sensor_data.lux,m_rules[source].conditions[index].luxmax,action_flag);
    return action_flag;
}

/**
 * 判断原子条件是否有效
 * @param range_num
 * @return true 有效 false 无效
 */
bool Rules::isAtomicConditionValid(double &range_num) {
    if (range_num == -1 || range_num == 65535){
        return false;
    }
    return true;
}

/**
 * 判断某源数据是否大于最小值
 * @note 当没有范围要求时，本方法不会改变action_flag的值
 * @param source 源数据
 * @param gt_range 最小值
 * @param action_flag 引用，标志是否符合范围
 * @return 返回本次判断是否成功，false表示没有被判断的源数据的范围要求
 */
bool Rules::judgeGtRange(double &source, double &gt_range, bool &action_flag) {
    if (isAtomicConditionValid(gt_range)){
        if (source > gt_range)
            action_flag = true;
        else
            action_flag = false;
        return true;
    }
    return false;
}

/**
 * 判断某源数据是否小于最大值
 * @param source 源数据
 * @param lt_range 最大值
 * @param action_flag 引用，标志是否符合范围
 * @return 返回本次判断是否成功，false表示没有被判断的源数据的范围要求
 */
bool Rules::judgeLtRange(double &source, double &lt_range, bool &action_flag) {
    if (isAtomicConditionValid(lt_range)){
        if (source < lt_range)
            action_flag = true;
        else
            action_flag = false;
        return true;
    }
    return false;
}

/**
 * 将传感器数据读入到rule对象中
 * @param pJsonStrConvertor
 */
void Rules::setSourceData(struct sJsonStrConvertor *pJsonStrConvertor) {
    m_sensor_data.temp = pJsonStrConvertor->parsedData.temp;
    m_sensor_data.humi = pJsonStrConvertor->parsedData.humi;
    m_sensor_data.lux = pJsonStrConvertor->parsedData.lux;
    m_sensor_data.co = pJsonStrConvertor->parsedData.co;
    m_sensor_data.co2 = pJsonStrConvertor->parsedData.co2;
    m_sensor_data.h2s = pJsonStrConvertor->parsedData.h2s;
    m_sensor_data.nh3 = pJsonStrConvertor->parsedData.nh3;
}
bool Rules::genCommands(struct sJsonStrConvertor *pSourceJsonStrConvertor, std::vector<std::string> &commands) {

    bool genFlag;
    std::string source(pSourceJsonStrConvertor->parsedData.devaddr); // 拿到source节点ID
    std::string command;
    int cond_num;
    if (m_rules.count(source)){
        cond_num = m_rules[source].conditions.size();
        genFlag = true;
    }
    else{
        genFlag = false;
    }
    for (int i = 0; i < cond_num; ++i) {
        if (judgeConditions(source,i)){
            if (m_rules[source].actions[i].light == 1){
                command.assign("light open"); // TODO 生成真正的规则
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].light == 0){
                command.assign("light close");
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].light == -1)
                ;
            if (m_rules[source].actions[i].fun == 1){
                command.assign("fun open");
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].fun == 0){
                command.assign("fun open");
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].fun == -1)
                ;
            if (m_rules[source].actions[i].curtain == 1){
                command.assign("curtain open");
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].curtain == 0){
                command.assign("curtain open");
                commands.push_back(command);
            }
            if (m_rules[source].actions[i].curtain == -1)
                ;
        }
    }
    return genFlag;
}






