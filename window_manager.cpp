#include "window_manager.h"
#include "globals.h"
#include "app_logic.h"
#include "directx_renderer.h"
#include "resource.h"
#include <shellapi.h>
#include <wchar.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void tray_add(HWND hwnd)
{
    NOTIFYICONDATAW nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = TRAY_UID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    // FIX: Load the custom icon using the correct ID and the module handle
    nid.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_ICON1));

    // Fallback just in case
    if (!nid.hIcon) nid.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);

    lstrcpynW(nid.szTip, L"keyPrefix", ARRAYSIZE(nid.szTip));
    if (Shell_NotifyIconW(NIM_ADD, &nid)) g_tray_added.store(true);
}

void tray_remove(HWND hwnd)
{
    NOTIFYICONDATAW nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = TRAY_UID;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    g_tray_added.store(false);
}

void tray_menu(HWND hwnd)
{
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    bool hook = g_hook_enabled.load();
    bool vis = g_window_visible.load();

    AppendMenuW(menu, MF_STRING, ID_TRAY_SHOW, vis ? L"Hide" : L"Show");
    AppendMenuW(menu, MF_STRING, ID_TRAY_TOGGLE_HOOK, hook ? L"Stop hook" : L"Start hook");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

void window_show(bool show)
{
    if (!g_hwnd) return;
    if (show)
    {
        ShowWindow(g_hwnd, SW_SHOW);
        SetForegroundWindow(g_hwnd);
        g_window_visible.store(true);
    }
    else
    {
        ShowWindow(g_hwnd, SW_HIDE);
        g_window_visible.store(false);
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    if (msg == WM_TRAYICON)
    {
        if (lParam == WM_RBUTTONUP)
        {
            tray_menu(hWnd);
            return 0;
        }
        if (lParam == WM_LBUTTONDBLCLK)
        {
            bool vis = g_window_visible.load();
            window_show(!vis);
            return 0;
        }
    }

    if (msg == WM_COMMAND)
    {
        UINT id = LOWORD(wParam);
        if (id == ID_TRAY_SHOW)
        {
            bool vis = g_window_visible.load();
            window_show(!vis);
            return 0;
        }
        if (id == ID_TRAY_TOGGLE_HOOK)
        {
            set_hook_enabled(!g_hook_enabled.load());
            return 0;
        }
        if (id == ID_TRAY_EXIT)
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    switch (msg)
    {
    case WM_CLOSE:
        window_show(false);
        return 0;
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceAndWindow(HINSTANCE hInstance)
{
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;

    // FIX: Load the correct icon ID into the window class
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));

    wc.lpszClassName = L"ScannerPrefixSuffixImGui";
    if (!RegisterClassExW(&wc)) return false;

    g_hwnd = CreateWindowW(wc.lpszClassName, L"keyPrefix", WS_OVERLAPPEDWINDOW, 100, 100, 400, 735, nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hwnd) return false;

    // FIX: Force the window to update its icon immediately (Title bar & Taskbar)
    HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));
    if (hIcon)
    {
        SendMessageW(g_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessageW(g_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    if (!CreateDeviceD3D(g_hwnd)) return false;

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    tray_add(g_hwnd);
    return true;
}

bool has_arg(LPWSTR cmd, const wchar_t* flag)
{
    if (!cmd || !flag) return false;
    return wcsstr(cmd, flag) != nullptr;
}
