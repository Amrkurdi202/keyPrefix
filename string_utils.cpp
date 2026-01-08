#include "string_utils.h"
#include <windows.h>

std::wstring utf8_to_wide(const std::string& s)
{
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w;
    w.resize((size_t)len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], len);
    return w;
}

std::string wide_to_utf8(const std::wstring& w)
{
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string s;
    s.resize((size_t)len);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], len, nullptr, nullptr);
    return s;
}

bool extract_vid_pid(const std::wstring& hwid, std::wstring& vid, std::wstring& pid)
{
    vid.clear();
    pid.clear();
    size_t vpos = hwid.find(L"VID_");
    size_t ppos = hwid.find(L"PID_");
    if (vpos != std::wstring::npos && vpos + 8 <= hwid.size()) vid = hwid.substr(vpos, 8);
    if (ppos != std::wstring::npos && ppos + 8 <= hwid.size()) pid = hwid.substr(ppos, 8);
    return !vid.empty() && !pid.empty();
}

std::wstring escape_text(const std::wstring& in)
{
    std::wstring out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); i++)
    {
        wchar_t c = in[i];
        if (c == L'\\') { out += L"\\\\"; continue; }
        if (c == L'\n') { out += L"\\n"; continue; }
        if (c == L'\t') { out += L"\\t"; continue; }
        if (c == L'\r') { continue; }
        out.push_back(c);
    }
    return out;
}

std::wstring unescape_text(const std::wstring& in)
{
    std::wstring out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); i++)
    {
        wchar_t c = in[i];
        if (c != L'\\')
        {
            out.push_back(c);
            continue;
        }
        if (i + 1 >= in.size())
        {
            out.push_back(L'\\');
            continue;
        }
        wchar_t n = in[i + 1];
        if (n == L'\\') { out.push_back(L'\\'); i++; continue; }
        if (n == L'n') { out.push_back(L'\n'); i++; continue; }
        if (n == L't') { out.push_back(L'\t'); i++; continue; }
        out.push_back(L'\\');
    }
    return out;
}
