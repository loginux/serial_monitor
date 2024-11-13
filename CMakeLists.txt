# 指定最低 CMake 版本要求
# 3.10 版本支持 C++17 和现代 CMake 特性
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和版本
# SerialMonitor: 项目名称
# VERSION 1.0.0: 项目版本号，用于生成包和安装信息
project(SerialMonitor VERSION 1.0.0)

# 设置 C++ 标准
# CMAKE_CXX_STANDARD: 使用 C++17
# CMAKE_CXX_STANDARD_REQUIRED: 强制要求编译器支持指定的 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinGW 静态链接设置
# -static-libgcc: 静态链接 GCC 运行时库
# -static-libstdc++: 静态链接 C++ 标准库
# -mwindows: 不显示控制台窗口
# -static: 静态链接所有依赖
# -Wl,-Bstatic: 优先使用静态库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mwindows")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-Bstatic")

# 自动查找 windres 资源编译器
# windres: MinGW 的资源编译器，用于编译 Windows 资源文件(.rc)
find_program(CMAKE_RC_COMPILER NAMES windres mingw32-windres i686-mingw32-windres)
if(NOT CMAKE_RC_COMPILER)
    message(FATAL_ERROR "Could not find windres. Please make sure MinGW-w64 is properly installed.")
endif()

# 设置资源文件
# RC_FILE: Windows 资源文件，包含图标和应用程序清单
set(RC_FILE resources.rc)

# 添加可执行文件
# WIN32: 创建 Windows GUI 应用程序（不显示控制台）
# serial_monitor.cpp: 主源文件
# ${RC_FILE}: 资源文件
add_executable(serial_monitor WIN32 serial_monitor.cpp ${RC_FILE})

# 添加头文件搜索路径
# PRIVATE: 仅在当前目标内使用
# ${CMAKE_SOURCE_DIR}: 项目根目录
target_include_directories(serial_monitor PRIVATE ${CMAKE_SOURCE_DIR})

# 链接 Windows 系统库和静态库
# -static: 静态链接
# -Wl,-Bstatic: 链接器参数，优先使用静态库
# -lwinpthread: Windows POSIX 线程库
# setupapi: 设备管理 API
# shell32: Shell32 API（系统托盘等）
# advapi32: 高级 Windows 32 API
# user32: 用户界面相关
# gdi32: 图形设备接口
# ole32: COM 支持
# uuid: GUID 支持
# comctl32: 通用控件
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
# RUNTIME DESTINATION .: 将可执行文件安装到当前目录
install(TARGETS serial_monitor
    RUNTIME DESTINATION .
)

# CPack 打包配置
# ZIP: 生成 ZIP 格式的安装包
# CPACK_PACKAGE_NAME: 包名
# CPACK_PACKAGE_VENDOR: 发布者
# CPACK_PACKAGE_DESCRIPTION_SUMMARY: 包描述
# CPACK_PACKAGE_VERSION: 包版本（使用项目版本）
# CPACK_PACKAGE_INSTALL_DIRECTORY: 安装目录
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "SerialMonitor")
set(CPACK_PACKAGE_VENDOR "YourCompany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Serial Monitor Tool")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SerialMonitor")

# 修改 ZIP 包结构
# OFF: 不创建顶级目录，直接包含文件
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

# 包含 CPack 模块，启用打包功能
include(CPack)