# CMakeList.txt: CaliperTool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


project ("vmeasure")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


# OpenCV 4.8.0
set(OpenCV_DIR "D:\\gw\\opencv\\opencv\\build-qt-contrib\\install")
find_package(OpenCV REQUIRED)

add_subdirectory(log)
add_subdirectory(measure)
add_subdirectory(imagelabelview)

add_executable(vmeasure "main.cpp")

target_link_libraries(vmeasure PRIVATE 
                            measure 
                            imagelabelview 
                            Qt${QT_VERSION_MAJOR}::Widgets
                            ${OpenCV_LIBS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vmeasure PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。

