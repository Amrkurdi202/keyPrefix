#include "imgui_helpers.h"

int TextResizeCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        std::string* str = (std::string*)data->UserData;
        str->resize((size_t)data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    return 0;
}

bool InputTextMultilineString(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, TextResizeCallback, (void*)str);
}

void ensure_capacity(std::string& s, size_t cap)
{
    if (s.capacity() < cap) s.reserve(cap);
    if (s.empty()) s.push_back('\0'), s.pop_back();
}
