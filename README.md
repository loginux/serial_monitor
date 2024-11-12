# 串口监控工具

这是一个用C++实现的Windows串口监控工具，具有极低的资源占用。程序会在系统托盘显示图标，并在串口设备插拔时发送Windows通知提醒。

## 功能特点

- 实时监控串口设备的插入和移除

- 使用Windows原生通知系统提醒设备变化

- 显示具体的COM口号

- 系统托盘图标

- 静默运行，不显示控制台窗口

- 极低的内存和CPU占用（<2MB内存）

- 使用Windows原生API，性能优异

## 编译要求

- Windows 10 或更高版本

- CMake 3.10 或更高版本

- MinGW-w64 或 Visual Studio 2019+

- Windows SDK 10.0 或更高版本

## 编译步骤

1. 克隆项目：
   
   ```bash
   git clone https://github.com/ckdfs/serial-monitor.git
   cd serial-monitor
   ```

2. 创建构建目录并编译：
   
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

## 使用方法

1. 直接运行编译生成的可执行文件：
   
   ```bash
   .\build\Release\serial_monitor.exe
   ```

2. 程序会在系统托盘显示图标，右键点击图标可以退出程序

3. 当有串口设备插入或移除时，会收到Windows通知提醒

## 注意事项

- 确保系统允许显示通知

- 如需开机自启动，可以将程序快捷方式放入启动文件夹

- 程序不需要管理员权限即可运行

- 支持所有标准串口设备（COM端口）

## 开发说明

- 使用C++17标准

- 使用Windows原生API (Win32)

## 许可证

MIT License


