#pragma once
#include <windows.h>
#include <string>
#include "interception.h"

void send_scancode(InterceptionContext ctx, InterceptionDevice dev, unsigned short scancode, unsigned short state);
bool vk_is_extended(WORD vk);
void send_vk(InterceptionContext ctx, InterceptionDevice dev, WORD vk, bool is_down, HKL layout);
void send_enter(InterceptionContext ctx, InterceptionDevice dev);
void send_tab(InterceptionContext ctx, InterceptionDevice dev);
void send_char(InterceptionContext ctx, InterceptionDevice dev, wchar_t ch);
void send_text(InterceptionContext ctx, InterceptionDevice dev, const std::wstring& text);
