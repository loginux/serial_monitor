#include <windows.h>
#include <setupapi.h>
#include <dbt.h>
#include <iostream>
#include <set>
#include <string>
#include <shellapi.h>
#include <codecvt>
#include <locale>
#include "resource.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "shell32.lib")

class SerialMonitor {
private:
    std::set<std::string> currentPorts;
    NOTIFYICONDATAW nid;
    HWND hwnd;
    HMENU hMenu;
    bool should_exit;

    std::wstring utf8ToUtf16(const std::string& str) {
        try {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(str);
        } catch(...) {
            return L"";
        }
    }

    void ShowContextMenu(HWND hwnd) {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    }

    void initNotification() {

        hwnd = CreateWindowW(L"SerialMonitorWindow", L"SerialMonitorWindow", 
            0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
        
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

        hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, 1, L"退出");

        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
        wcscpy(nid.szTip, L"串口监控");
        
        Shell_NotifyIconW(NIM_ADD, &nid);
    }

public:
    SerialMonitor() : should_exit(false) {
        hMenu = NULL;
        initNotification();
    }

    ~SerialMonitor() {
        if (hMenu) DestroyMenu(hMenu);
        Shell_NotifyIconW(NIM_DELETE, &nid);
        if (hwnd) DestroyWindow(hwnd);
    }
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_USER + 1) {
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
                SerialMonitor* monitor = (SerialMonitor*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                if (monitor) {
                    monitor->ShowContextMenu(hwnd);
                }
                return 0;
            }
        }
        else if (uMsg == WM_COMMAND) {
            if (LOWORD(wParam) == 1) {  // 退出菜单项的ID
                PostQuitMessage(0);
                return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void showNotification(const std::string& title, const std::string& message) {
        std::wstring wTitle = utf8ToUtf16(title);
        std::wstring wMessage = utf8ToUtf16(message);
        
        wcscpy(nid.szInfoTitle, wTitle.c_str());
        wcscpy(nid.szInfo, wMessage.c_str());
        nid.dwInfoFlags = NIIF_INFO;
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    std::set<std::string> getSerialPorts() {
        std::set<std::string> ports;
        HDEVINFO hDevInfo = SetupDiGetClassDevs(
            &GUID_DEVINTERFACE_COMPORT, 0, 0, 
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        
        if (hDevInfo == INVALID_HANDLE_VALUE) return ports;

        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); i++) {
            char portName[256] = {0};
            HKEY hKey = SetupDiOpenDevRegKey(
                hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 
                0, DIREG_DEV, KEY_READ);
            
            if (hKey != INVALID_HANDLE_VALUE) {
                DWORD size = sizeof(portName);
                RegQueryValueExA(hKey, "PortName", NULL, NULL, 
                    (LPBYTE)portName, &size);
                RegCloseKey(hKey);
                if (strncmp(portName, "COM", 3) == 0) {
                    ports.insert(portName);
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return ports;
    }

    void checkPortChanges() {
        auto newPorts = getSerialPorts();
        
        if (currentPorts.empty()) {
            currentPorts = newPorts;
            return;
        }
        
        for (const auto& port : newPorts) {
            if (currentPorts.find(port) == currentPorts.end()) {
                std::string msg = "新串口 " + port + " 已插入";
                showNotification("串口已连接", msg);
            }
        }
        
        for (const auto& port : currentPorts) {
            if (newPorts.find(port) == newPorts.end()) {
                std::string msg = "串口 " + port + " 已移除";
                showNotification("串口已断开", msg);
            }
        }
        
        currentPorts = newPorts;
    }

    bool ProcessMessages() {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }
            if (msg.message == WM_COMMAND && msg.wParam == 1) {
                PostQuitMessage(0);
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow) {
    
    // 设置应用程序图标
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpszClassName = L"SerialMonitorWindow";
    wc.hInstance = hInstance;
    wc.lpfnWndProc = SerialMonitor::WindowProc;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    
    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"窗口类注册失败", L"错误", MB_ICONERROR);
        return 1;
    }
    
    SerialMonitor monitor;
    
    // 显示启动通知
    monitor.showNotification("串口监控", "串口监控已启动，正在监控串口变化...");
    
    while (true) {
        if (!monitor.ProcessMessages()) {
            break;
        }
        monitor.checkPortChanges();
        Sleep(1000);
    }

    return 0;
} 