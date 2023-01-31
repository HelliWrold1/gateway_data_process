project(sql)

set(HEADER_FILES
        mariadb_connector.h
        )

set(SOURCE_FILES
        mariadb_connector.c
        )

add_library(sql STATIC ${SOURCE_FILES} ${HEADER_FILES})