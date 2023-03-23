project(pthread-pool CXX)

set(HEADER_FILES
        PthreadPool.h
        )

set(SOURCE_FILES
        PthreadPool.cpp
        )

add_library(pthread-pool STATIC ${SOURCE_FILES} ${HEADER_FILES})