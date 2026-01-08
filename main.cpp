// Link necessary libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")

#include <vector>
#include <thread>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "globals.h"
#include "string_utils.h"
#include "keyboard_info.h"
#include "interception_logic.h"
#include "config.h"
#include "app_logic.h"
#include "directx_renderer.h"
#include "window_manager.h"
#include "imgui_helpers.h"
#include "startup_manager.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR cmdLine, int)
{
    // FIX: Make the app DPI aware so it doesn't look blurry/stretched
    SetProcessDPIAware();

    bool start_min = has_arg(GetCommandLineW(), L"/minimized");

    config_load();

    if (!CreateDeviceAndWindow(hInstance))
    {
        CleanupDeviceD3D();
        return 1;
    }

    if (start_min)
    {
        window_show(false);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    std::thread hook_thread(hook_thread_main);

    InterceptionContext enum_ctx = interception_create_context();

    std::vector<KeyboardInfo> keyboards;
    ULONGLONG last_refresh = 0;

    int selected_index = -1;
    std::wstring selected_vid;
    std::wstring selected_pid;

    std::string prefix_utf8;
    std::string suffix_utf8;
    {
        std::lock_guard<std::mutex> lock(g_text_mutex);
        prefix_utf8 = wide_to_utf8(g_prefix_w);
        suffix_utf8 = wide_to_utf8(g_suffix_w);
    }

    ensure_capacity(prefix_utf8, 4096);
    ensure_capacity(suffix_utf8, 4096);

    int idle_ms_ui = (int)g_idle_timeout_ms.load();
    int tick_ms_ui = (int)g_timer_tick_ms.load();

    bool startup_installed = startup_task_exists();
    bool done = false;

    while (!done)
    {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ULONGLONG now = GetTickCount64();

        if (enum_ctx && (last_refresh == 0 || (now - last_refresh) >= 1000))
        {
            keyboards.clear();
            for (int d = 1; d <= 20; d++)
            {
                if (!interception_is_keyboard(d)) continue;
                wchar_t hwid_buf[512];
                hwid_buf[0] = 0;
                interception_get_hardware_id(enum_ctx, d, hwid_buf, sizeof(hwid_buf));
                KeyboardInfo ki;
                ki.device = d;
                ki.hwid = hwid_buf;
                extract_vid_pid(ki.hwid, ki.vid, ki.pid);
                keyboards.push_back(ki);
            }
            last_refresh = now;

            int current_dev = g_selected_device.load();
            int new_index = -1;
            for (int i = 0; i < (int)keyboards.size(); i++)
            {
                if (keyboards[i].device == current_dev)
                {
                    new_index = i;
                    break;
                }
            }

            if (new_index == -1)
            {
                if (!g_saved_vid.empty() && !g_saved_pid.empty())
                {
                    for (int i = 0; i < (int)keyboards.size(); i++)
                    {
                        if (keyboards[i].vid == g_saved_vid && keyboards[i].pid == g_saved_pid)
                        {
                            new_index = i;
                            g_selected_device.store(keyboards[i].device);
                            break;
                        }
                    }
                }
            }

            if (new_index != -1)
            {
                selected_index = new_index;
                selected_vid = keyboards[new_index].vid;
                selected_pid = keyboards[new_index].pid;
            }
        }

        maybe_save_cfg();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);

        ImGuiWindowFlags wf = 0;
        wf |= ImGuiWindowFlags_NoTitleBar;
        wf |= ImGuiWindowFlags_NoResize;
        wf |= ImGuiWindowFlags_NoMove;
        wf |= ImGuiWindowFlags_NoCollapse;
        wf |= ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("##Main", nullptr, wf);

        bool hook_ui = g_hook_enabled.load();
        if (ImGui::Checkbox("Hook enabled", &hook_ui))
        {
            set_hook_enabled(hook_ui);
        }

        if (ImGui::InputInt("Idle timeout ms", &idle_ms_ui))
        {
            if (idle_ms_ui < 10) idle_ms_ui = 10;
            if (idle_ms_ui > 2000) idle_ms_ui = 2000;
            g_idle_timeout_ms.store((DWORD)idle_ms_ui);
            mark_cfg_dirty();
        }

        if (ImGui::InputInt("Timer tick ms", &tick_ms_ui))
        {
            if (tick_ms_ui < 1) tick_ms_ui = 1;
            if (tick_ms_ui > 50) tick_ms_ui = 50;
            g_timer_tick_ms.store((DWORD)tick_ms_ui);
            mark_cfg_dirty();
        }

        ImGui::Separator();

        if (startup_installed)
        {
            if (ImGui::Button("Remove startup"))
            {
                startup_task_remove();
                startup_installed = startup_task_exists();
            }
        }
        else
        {
            if (ImGui::Button("Run on startup"))
            {
                startup_task_install();
                startup_installed = startup_task_exists();
            }
        }

        ImGui::SameLine();
        ImGui::Text(startup_installed ? "startup on" : "startup off");

        ImGui::Separator();
        ImGui::Text("Keyboards");

        float list_h = ImGui::GetContentRegionAvail().y * 0.40f;
        if (list_h < 140.0f) list_h = 140.0f;

        if (ImGui::BeginListBox("##kb", ImVec2(-1.0f, list_h)))
        {
            for (int i = 0; i < (int)keyboards.size(); i++)
            {
                const KeyboardInfo& ki = keyboards[i];
                std::wstring linew = std::to_wstring(ki.device) + L"  " + ki.vid + L"  " + ki.pid + L"  " + ki.hwid;
                std::string line = wide_to_utf8(linew);
                bool is_selected = (i == selected_index);
                if (ImGui::Selectable(line.c_str(), is_selected))
                {
                    selected_index = i;
                    selected_vid = ki.vid;
                    selected_pid = ki.pid;
                    g_selected_device.store(ki.device);
                    g_in_scan.store(false);

                    g_saved_vid = ki.vid;
                    g_saved_pid = ki.pid;
                    mark_cfg_dirty();
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        ImGui::Separator();
        ImGui::Text("Selected device %d", g_selected_device.load());
        ImGui::Text("%s", wide_to_utf8(selected_vid).c_str());
        ImGui::Text("%s", wide_to_utf8(selected_pid).c_str());

        ImGui::Separator();
        ImGui::Text("Prefix");
        ensure_capacity(prefix_utf8, 4096);
        if (InputTextMultilineString("##prefix", &prefix_utf8, ImVec2(-1.0f, 110.0f), 0))
        {
            {
                std::lock_guard<std::mutex> lock(g_text_mutex);
                g_prefix_w = utf8_to_wide(prefix_utf8);
            }
            mark_cfg_dirty();
        }

        ImGui::Text("Suffix");
        ensure_capacity(suffix_utf8, 4096);
        if (InputTextMultilineString("##suffix", &suffix_utf8, ImVec2(-1.0f, 110.0f), 0))
        {
            {
                std::lock_guard<std::mutex> lock(g_text_mutex);
                g_suffix_w = utf8_to_wide(suffix_utf8);
            }
            mark_cfg_dirty();
        }

        ImGui::Text("Tray has show hide and start stop and exit");
        ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.12f, 0.12f, 0.12f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    config_save();
    stop_hook_now_for_exit();

    if (hook_thread.joinable()) hook_thread.detach();

    if (enum_ctx) interception_destroy_context(enum_ctx);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_tray_added.load()) tray_remove(g_hwnd);

    CleanupDeviceD3D();
    if (g_hwnd) DestroyWindow(g_hwnd);

    ExitProcess(0);
    return 0;
}
