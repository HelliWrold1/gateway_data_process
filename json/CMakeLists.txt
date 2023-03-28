project(log)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILES
        json_str_convertor.h
        )

set(SOURCE_FILES
        json_str_convertor.cpp
        )

add_library(json STATIC ${SOURCE_FILES} ${HEADER_FILES})