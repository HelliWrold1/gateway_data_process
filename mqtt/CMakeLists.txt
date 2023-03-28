project(mqtt)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILES
        mqtt_connector.h
        )

set(SOURCE_FILES
        mqtt_connector.cpp
        )

add_library(mqtt STATIC ${SOURCE_FILES} ${HEADER_FILES})