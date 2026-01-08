#include "app_logic.h"
#include "globals.h"
#include "config.h"
#include "interception_logic.h"
#include <windows.h>
#include <thread>

void set_hook_enabled(bool on)
{
    g_hook_enabled.store(on);
    if (!on)
    {
        g_in_scan.store(false);
        g_last_activity.store(0);
        g_last_dev.store(0);
    }
    mark_cfg_dirty();
}

void stop_hook_now_for_exit()
{
    g_shutting_down.store(true);
    set_hook_enabled(false);

    {
        std::lock_guard<std::mutex> lock(g_send_mutex);
        g_app_running.store(false);
        std::lock_guard<std::mutex> ctx_lock(g_hook_ctx_mutex);
        if (g_hook_ctx_shared)
        {
            interception_destroy_context(g_hook_ctx_shared);
            g_hook_ctx_shared = 0;
        }
    }
}

void inject_prefix(InterceptionContext ctx, InterceptionDevice dev)
{
    if (g_shutting_down.load()) return;

    std::wstring prefix_local;
    {
        std::lock_guard<std::mutex> lock(g_text_mutex);
        prefix_local = g_prefix_w;
    }
    if (prefix_local.empty()) return;

    std::lock_guard<std::mutex> send_lock(g_send_mutex);
    if (g_shutting_down.load()) return;
    if (g_injecting.load()) return;

    g_injecting.store(true);
    send_text(ctx, dev, prefix_local);
    g_injecting.store(false);
}

void inject_suffix(InterceptionContext ctx, InterceptionDevice dev)
{
    if (g_shutting_down.load()) return;

    std::wstring suffix_local;
    {
        std::lock_guard<std::mutex> lock(g_text_mutex);
        suffix_local = g_suffix_w;
    }
    if (suffix_local.empty()) return;

    std::lock_guard<std::mutex> send_lock(g_send_mutex);
    if (g_shutting_down.load()) return;
    if (g_injecting.load()) return;

    g_injecting.store(true);
    send_text(ctx, dev, suffix_local);
    g_injecting.store(false);
}

void hook_thread_main()
{
    InterceptionContext ctx = interception_create_context();
    if (!ctx) return;

    {
        std::lock_guard<std::mutex> lock(g_hook_ctx_mutex);
        g_hook_ctx_shared = ctx;
    }

    interception_set_filter(ctx, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    std::thread timer_thread([ctx]()
    {
        while (g_app_running.load())
        {
            if (!g_shutting_down.load() && g_hook_enabled.load() && g_in_scan.load() && !g_injecting.load())
            {
                ULONGLONG last = g_last_activity.load();
                if (last != 0)
                {
                    ULONGLONG now = GetTickCount64();
                    DWORD timeout = g_idle_timeout_ms.load();
                    if ((now - last) > (ULONGLONG)timeout)
                    {
                        int dev_i = g_last_dev.load();
                        int target = g_selected_device.load();
                        if (dev_i > 0 && dev_i == target && g_in_scan.load())
                        {
                            inject_suffix(ctx, (InterceptionDevice)dev_i);
                            g_in_scan.store(false);
                        }
                    }
                }
            }
            DWORD tick = g_timer_tick_ms.load();
            if (tick < 1) tick = 1;
            Sleep(tick);
        }
    });

    while (g_app_running.load())
    {
        InterceptionDevice dev = interception_wait(ctx);
        if (!g_app_running.load()) break;
        if (dev == 0) break;

        InterceptionStroke stroke;
        if (interception_receive(ctx, dev, &stroke, 1) <= 0) continue;

        if (!interception_is_keyboard(dev))
        {
            interception_send(ctx, dev, &stroke, 1);
            continue;
        }

        int target = g_selected_device.load();
        if (!g_hook_enabled.load() || target <= 0 || dev != (InterceptionDevice)target)
        {
            interception_send(ctx, dev, &stroke, 1);
            continue;
        }

        InterceptionKeyStroke* k = (InterceptionKeyStroke*)&stroke;

        ULONGLONG now_ms = GetTickCount64();
        bool is_key_down = (k->state & INTERCEPTION_KEY_UP) == 0;
        bool is_enter = (k->code == ENTER_SCANCODE);
        bool is_tab = (k->code == TAB_SCANCODE);
        bool is_end_key = is_enter || is_tab;
        bool is_end_key_up = is_end_key && !is_key_down;

        g_last_activity.store(now_ms);
        g_last_dev.store((int)dev);

        if (!g_injecting.load() && !g_in_scan.load() && is_key_down)
        {
            inject_prefix(ctx, dev);
            g_in_scan.store(true);
        }

        interception_send(ctx, dev, &stroke, 1);

        if (!g_injecting.load() && g_in_scan.load() && is_end_key_up)
        {
            inject_suffix(ctx, dev);
            g_in_scan.store(false);
            g_last_activity.store(GetTickCount64());
        }
    }

    if (timer_thread.joinable()) timer_thread.join();

    {
        std::lock_guard<std::mutex> lock(g_hook_ctx_mutex);
        if (g_hook_ctx_shared == ctx)
        {
            g_hook_ctx_shared = 0;
            interception_destroy_context(ctx);
        }
    }
}
