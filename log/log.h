/*
 * Created by HelliWrold1 on 2023/1/22 1:08.
 */

#ifndef GATEWAY_DATA_PROCESS_LOG_H
#define GATEWAY_DATA_PROCESS_LOG_H

#include <stdio.h>
#include <stdarg.h>

#define OPEN_LOG // comment this to close log
//#define WRITE_LOG_FILE
#ifdef WRITE_LOG_FILE
#define LOG_FILE_PATH "./mqtt_connector.log"
#endif

#define LOG_LEVEL LOG_DEBUG // level ref: enum:G_LOG_LEVEL

#define GW_LOG(level, fmt,...) gw_log(level, __DATE__, __TIME__, __FILE__, \
                                    __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
}G_LOG_LEVEL;

char* logLevelGet(const int level);
void gw_log(const int level, const char* date, const char* time, const char* file,
            const char* fun, const int line,const char* fmt,...);

#endif //GATEWAY_DATA_PROCESS_LOG_H
