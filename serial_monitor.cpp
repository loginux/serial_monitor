// Windows API 头文件
#include <windows.h>  // Windows核心API
#include <setupapi.h> // 设备管理API
#include <dbt.h>      // 设备事件通知
#include <iostream>
#include <set>
#include <string>
#include <shellapi.h> // Shell API（系统托盘相关）
#include <codecvt>    // 字符编码转换
#include <locale>
#include "resource.h" // 资源文件定义

// 链接必要的库
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "shell32.lib")

// 串口监控类
// 负责监控串口设备的插拔，显示系统托盘图标和通知
class SerialMonitor
{
private:
    std::set<std::string> currentPorts; // 当前已连接的串口集合
    NOTIFYICONDATAW nid;                // 系统托盘图标数据
    HWND hwnd;                          // 隐藏窗口句柄
    HMENU hMenu;                        // 右键菜单句柄
    HMENU hSubMenu;                     // 串口查看菜单句柄
    bool should_exit;                   // 退出标志

    // 获取串口列表菜单句柄
    HMENU getSubMenuHandle()
    {
        return this->hSubMenu;
    }

    // UTF-8字符串转换为UTF-16
    // 用于将ASCII串口名转换为Windows API所需的宽字符
    std::wstring utf8ToUtf16(const std::string &str)
    {
        try
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(str);
        }
        catch (...)
        {
            return L"";
        }
    }

    std::string utf16ToUtf8(const std::wstring &utf16)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(utf16);
    }

    // 显示系统托盘右键菜单
    void ShowContextMenu(HWND hwnd)
    {
        POINT pt;
        GetCursorPos(&pt);         // 获取鼠标位置
        SetForegroundWindow(hwnd); // 设置窗口为前台
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON,
                       pt.x, pt.y, 0, hwnd, NULL); // 在鼠标位置显示菜单
    }

    // 初始化系统托盘图标和通知
    void initNotification()
    {
        // 创建隐藏窗口用于接收消息
        hwnd = CreateWindowW(L"SerialMonitorWindow", L"SerialMonitorWindow",
                             0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

        // 存储this指针以在静态WindowProc中访问实例
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

        // 创建右键菜单
        hMenu = CreatePopupMenu();
        // 创建串口列表菜单
        hSubMenu = CreatePopupMenu();
        // 将串口列表菜单添加到主菜单中
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"串口列表");

        AppendMenuW(hMenu, MF_STRING, 2, L"版本 V1.2.2");
        AppendMenuW(hMenu, MF_STRING, 1, L"退出");

        // 初始化系统托盘图标数据
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
        wcscpy(nid.szTip, L"串口监控");

        // 添加系统托盘图标
        Shell_NotifyIconW(NIM_ADD, &nid);
    }

