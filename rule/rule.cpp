/*
 * Created by HelliWrold1 on 2023/3/20 21:20.
 */

#include "rule.h"

static auto logger = spdlog::get("logger");

int Rules::m_rules_num = 0;
int Rules::m_rules_index = 0;
std::map<const std::string ,Rule_t> Rules::m_rules;
std::map<const std::string ,IOExceptStatus_t> Rules::m_excepts;
ParsedJsonRule_t Rules::m_json_rules;
DB* Rules::m_db = DB::getDB();
std::mutex Rules::m_rules_lock;

Rules::Rules() {
    m_rules_index = 0;
}

/**
 * 从解析后的条件结构体中取出源节点（传感器节点）、条件（范围要求）、目标（关联的控制节点）、命令（控制节点的设备动作）
 */
bool Rules::setRule() {
    // 递归读取规则
    if (m_rules_index < m_rules_num){

        std::string source;
        // 获取一个rule的源节点
        if (!m_json_rules.rules[m_rules_index].source.empty()){
            source = m_json_rules.rules[m_rules_index].source;
        }

        Rule_t rule;
        // 获取一组条件
        if (!m_json_rules.rules[m_rules_index].conditions.empty()) {
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
            } // end for (int i = 0; i < condition_num; ++i)
        } // end if (!m_json_rules.rules[m_rules_index].conditions.empty())

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
//// 添加规则的动作的示例
//                device_str = "curtain";
//                judgeAction(actions.curtain,device_str,i);
                rule.actions.push_back(actions);
            }
        }

        m_rules.insert({source,rule});

        // 读入规则的游标移动
        m_rules_index += 1;
        setRule(); // 递归读取
    }
    m_rules_lock.unlock(); // 规则锁解开，其他对象可以重新读取规则

    return true;
}

/**
 * 将动作读入到rule对象中
 * @param device 引用设备成员变量，分别为light、fun
 * @param device_str 引用设备字符串，分别为 "light"、"fun"
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
 * @return 返回对象的指针，必须使用指针变量接收地址，用完后delete
 */
Rules* Rules::getRules(char* jsonFilePath) {
    m_rules_lock.lock(); // 读取规则的时候就加锁
    JsonStrConvertor * pJsonStrConvertor = new JsonStrConvertor();
    ParsedJsonRule_t parsedJsonRule;
    if (!pJsonStrConvertor->parseRuleFile(jsonFilePath,&parsedJsonRule)){
        m_json_rules = parsedJsonRule;
    }
    m_rules_num = m_json_rules.rules.size();
    if (setRule() == true)
        return getRules();
    else
        return nullptr; // 代表设置规则失败
}

