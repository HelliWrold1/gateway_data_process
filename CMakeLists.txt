cmake_minimum_required(VERSION 3.16)
project(gateway_data_process C)

set(CMAKE_C_STANDARD 99)
include_directories(log mqtt)
add_executable(gateway_data_process main.c mqtt/mqtt_connector.c mqtt/mqtt_connector.h log/log.c log/log.h json/json_str_convertor.c json/json_str_convertor.h sql/sqlite_connector.c sql/sqlite_connector.h sql/sqlite_connector.h)
target_link_libraries(gateway_data_process paho-mqtt3as cjson sqlite3)
