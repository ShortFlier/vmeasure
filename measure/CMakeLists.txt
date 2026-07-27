cmake_minimum_required(VERSION 3.8)

project(measure)

add_library(measure STATIC
    calipertool.cpp
    circlefind.cpp
    ellipsefind.cpp
)

target_include_directories(measure
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(measure
    PUBLIC
    log
    ${OpenCV_LIBS}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET measure PROPERTY CXX_STANDARD 20)
endif()

