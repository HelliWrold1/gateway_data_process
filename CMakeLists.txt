cmake_minimum_required(VERSION 3.16)
project(gateway_data_process)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp
        mqtt/mqtt_connector.cpp
        json/json_str_convertor.cpp
        sql/mysql_pool.cpp
        sql/DB.cpp
        pthread_pool/PthreadPool.cpp
        rule/rule.cpp)
add_executable(gateway_data_process  ${SOURCE_FILES})

include_directories(
        json
        log
        mqtt
        sql
        pthread_pool
        rule
)
add_subdirectory(sql)
add_subdirectory(log)
add_subdirectory(mqtt)
add_subdirectory(json)
add_subdirectory(pthread_pool)
add_subdirectory(rule)
add_subdirectory(doctest)

target_link_libraries(gateway_data_process
        paho-mqtt3as
        cjson
        mysqlclient
        sql
        mqtt
        json
        pthread-pool
        pthread
        rule
)
