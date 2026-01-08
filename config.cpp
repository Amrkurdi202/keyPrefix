#include "config.h"
#include "globals.h"
#include "string_utils.h"
#include <windows.h>
#include <shlobj.h>

std::wstring get_cfg_path()
{
    if (!g_cfg_path.empty()) return g_cfg_path;

    PWSTR base = nullptr;
    std::wstring dir;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &base)) && base)
    {
        dir = std::wstring(base) + L"\\keyPrefix";
        CoTaskMemFree(base);
    }
    else
    {
        wchar_t tmp[MAX_PATH];
        GetTempPathW(MAX_PATH, tmp);
        dir = std::wstring(tmp) + L"keyPrefix";
    }

    CreateDirectoryW(dir.c_str(), nullptr);
    g_cfg_path = dir + L"\\config.ini";
    return g_cfg_path;
}

std::wstring ini_read_str(const wchar_t* sec, const wchar_t* key, const wchar_t* defv)
{
    std::wstring path = get_cfg_path();
    wchar_t buf[8192];
    buf[0] = 0;
    GetPrivateProfileStringW(sec, key, defv, buf, (DWORD)(sizeof(buf) / sizeof(buf[0])), path.c_str());
    return std::wstring(buf);
}

int ini_read_int(const wchar_t* sec, const wchar_t* key, int defv)
{
    std::wstring path = get_cfg_path();
    return (int)GetPrivateProfileIntW(sec, key, (UINT)defv, path.c_str());
}

void ini_write_str(const wchar_t* sec, const wchar_t* key, const std::wstring& val)
{
    std::wstring path = get_cfg_path();
    WritePrivateProfileStringW(sec, key, val.c_str(), path.c_str());
}

void ini_write_int(const wchar_t* sec, const wchar_t* key, int val)
{
    wchar_t buf[64];
    wsprintfW(buf, L"%d", val);
    ini_write_str(sec, key, buf);
}

void config_load()
{
    g_saved_vid = ini_read_str(L"device", L"vid", L"");
    g_saved_pid = ini_read_str(L"device", L"pid", L"");

    std::wstring p = ini_read_str(L"text", L"prefix", L"|");
    std::wstring s = ini_read_str(L"text", L"suffix", L"");

    {
        std::lock_guard<std::mutex> lock(g_text_mutex);
        g_prefix_w = unescape_text(p);
        g_suffix_w = unescape_text(s);
    }

    int hook = ini_read_int(L"state", L"hook_enabled", 1);
    g_hook_enabled.store(hook != 0);

    int idle = ini_read_int(L"state", L"idle_timeout_ms", 60);
    if (idle < 10) idle = 10;
    if (idle > 2000) idle = 2000;
    g_idle_timeout_ms.store((DWORD)idle);

    int tick = ini_read_int(L"state", L"timer_tick_ms", 1);
    if (tick < 1) tick = 1;
    if (tick > 50) tick = 50;
    g_timer_tick_ms.store((DWORD)tick);
}

void config_save()
{
    std::wstring p;
    std::wstring s;
    {
        std::lock_guard<std::mutex> lock(g_text_mutex);
        p = escape_text(g_prefix_w);
        s = escape_text(g_suffix_w);
    }

    ini_write_str(L"text", L"prefix", p);
    ini_write_str(L"text", L"suffix", s);
    ini_write_str(L"device", L"vid", g_saved_vid);
    ini_write_str(L"device", L"pid", g_saved_pid);
    ini_write_int(L"state", L"hook_enabled", g_hook_enabled.load() ? 1 : 0);
    ini_write_int(L"state", L"idle_timeout_ms", (int)g_idle_timeout_ms.load());
    ini_write_int(L"state", L"timer_tick_ms", (int)g_timer_tick_ms.load());

    g_last_cfg_save_ms = GetTickCount64();
    g_cfg_dirty.store(false);
}

void mark_cfg_dirty()
{
    g_cfg_dirty.store(true);
}

void maybe_save_cfg()
{
    if (!g_cfg_dirty.load()) return;
    ULONGLONG now = GetTickCount64();
    if (g_last_cfg_save_ms == 0 || (now - g_last_cfg_save_ms) >= 500)
    {
        config_save();
    }
}
