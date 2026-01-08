#include "globals.h"

// -----------------------------------------------------------------------------
// GLOBAL DIRECTX VARS
// -----------------------------------------------------------------------------
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// -----------------------------------------------------------------------------
// APPLICATION STATE
// -----------------------------------------------------------------------------
std::atomic<bool> g_app_running(true);
std::atomic<bool> g_shutting_down(false);

std::atomic<bool> g_hook_enabled(true);
std::atomic<int> g_selected_device(0);

std::mutex g_text_mutex;
std::wstring g_prefix_w;
std::wstring g_suffix_w;

std::atomic<bool> g_in_scan(false);
std::atomic<bool> g_injecting(false);
std::atomic<ULONGLONG> g_last_activity(0);
std::atomic<int> g_last_dev(0);

std::mutex g_send_mutex;

std::atomic<DWORD> g_idle_timeout_ms(60);
std::atomic<DWORD> g_timer_tick_ms(1);

const unsigned short ENTER_SCANCODE = 0x1C;
const unsigned short TAB_SCANCODE = 0x0F;

const UINT WM_TRAYICON = WM_APP + 10;
const UINT TRAY_UID = 1;

const UINT ID_TRAY_SHOW = 1001;
const UINT ID_TRAY_TOGGLE_HOOK = 1002;
const UINT ID_TRAY_EXIT = 1003;

HWND g_hwnd = nullptr;
std::atomic<bool> g_window_visible(true);
std::atomic<bool> g_tray_added(false);

std::mutex g_hook_ctx_mutex;
InterceptionContext g_hook_ctx_shared = 0;

std::wstring g_cfg_path;
std::wstring g_saved_vid;
std::wstring g_saved_pid;

std::atomic<bool> g_cfg_dirty(false);
ULONGLONG g_last_cfg_save_ms = 0;
