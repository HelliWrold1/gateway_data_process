cmake_minimum_required(VERSION 3.16)
project(gateway_data_process)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.c
        mqtt/mqtt_connector.c
        log/log.c
        json/json_str_convertor.c
        sql/mariadb_connector.c
        )
add_executable(gateway_data_process  ${SOURCE_FILES})

include_directories(
        json
        log
        mqtt
        sql
)
add_subdirectory(sql)
add_subdirectory(log)
add_subdirectory(mqtt)
add_subdirectory(json)
target_link_libraries(gateway_data_process
        paho-mqtt3as
        cjson
        mysqlclient
        sql
        log
        mqtt
        json
        )
add_subdirectory(doctest)
