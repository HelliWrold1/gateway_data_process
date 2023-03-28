project(sql)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILES
        mysql_pool.h
        DB.h
        )

set(SOURCE_FILES
        mysql_pool.cpp
        DB.cpp
        )

add_library(sql STATIC ${SOURCE_FILES} ${HEADER_FILES})