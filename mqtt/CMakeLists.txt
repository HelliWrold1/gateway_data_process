project(mqtt)

set(HEADER_FILES
        mqtt_connector.h
        )

set(SOURCE_FILES
        mqtt_connector.c
        )

add_library(mqtt STATIC ${SOURCE_FILES} ${HEADER_FILES})