cmake_minimum_required(VERSION 3.16)

project(imagelabelview)


add_library(imagelabelview STATIC
    imagelabelview.h  imagelabelview.cpp
    imagelabel.h    imagelabel.cpp
    draw.h
    draw.cpp
)


target_include_directories(imagelabelview
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(imagelabelview PRIVATE measure  Qt${QT_VERSION_MAJOR}::Widgets)