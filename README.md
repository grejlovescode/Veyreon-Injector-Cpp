# Veyreon Injector — C++ Edition

A lightweight, modern DLL injector for Windows built with C++17, Dear ImGui, and DirectX 11. Features a borderless transparent UI with per-pixel alpha compositing via DirectComposition. No .NET required — runs on stock Windows 10/11.

---

## Features

- **Process picker** — live searchable list of all running processes with real-time filtering
- **Multi-DLL support** — queue multiple modules and inject them in order
- **Activity log** — timestamped event log for the current session
- **Config system** — save and load your module list and settings to `config.ini`
- **Stealth mode** — hides the window from screen capture and recording tools via `SetWindowDisplayAffinity`
- **Close after injection** — exits automatically once every module loads
- **Six accent themes** — switchable accent colors in Settings
- **Zero idle CPU** — redraws only on input; sleeps otherwise
- **No runtime dependencies** — shaders are pre-compiled; no `d3dcompiler_47.dll` required; fonts embedded in the binary

---

## Requirements

- Windows 10 or later
- Administrator privileges (required to open most processes)

---

## Building from source

**Prerequisites:** Visual Studio 2022 (Desktop C++ workload), CMake 3.20+

**1. Get ImGui**

Download the master branch of [ocornut/imgui](https://github.com/ocornut/imgui) and place it at `vendor\imgui\`:

```
vendor\imgui\
├── imgui.h / imgui.cpp
├── imgui_draw.cpp / imgui_tables.cpp / imgui_widgets.cpp
└── backends\
    ├── imgui_impl_win32.h / imgui_impl_win32.cpp
    └── imgui_impl_dx11.h  / imgui_impl_dx11.cpp
```

**2. Build**

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The compiled binary is written to `bin\Veyreon Injector.exe`. Run it as Administrator.

---

## Injection method

Uses the classic `LoadLibrary` injection:
`OpenProcess` → `VirtualAllocEx` → `WriteProcessMemory` → `CreateRemoteThread`

---

## Project layout

```
CMakeLists.txt
app.manifest          UAC elevation
vendor\imgui\         Dear ImGui
src\
├── app\              WinMain, D3D11 setup, resources
├── core\             injection logic, config I/O
├── gfx\              embedded fonts
└── ui\               draw, widgets, pages, sidebar, actions
```

---

## License

MIT
