#pragma once
#include "imgui.h"
#include <string>

int TextResizeCallback(ImGuiInputTextCallbackData* data);
bool InputTextMultilineString(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags);
void ensure_capacity(std::string& s, size_t cap);
