cmake_minimum_required(VERSION 3.10)
project(SerialMonitor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinGW 静态链接设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mwindows")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-Bstatic")

# 自动查找 windres
find_program(CMAKE_RC_COMPILER NAMES windres mingw32-windres i686-mingw32-windres)
if(NOT CMAKE_RC_COMPILER)
    message(FATAL_ERROR "Could not find windres. Please make sure MinGW-w64 is properly installed.")
endif()

# 添加资源文件
set(RC_FILE resources.rc)

# 添加可执行文件和资源文件
add_executable(serial_monitor WIN32 serial_monitor.cpp ${RC_FILE})

# 添加包含目录
target_include_directories(serial_monitor PRIVATE ${CMAKE_SOURCE_DIR})

# 静态链接 Windows 系统库
target_link_libraries(serial_monitor PRIVATE
    -static
    -Wl,-Bstatic
    -lwinpthread
    setupapi
    shell32
    advapi32
    user32
    gdi32
    ole32
    uuid
    comctl32
)

# 安装配置
install(TARGETS serial_monitor
    RUNTIME DESTINATION .
)

# CPack 配置
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "SerialMonitor")
set(CPACK_PACKAGE_VENDOR "YourCompany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Serial Monitor Tool")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SerialMonitor")

# 修改安装目录结构
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

include(CPack)