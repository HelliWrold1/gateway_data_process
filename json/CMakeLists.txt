project(log)

set(HEADER_FILES
        json_str_convertor.h
        )

set(SOURCE_FILES
        json_str_convertor.c
        )

add_library(json STATIC ${SOURCE_FILES} ${HEADER_FILES})