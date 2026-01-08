#include "interception_logic.h"
#include <windows.h>

void send_scancode(InterceptionContext ctx, InterceptionDevice dev, unsigned short scancode, unsigned short state)
{
    InterceptionStroke s;
    ZeroMemory(&s, sizeof(s));
    InterceptionKeyStroke* k = (InterceptionKeyStroke*)&s;
    k->code = scancode;
    k->state = state;
    interception_send(ctx, dev, &s, 1);
}

bool vk_is_extended(WORD vk)
{
    switch (vk)
    {
    case VK_RMENU: case VK_RCONTROL: case VK_INSERT: case VK_DELETE:
    case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:
    case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
    case VK_DIVIDE: case VK_NUMLOCK: case VK_SNAPSHOT: case VK_APPS:
        return true;
    default:
        return false;
    }
}

void send_vk(InterceptionContext ctx, InterceptionDevice dev, WORD vk, bool is_down, HKL layout)
{
    UINT sc = MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC, layout);
    if (sc == 0) return;

    unsigned short state = 0;
    if (vk_is_extended(vk)) state |= INTERCEPTION_KEY_E0;
    state |= is_down ? INTERCEPTION_KEY_DOWN : INTERCEPTION_KEY_UP;

    send_scancode(ctx, dev, (unsigned short)sc, state);
}

void send_enter(InterceptionContext ctx, InterceptionDevice dev)
{
    HKL layout = GetKeyboardLayout(0);
    send_vk(ctx, dev, VK_RETURN, true, layout);
    send_vk(ctx, dev, VK_RETURN, false, layout);
}

void send_tab(InterceptionContext ctx, InterceptionDevice dev)
{
    HKL layout = GetKeyboardLayout(0);
    send_vk(ctx, dev, VK_TAB, true, layout);
    send_vk(ctx, dev, VK_TAB, false, layout);
}

void send_char(InterceptionContext ctx, InterceptionDevice dev, wchar_t ch)
{
    if (ch == L'\n') { send_enter(ctx, dev); return; }
    if (ch == L'\r') { return; }
    if (ch == L'\t') { send_tab(ctx, dev); return; }

    HKL layout = GetKeyboardLayout(0);
    SHORT v = VkKeyScanExW(ch, layout);
    if (v == -1) return;

    WORD vk = (WORD)(v & 0xFF);
    BYTE mods = (BYTE)((v >> 8) & 0xFF);

    bool need_shift = (mods & 1) != 0;
    bool need_ctrl = (mods & 2) != 0;
    bool need_alt = (mods & 4) != 0;

    if (need_ctrl) send_vk(ctx, dev, VK_LCONTROL, true, layout);
    if (need_alt) send_vk(ctx, dev, VK_LMENU, true, layout);
    if (need_shift) send_vk(ctx, dev, VK_LSHIFT, true, layout);

    send_vk(ctx, dev, vk, true, layout);
    send_vk(ctx, dev, vk, false, layout);

    if (need_shift) send_vk(ctx, dev, VK_LSHIFT, false, layout);
    if (need_alt) send_vk(ctx, dev, VK_LMENU, false, layout);
    if (need_ctrl) send_vk(ctx, dev, VK_LCONTROL, false, layout);
}

void send_text(InterceptionContext ctx, InterceptionDevice dev, const std::wstring& text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        send_char(ctx, dev, text[i]);
    }
}
