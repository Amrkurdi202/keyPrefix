#pragma once
#include <string>

std::wstring get_cfg_path();
std::wstring ini_read_str(const wchar_t* sec, const wchar_t* key, const wchar_t* defv);
int ini_read_int(const wchar_t* sec, const wchar_t* key, int defv);
void ini_write_str(const wchar_t* sec, const wchar_t* key, const std::wstring& val);
void ini_write_int(const wchar_t* sec, const wchar_t* key, int val);
void config_load();
void config_save();
void mark_cfg_dirty();
void maybe_save_cfg();
