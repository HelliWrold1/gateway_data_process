project(sql)

set(HEADER_FILES
        mariadb_connector.h
        mysql_pool.h
        DB.h
        )

set(SOURCE_FILES
        mariadb_connector.c
        mysql_pool.cpp
        DB.cpp
        )

add_library(sql STATIC ${SOURCE_FILES} ${HEADER_FILES})