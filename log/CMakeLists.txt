project(log)

set(HEADER_FILES
        log.h
        )

set(SOURCE_FILES
        log.c
        )

add_library(log STATIC ${SOURCE_FILES} ${HEADER_FILES})