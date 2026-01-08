#include "startup_manager.h"
#include <windows.h>

std::wstring exe_path()
{
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0) return L"";
    return std::wstring(buf);
}

bool startup_task_install()
{
    std::wstring path = exe_path();
    if (path.empty()) return false;

    // Quote the path and add the flag: "C:\Path\To\App.exe" /minimized
    std::wstring cmd = L"\"" + path + L"\" /minimized";

    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    if (status != ERROR_SUCCESS) return false;

    // Set the value. Value name is "keyPrefix"
    status = RegSetValueExW(hKey, L"keyPrefix", 0, REG_SZ,
        (const BYTE*)cmd.c_str(),
        (DWORD)((cmd.size() + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS);
}

bool startup_task_remove()
{
    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    if (status != ERROR_SUCCESS) return false;

    status = RegDeleteValueW(hKey, L"keyPrefix");

    RegCloseKey(hKey);
    // Return true if deleted OR if it didn't exist (ERROR_FILE_NOT_FOUND)
    return (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND);
}

bool startup_task_exists()
{
    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_QUERY_VALUE, &hKey);

    if (status != ERROR_SUCCESS) return false;

    // Check if "keyPrefix" exists
    status = RegQueryValueExW(hKey, L"keyPrefix", nullptr, nullptr, nullptr, nullptr);

    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS);
}
