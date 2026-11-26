cmake_minimum_required(VERSION 3.20)
project(VeyreonInjectorCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── MSVC settings ──────────────────────────────────────────────────────────────
if(MSVC)
    # Static runtime — no MSVC runtime DLL needed on target machine
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(
        /W3
        /D_UNICODE /DUNICODE
        /DNOMINMAX
        /DWIN32_LEAN_AND_MEAN
        /MP          # parallel compilation
        /utf-8
        /GS          # stack buffer overrun protection
    )
    add_link_options(
        /SUBSYSTEM:WINDOWS
        /DYNAMICBASE  # ASLR
        /NXCOMPAT     # DEP
    )
endif()

# ── Sources ────────────────────────────────────────────────────────────────────
# ImGui (place in vendor/imgui/ — download from github.com/ocornut/imgui)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

set(APP_SOURCES
    src/app/main.cpp
    src/app/resources.rc    # embeds the UAC manifest
    src/core/config.cpp
    src/core/injector.cpp
    src/ui/actions.cpp
    src/ui/chrome.cpp
    src/ui/draw.cpp
    src/ui/pages.cpp
    src/ui/ui.cpp
    src/ui/widgets.cpp
)

add_executable(VeyreonInjector WIN32 ${APP_SOURCES} ${IMGUI_SOURCES})

# ── Includes ───────────────────────────────────────────────────────────────────
target_include_directories(VeyreonInjector PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ui.cpp includes imgui via "imgui/imgui.h" — alias the include path
# so both "imgui.h" and "imgui/imgui.h" resolve correctly
target_include_directories(VeyreonInjector PRIVATE ${IMGUI_DIR}/..)

# ── Libraries ─────────────────────────────────────────────────────────────────
target_link_libraries(VeyreonInjector PRIVATE
    d3d11
    dxgi
    comdlg32
    shell32
    user32
    gdi32
    dwmapi
    dcomp
    # d3dcompiler removed: shaders are pre-compiled bytecode
)

# ── Output name ───────────────────────────────────────────────────────────────
set_target_properties(VeyreonInjector PROPERTIES
    OUTPUT_NAME "Veyreon Injector"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

# Suppress duplicate manifest: we embed via resources.rc, so tell the linker not to auto-embed one
if(MSVC)
    target_link_options(VeyreonInjector PRIVATE "/MANIFEST:NO")
endif()
