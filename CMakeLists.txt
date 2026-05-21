# CMakeList.txt: CaliperTool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#opencv库
include_directories("D:\\gw\\opencv\\opencv\\build\\include")
link_directories("D:\\gw\\opencv\\opencv\\build\\x64\\vc16\\lib")

project ("CaliperTool")

# 将源代码添加到此项目的可执行文件。
add_executable (CaliperTool "calipertool.cpp" "calipertool.h" "mainn.cpp")

target_link_libraries(CaliperTool PRIVATE	opencv_world480d.lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CaliperTool PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
