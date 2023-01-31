/*
 * Created by HelliWrold1 on 2023/1/22 1:08.
 */

#ifndef GATEWAY_DATA_PROCESS_LOG_H
#define GATEWAY_DATA_PROCESS_LOG_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

#define OPEN_LOG // comment this to close log
//#define WRITE_LOG_FILE
#ifdef WRITE_LOG_FILE
#define LOG_FILE_PATH "./mqtt_connector.log"
#endif

#define LOG_LEVEL LOG_DEBUG // level ref: enum:G_LOG_LEVEL

/**
 * @warning 使用这样的调用可能会出现问题：
 *          char sql[1024];
 *          sprintf(sql, "%s'%%Y-%%m-%%dT%%H:%%i:%%s'","insert ");
 *          GW_LOG(LOG_DEBUG, sql);
 *          输出的信息含有乱码和Resource temporarily unavailable报错
 * @note 如果输出带有%%的字符串，应该向第二个参数fmt传递"%s"，用于说明要输出的信息不是fmt，而是输出信息
 * @example GW_LOG(LOG_DEBUG, "%s", "%%Y-%%m-%%d %%H:%%i:%%s");
 */
#define GW_LOG(level, fmt, ...) gw_log(level, __DATE__, __TIME__, __FILE__, \
                                    __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} G_LOG_LEVEL;

char *logLevelGet(const int level);
void gw_log(const int level, const char *date, const char *time, const char *file,
            const char *fun, const int line, const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif //GATEWAY_DATA_PROCESS_LOG_H
