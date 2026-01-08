#pragma once
#include <windows.h>

void tray_add(HWND hwnd);
void tray_remove(HWND hwnd);
void tray_menu(HWND hwnd);
void window_show(bool show);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CreateDeviceAndWindow(HINSTANCE hInstance);
bool has_arg(LPWSTR cmd, const wchar_t* flag);
