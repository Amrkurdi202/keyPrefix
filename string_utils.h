#pragma once
#include <string>

std::wstring utf8_to_wide(const std::string& s);
std::string wide_to_utf8(const std::wstring& w);
bool extract_vid_pid(const std::wstring& hwid, std::wstring& vid, std::wstring& pid);
std::wstring escape_text(const std::wstring& in);
std::wstring unescape_text(const std::wstring& in);