public:
    // 构造函数：初始化系统托盘
    SerialMonitor() : should_exit(false)
    {
        hMenu = NULL;
        hSubMenu = NULL;
        // 初始化当前已连接的串口
        currentPorts = getSerialPorts();
        initNotification();
    }

    // 析构函数：清理资源
    ~SerialMonitor()
    {
        if (hMenu)
            DestroyMenu(hMenu);
        Shell_NotifyIconW(NIM_DELETE, &nid);
        if (hwnd)
            DestroyWindow(hwnd);
    }

    // 窗口消息处理函数
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                                       WPARAM wParam, LPARAM lParam)
    {
        // 处理系统托盘消息
        if (uMsg == WM_USER + 1)
        {
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
            {
                SerialMonitor *monitor =
                    (SerialMonitor *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                if (monitor)
                {
                    monitor->ShowContextMenu(hwnd);
                }
                return 0;
            }
        }
        // 处理菜单命令
        else if (uMsg == WM_COMMAND)
        {
            if (LOWORD(wParam) >= 1000 && LOWORD(wParam) < 1100)
            { // 处理串口菜单项
#if 0 
                SerialMonitor* monitor = 
                    (SerialMonitor*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                if (monitor) {
                    int index = LOWORD(wParam) - 1000;
                    std::string selectedPort = *std::next(monitor->currentPorts.begin(), index);
                    monitor->showNotification("选择串口", "选择了串口: " + selectedPort);
                }
#endif
                return 0;
            }
            else if (LOWORD(wParam) == 1)
            { // 退出菜单项
                PostQuitMessage(0);
                return 0;
            }
        }
        // 处理菜单进入消息，更新二级菜单
        else if (uMsg == WM_INITMENUPOPUP)
        {
            SerialMonitor *monitor = (SerialMonitor *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (monitor)
            {
                HMENU hSubMenu = monitor->getSubMenuHandle(); // 列表菜单的句柄
                if (hSubMenu)
                {
                    monitor->updateSerialPortMenu(hSubMenu);
                }
            }
        }
        else
        {
            ;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void updateSerialPortMenu(HMENU hSubMenu)
    {
        //清除全部串口列表
        while (DeleteMenu(hSubMenu, 0, MF_BYPOSITION))
        {
            ;
        }
        auto ports = getSerialPorts();
        if (ports.empty())
        {
            return;
        }
        int index = 1000;
        for (const auto &port : ports)
        {
            AppendMenuW(hSubMenu, MF_STRING, index++, utf8ToUtf16(port).c_str());
        }
    }

    // 显示Windows通知
    void showNotification(const std::string &title, const std::string &message)
    {
        std::wstring wTitle = utf8ToUtf16(title);
        std::wstring wMessage = utf8ToUtf16(message);

        wcscpy(nid.szInfoTitle, wTitle.c_str());
        wcscpy(nid.szInfo, wMessage.c_str());
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = 2000;
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    // 获取当前系统中的所有串口
    std::set<std::string> getSerialPorts()
    {
        std::set<std::string> ports;
        // 获取串口设备信息集合
        HDEVINFO hDevInfo = SetupDiGetClassDevs(
            &GUID_DEVINTERFACE_COMPORT, 0, 0,
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (hDevInfo == INVALID_HANDLE_VALUE)
            return ports;

        // 遍历所有串口设备
        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); i++)
        {
            char portName[256] = {0};
            // 打开设备注册表键值
            HKEY hKey = SetupDiOpenDevRegKey(
                hDevInfo, &devInfo, DICS_FLAG_GLOBAL,
                0, DIREG_DEV, KEY_READ);

            if (hKey != INVALID_HANDLE_VALUE)
            {
                // 读取串口名称
                DWORD size = sizeof(portName);
                RegQueryValueExA(hKey, "PortName", NULL, NULL,
                                 (LPBYTE)portName, &size);
                RegCloseKey(hKey);
                if (strncmp(portName, "COM", 3) == 0)
                {

                    std::string port = portName;
                    // 获取设备描述
                    wchar_t deviceDesc[128] = {0};
                    // SetupDiGetDeviceRegistryPropertyW 是获取ANSIC编码的字符串
                    // SetupDiGetDeviceRegistryPropertyW 是获取宽字符编码的字符串
                    SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfo, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDesc, sizeof(deviceDesc), NULL);
                    port.append(":").append(utf16ToUtf8(deviceDesc));
                    ports.insert(port);
#if 0
                    // 获取制造商信息
                    wchar_t  manufacturer[128] = {0};

                    SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfo, SPDRP_MFG, NULL, (PBYTE)manufacturer, sizeof(manufacturer), NULL);
                    std::cout << "Manufacturer: " << manufacturer << std::endl;
#endif
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return ports;
    }

    // 检查串口变化并发送通知
    int checkPortChanges()
    {
        int ret = 0;
        auto newPorts = getSerialPorts();

        // 检查新增的串口
        for (const auto &port : newPorts)
        {
            if (currentPorts.find(port) == currentPorts.end())
            {
                std::string msg = "新串口 " + port + " 已插入";
                showNotification("串口已连接", msg);
                ret++;
            }
        }

        // 检查移除的串口
        for (const auto &port : currentPorts)
        {
            if (newPorts.find(port) == newPorts.end())
            {
                std::string msg = "串口 " + port + " 已移除";
                showNotification("串口已断开", msg);
                ret++;
            }
        }

        currentPorts = newPorts;
        return ret;
    }

    // 处理Windows消息
    bool ProcessMessages()
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false;
            }
            if (msg.message == WM_COMMAND && msg.wParam == 1)
            {
                PostQuitMessage(0);
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }
};

// 程序入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{

    // 注册窗口类
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpszClassName = L"SerialMonitorWindow";
    wc.hInstance = hInstance;
    wc.lpfnWndProc = SerialMonitor::WindowProc;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    if (!RegisterClassExW(&wc))
    {
        MessageBoxW(NULL, L"窗口类注册失败", L"错误", MB_ICONERROR);
        return 1;
    }

    // 创建监控实例
    SerialMonitor monitor;

    // 显示启动通知
    monitor.showNotification("串口监控", "串口监控已启动，正在监控串口变化...");

    int tick = 100;
    // 主消息循环
    while (true)
    {
        if (!monitor.ProcessMessages())
        {
            break;
        }
        if (tick == 0)
        {
            if (0 == monitor.checkPortChanges())
            {
                // 如果没有监测到拔插事件，则休眠1000ms
                tick = 50;
            }
            else
            {
                // 相反的 如果监测到拔插事件 则下一次休眠事件只有500ms
                // 有利于提高快速拔插时的响应速度
                tick = 100;
            }
        }
        Sleep(10);
        tick--;
    }

    return 0;
}
