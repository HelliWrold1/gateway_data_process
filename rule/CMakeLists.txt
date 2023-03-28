project(rule)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILES
        rule.h
        )

set(SOURCE_FILES
        rule.cpp
        )

add_library(rule STATIC ${SOURCE_FILES} ${HEADER_FILES})