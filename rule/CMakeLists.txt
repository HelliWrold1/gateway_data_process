project(rule)

set(HEADER_FILES
        rule.h
        )

set(SOURCE_FILES
        rule.cpp
        )

add_library(rule STATIC ${SOURCE_FILES} ${HEADER_FILES})