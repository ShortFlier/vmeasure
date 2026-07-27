cmake_minimum_required(VERSION 3.8)

project(log)

add_library(log STATIC
            log.h)

target_include_directories(log
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
