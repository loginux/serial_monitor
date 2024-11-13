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

- MinGW-w64

- Windows SDK 10.0 或更高版本

## 编译步骤

### 使用 MinGW-w64 编译（推荐）

1. 克隆项目：
   
   ```bash
   git clone https://github.com/ckdfs/serial-monitor.git
   cd serial-monitor
   ```

2. 创建构建目录并编译：
   
   ```bash
   # 生成构建文件
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

   # 编译
   cmake --build build --config Release

   # 打包
   cd build
   cpack -C Release
   ```

## 使用方法

### 方法一：直接运行
1. 运行编译生成的可执行文件：
   ```bash
   .\build\Release\serial_monitor.exe
   ```

### 方法二：使用打包版本
1. 在 `build` 目录下找到 `SerialMonitor-1.0.0-win64.zip`
2. 解压到任意目录
3. 运行 `serial_monitor.exe`

## 程序功能
1. 程序会在系统托盘显示图标，右键点击图标可以退出程序
2. 当有串口设备插入或移除时，会收到Windows通知提醒

## 注意事项

- 确保系统允许显示通知

- 如需开机自启动，可以将程序快捷方式放入启动文件夹：
  - 按 Win+R，输入 `shell:startup`，将快捷方式放入打开的文件夹

- 程序不需要管理员权限即可运行

- 支持所有标准串口设备（COM端口）

- 程序已经静态链接运行时库，可以直接在其他Windows系统上运行

## 分发说明

### 分发前的检查
1. 确保使用 Release 配置编译
2. 确保启用了静态链接（已在 CMake 中配置）
3. 测试程序在未安装开发环境的系统上的运行情况

### 分发方式
1. 使用 `build` 目录下的 ZIP 包
2. ZIP 包包含了所有必要的运行时文件
3. 用户只需解压后运行即可，无需安装额外的运行时

### 故障排除
如果程序无法运行：
1. 确保使用兼容的 Windows 版本（Windows 10 或更高）
2. 检查系统通知设置是否启用
3. 检查是否有防病毒软件拦截

## 开发说明

- 使用C++17标准

- 使用Windows原生API (Win32)

- 使用CMake构建系统

- 静态链接运行时库，确保可移植性

## 许可证

MIT License