Rules* Rules::getRules() {
    return new Rules();
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
void Rules::setSourceData(JsonStrConvertor *pJsonStrConvertor) {
    m_datatype = pJsonStrConvertor->parsedData.datatype;
    if (m_datatype == 0x00){
        m_sensor_data.temp = pJsonStrConvertor->parsedData.temp;
        m_sensor_data.humi = pJsonStrConvertor->parsedData.humi;
        m_sensor_data.lux = pJsonStrConvertor->parsedData.lux;
        m_sensor_data.co = pJsonStrConvertor->parsedData.co;
        m_sensor_data.co2 = pJsonStrConvertor->parsedData.co2;
        m_sensor_data.h2s = pJsonStrConvertor->parsedData.h2s;
        m_sensor_data.nh3 = pJsonStrConvertor->parsedData.nh3;
        SPDLOG_LOGGER_INFO(logger, "temp:{} humi:{} lux:{} co:{} co2:{} h2s:{} nh3:{}",
                           m_sensor_data.temp, m_sensor_data.humi,m_sensor_data.lux, m_sensor_data.co,
                           m_sensor_data.co2, m_sensor_data.h2s, m_sensor_data.nh3);
    }

    if (m_datatype == 0x01){
        m_control_data.io4 = pJsonStrConvertor->parsedData.io4 == 0 ? false : true;
        m_control_data.io5 = pJsonStrConvertor->parsedData.io5 == 0 ? false : true;
        m_control_data.io8 = pJsonStrConvertor->parsedData.io8 == 0 ? false : true;
        m_control_data.io9 = pJsonStrConvertor->parsedData.io9 == 0 ? false : true;
        m_control_data.io11 = pJsonStrConvertor->parsedData.io11 == 0 ? false : true;
        m_control_data.io14 = pJsonStrConvertor->parsedData.io14 == 0 ? false : true;
        m_control_data.io15 = pJsonStrConvertor->parsedData.io15 == 0 ? false : true;
    }

    if (m_datatype == 0x1E){
        m_time_data.hour = pJsonStrConvertor->parsedData.hour;
        m_time_data.min = pJsonStrConvertor->parsedData.min;
        m_time_data.sec = pJsonStrConvertor->parsedData.sec;
    }
}
/**
 * 判断期望的IO状态，存到rule类中
 * @note 每次收到传感器数据的时候调用，符合condition则要对期望IO状态做出改变
 * @param source
 * @return 返回是否符合条件
 */
bool Rules::judgeIOExcepts(std::string &source) {
    bool judgeIOFlag;

    int cond_num;
    int light;
    int fun;
//// 添加规则的动作的示例
//    int curtain;
    if (m_rules.count(source)){
        cond_num = m_rules[source].conditions.size();
        judgeIOFlag = true;
    }
    else{
        return false;
    }

    IOExceptStatus_t io_except;
    int target_num = m_rules[source].targets.size();
    for (int i = 0; i < cond_num; ++i) {
        Actions_t action;
        if (judgeConditions(source,i)) {
            // 初始化本次action
            action.light = m_rules[source].actions[i].light;
            action.fun = m_rules[source].actions[i].fun;
//// 添加规则的动作的示例
//            action.curtain = m_rules[source].actions[i].curtain;
            // 条件组判断通过后，对期望值做出更改
            if (action.light == 1) {
                for (int j = 0; j < target_num; ++j) {
                    // 如果期望值里没有某ControlNode的期望值，那么新建
                    if ( m_excepts.count(m_rules[source].targets[j]) )
                        m_excepts.insert({m_rules[source].targets[j], io_except});
                    m_excepts[ m_rules[source].targets[j] ].io4 = true;
                }
            }
            if (action.light == 0) {
                for (int j = 0; j < target_num; ++j) {
                    // 如果期望值里没有某ControlNode的期望值，那么新建
                    if ( m_excepts.count(m_rules[source].targets[j]) )
                        m_excepts.insert({m_rules[source].targets[j], io_except});
                    m_excepts[ m_rules[source].targets[j] ].io4 = false;
                }
            }

            if (action.fun == 1) {
                for (int j = 0; j < target_num; ++j) {
                    // 如果期望值里没有某ControlNode的期望值，那么新建
                    if ( m_excepts.count(m_rules[source].targets[j]) )
                        m_excepts.insert({m_rules[source].targets[j], io_except});
                    m_excepts[ m_rules[source].targets[j] ].io5 = true;
                }
            }
            if (action.fun == 0) {
                for (int j = 0; j < target_num; ++j) {
                    // 如果期望值里没有某ControlNode的期望值，那么新建
                    if ( m_excepts.count(m_rules[source].targets[j]) )
                        m_excepts.insert({m_rules[source].targets[j], io_except});
                    m_excepts[ m_rules[source].targets[j] ].io5 = false;
                }
            }

//// 可以添加更多的io动作，下面是个示例
//            if (action.curtain == 1) {
//                for (int j = 0; j < target_num; ++j) {
//                    // 如果期望值里没有某ControlNode的期望值，那么新建
//                    if ( m_excepts.count(m_rules[source].targets[j]) )
//                        m_excepts.insert({m_rules[source].targets[j], io_except});
//                    m_excepts[ m_rules[source].targets[j] ].io9 = true;
//                }
//            }
//            if (action.curtain == 0) {
//                for (int j = 0; j < target_num; ++j) {
//                    // 如果期望值里没有某ControlNode的期望值，那么新建
//                    if ( m_excepts.count(m_rules[source].targets[j]) )
//                        m_excepts.insert({m_rules[source].targets[j], io_except});
//                    m_excepts[ m_rules[source].targets[j] ].io9 = false;
//                }
//            }

        } // end if (judgeConditions(source,i))
    } // end for (int i = 0; i < cond_num; ++i)
    return judgeIOFlag;
}

bool Rules::genCommands(JsonStrConvertor *pSourceJsonStrConvertor, std::vector<std::string> &commands) {
    bool genFlag;
    std::string source(pSourceJsonStrConvertor->parsedData.devaddr);
    std::stringstream command;

    if (m_datatype == 0x00) {
        std::string target;
        int cond_num;
        if (m_rules.count(source)){
            cond_num = m_rules[source].conditions.size();
            genFlag = true;
        }
        else{
            genFlag = false;
        }

        int target_num = m_rules[source].targets.size();

        for (int i = 0; i < cond_num; ++i) {
            // 判断是否符合条件
            if (judgeConditions(source,i)){
                this->judgeIOExcepts(source);
                std::unordered_map<std::string, std::vector<std::string>, sHash> frame;
                IOExceptStatus_t io_except;

                for (int j = 0; j < target_num; ++j) {

                    io_except = m_excepts[ m_rules[source].targets[j] ]; // 拿到期望io
                    target.assign(m_rules[source].targets[j]); // 将接收命令的nodeID取出

                    // DB查表，查找目前状态，根据nodeID查找，如果目前无状态，则直接生成命令
                    if (m_db->queryIOStatus(m_rules[source].targets[j], frame)) {
                        // 拿到目前IO状态，与期望IO对比
                        JsonStrConvertor *pJsonStrConvertor = new JsonStrConvertor();
                        std::string io_status_str = frame["frame"][0];
                        pJsonStrConvertor->parseNodeUplink(io_status_str.data());

                        if (pJsonStrConvertor->parsedData.io4 != io_except.io4) {
                            command.str(""); // reset string stream
                            if (io_except.io4 == true){
                                command << R"({ "devaddr":")" << target << R"(", "data":"FA4F", "confirmed":true, "port":2, "time":"immediately" })";
                            } else {
                                command << R"({ "devaddr":")" << target << R"(", "data":"FA40", "confirmed":true, "port":2, "time":"immediately" })";
                            }
                            commands.push_back(command.str());
                            SPDLOG_LOGGER_INFO(logger, command.str());
                        }

                        if (pJsonStrConvertor->parsedData.io5 != io_except.io5) {
                            command.str(""); // reset string stream
                            if (io_except.io5 == true){
                                command << R"({ "devaddr":")" << target << R"(", "data":"FA5F", "confirmed":true, "port":2, "time":"immediately" })";
                            } else {
                                command << R"({ "devaddr":")" << target << R"(", "data":"FA50", "confirmed":true, "port":2, "time":"immediately" })";
                            }
                            commands.push_back(command.str());
                            SPDLOG_LOGGER_INFO(logger, command.str());
                        }
//// 还可以添加其他IO，下面是个示例
//                        if (pJsonStrConvertor->parsedData.io9 != io_except.io9) {
//                            command.str(""); // reset string stream
//                            if (io_except.io9 == true){
//                                command << R"({ "devaddr":")" << target << R"(", "data":"FA9F", "confirmed":true, "port":2, "time":"immediately" })";
//                            } else {
//                                command << R"({ "devaddr":")" << target << R"(", "data":"FA90", "confirmed":true, "port":2, "time":"immediately" })";
//                            }
//                            commands.push_back(command.str());
//                            SPDLOG_LOGGER_INFO(logger, command.str());
//                        }
                        delete pJsonStrConvertor;
                    } else {
                        // 无状态，则直接通过期望IO生成指令
                        command.str(""); // reset string stream
                        if (io_except.io4 == true){
                            command << R"({ "devaddr":")" << target << R"(", "data":"FA4F", "confirmed":true, "port":2, "time":"immediately" })";
                        } else {
                            command << R"({ "devaddr":")" << target << R"(", "data":"FA40", "confirmed":true, "port":2, "time":"immediately" })";
                        }
                        commands.push_back(command.str());
                        SPDLOG_LOGGER_INFO(logger, command.str());

                        command.str(""); // reset string stream
                        if (io_except.io5 == true){
                            command << R"({ "devaddr":")" << target << R"(", "data":"FA5F", "confirmed":true, "port":2, "time":"immediately" })";
                        } else {
                            command << R"({ "devaddr":")" << target << R"(", "data":"FA50", "confirmed":true, "port":2, "time":"immediately" })";
                        }
                        commands.push_back(command.str());
                        SPDLOG_LOGGER_INFO(logger, command.str());
//// 还可以添加其他IO，下面是个示例
//                        command.str(""); // reset string stream
//                        if (io_except.io9 == true){
//                            command << R"({ "devaddr":")" << target << R"(", "data":"FA9F", "confirmed":true, "port":2, "time":"immediately" })";
//                        } else {
//                            command << R"({ "devaddr":")" << target << R"(", "data":"FA90", "confirmed":true, "port":2, "time":"immediately" })";
//                        }
//                        commands.push_back(command.str());
//                        SPDLOG_LOGGER_INFO(logger, command.str());
                    } // end if (m_db->queryIOStatus(m_rules[source].targets[j], frame))
                } // end for (int j = 0; j < target_num; ++j)
            } // end if (judgeConditions(source,i))
        } // end for (int i = 0; i < cond_num; ++i)

    } else if (m_datatype == 0x01) { // elseif if (m_datatype == 0x01)
        // 处理控制节点数据
        if (m_control_data.io4 == false) {
            command.str("");
            command << R"({ "devaddr":")" << source << R"(", "data":"FA4F", "confirmed":true, "port":2, "time":"immediately" })";
            m_db->updateCmdStatus(command.str().data(), 0);
        }
        if (m_control_data.io4 == true) {
            command.str("");
            command << R"({ "devaddr":")" << source << R"(", "data":"FA40", "confirmed":true, "port":2, "time":"immediately" })";
            m_db->updateCmdStatus(command.str().data(), 0);
        }
        if (m_control_data.io5 == false) {
            command.str("");
            command << R"({ "devaddr":")" << source << R"(", "data":"FA5F", "confirmed":true, "port":2, "time":"immediately" })";
            m_db->updateCmdStatus(command.str().data(), 0);
        }
        if (m_control_data.io5 == true) {
            command.str("");
            command << R"({ "devaddr":")" << source << R"(", "data":"FA50", "confirmed":true, "port":2, "time":"immediately" })";
            m_db->updateCmdStatus(command.str().data(), 0);
        }
//// 可以添加其他IO，下面是个示例
//        if (m_control_data.io9 == false) {
//            command.str("");
//            command << R"({ "devaddr":")" << source << R"(", "data":"FA9F", "confirmed":true, "port":2, "time":"immediately" })";
//            m_db->updateCmdStatus(command.str().data(), 0);
//        }
//        if (m_control_data.io9 == true) {
//            command.str("");
//            command << R"({ "devaddr":")" << source << R"(", "data":"FA90", "confirmed":true, "port":2, "time":"immediately" })";
//            m_db->updateCmdStatus(command.str().data(), 0);
//        }
    } else if (m_datatype == 0x1E) { // elseif if (m_datatype == 0x01)
//// 由于向ClassA和ClassC下发的指令的格式不能相同，暂时不做时间间隔重发的功能
//        char intervalTime[15];
//        std::string intervalTimeString;
//        sprintf(intervalTime, "%02X%02X%02X", m_time_data.hour, m_time_data.min, m_time_data.sec);
//        intervalTimeString.assign(intervalTime);
//        command.str("");
//        command << R"({ "devaddr":")" << source << R"(", "data":"1E)" << intervalTimeString << R"(", )" << R"("confirmed":true, "port":2, "time":"immediately" })";
//        m_db->updateCmdStatus(command.str().data(), 0);
//        m_db->updateCmdStatus(command.str().data(), 1);
    } // end if (m_datatype == 0x01)

    return genFlag;
}




