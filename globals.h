#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <d3d11.h>
#include <string>
#include <mutex>
#include <atomic>

#ifndef INTERCEPTION_STATIC
#define INTERCEPTION_STATIC
#endif

#ifndef INTERCEPTION_API
#define INTERCEPTION_API
#endif

#include "interception.h"

// -----------------------------------------------------------------------------
// GLOBAL DIRECTX VARS
// -----------------------------------------------------------------------------
extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

// -----------------------------------------------------------------------------
// APPLICATION STATE
// -----------------------------------------------------------------------------
extern std::atomic<bool> g_app_running;
extern std::atomic<bool> g_shutting_down;

extern std::atomic<bool> g_hook_enabled;
extern std::atomic<int> g_selected_device;

extern std::mutex g_text_mutex;
extern std::wstring g_prefix_w;
extern std::wstring g_suffix_w;

extern std::atomic<bool> g_in_scan;
extern std::atomic<bool> g_injecting;
extern std::atomic<ULONGLONG> g_last_activity;
extern std::atomic<int> g_last_dev;

extern std::mutex g_send_mutex;

extern std::atomic<DWORD> g_idle_timeout_ms;
extern std::atomic<DWORD> g_timer_tick_ms;

extern const unsigned short ENTER_SCANCODE;
extern const unsigned short TAB_SCANCODE;

extern const UINT WM_TRAYICON;
extern const UINT TRAY_UID;

extern const UINT ID_TRAY_SHOW;
extern const UINT ID_TRAY_TOGGLE_HOOK;
extern const UINT ID_TRAY_EXIT;

extern HWND g_hwnd;
extern std::atomic<bool> g_window_visible;
extern std::atomic<bool> g_tray_added;

extern std::mutex g_hook_ctx_mutex;
extern InterceptionContext g_hook_ctx_shared;

extern std::wstring g_cfg_path;
extern std::wstring g_saved_vid;
extern std::wstring g_saved_pid;

extern std::atomic<bool> g_cfg_dirty;
extern ULONGLONG g_last_cfg_save_ms;
