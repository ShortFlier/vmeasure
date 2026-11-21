cmake_minimum_required(VERSION 3.16)

project(imagelabelview)

add_library(imagelabelview STATIC
    imagelabelview.h  imagelabelview.cpp    imagelabelview.ui
    imagelabel.h    imagelabel.cpp

)


target_include_directories(imagelabelview
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(imagelabelview PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)