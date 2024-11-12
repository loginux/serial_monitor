cmake_minimum_required(VERSION 3.10)
project(SerialMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置资源编译器
if(MINGW)
    set(CMAKE_RC_COMPILER "C:/msys64/mingw64/bin/windres.exe")
endif()

# 添加资源文件
set(RC_FILE resources.rc)

# 添加可执行文件和资源文件，使用 WIN32 创建 Windows GUI 应用
add_executable(serial_monitor WIN32 serial_monitor.cpp ${RC_FILE})

# 添加包含目录
target_include_directories(serial_monitor PRIVATE ${CMAKE_SOURCE_DIR})

# 链接必要的Windows库
target_link_libraries(serial_monitor setupapi shell32)