cmake_minimum_required(VERSION 3.20)
project(keyPrefix LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(IMGUI_DIR "C:/dev/imgui")


if(NOT IMGUI_DIR OR NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "IMGUI_DIR is not set or invalid. Set it to the folder that contains imgui.cpp. Example: -DIMGUI_DIR=C:/dev/imgui")
endif()

if(NOT EXISTS "${IMGUI_DIR}/backends/imgui_impl_win32.cpp" OR NOT EXISTS "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp")
    message(FATAL_ERROR "ImGui backends not found in ${IMGUI_DIR}/backends")
endif()

if(MSVC)
    enable_language(RC)
endif()

add_executable(keyPrefix WIN32
        main.cpp
        globals.cpp
        string_utils.cpp
        interception_logic.cpp
        config.cpp
        app_logic.cpp
        directx_renderer.cpp
        window_manager.cpp
        imgui_helpers.cpp
        startup_manager.cpp
        interception.c
        resource.rc
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)

target_include_directories(keyPrefix PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
)

target_compile_definitions(keyPrefix PRIVATE
        INTERCEPTION_STATIC
        INTERCEPTION_API=
        NOMINMAX
)

target_link_libraries(keyPrefix PRIVATE
        d3d11
        dxgi
        d3dcompiler
        dwmapi
        user32
        gdi32
        imm32
        ole32
        shell32
)

