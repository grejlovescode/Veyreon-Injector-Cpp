# Veyreon Injector — C++ Edition

DirectX 11 + Dear ImGui rewrite. No .NET required. Runs on stock Windows 10/11.

## Build requirements

| Tool | Version |
|------|---------|
| Visual Studio | 2022 (Desktop C++ workload) |
| CMake | 3.20+ |
| Windows SDK | 10.0+ |

## 1 — Get ImGui

Download the **master** branch of [ocornut/imgui](https://github.com/ocornut/imgui):

```
.\
└── vendor\
    └── imgui\          ← extract zip contents here
        ├── imgui.h
        ├── imgui.cpp
        ├── imgui_draw.cpp
        ├── imgui_tables.cpp
        ├── imgui_widgets.cpp
        └── backends\
            ├── imgui_impl_win32.h
            ├── imgui_impl_win32.cpp
            ├── imgui_impl_dx11.h
            └── imgui_impl_dx11.cpp
```

## 2 - Build

Double-click `build.bat`. The exe appears at `bin\Veyreon Injector.exe`.
Run it as **Administrator** (the manifest asks for elevation).

`build\` holds generated files and can be deleted at any time.

## Features

- DLL injection via `LoadLibraryW` / `CreateRemoteThread`  
- Real-time process search & filtering  
- Multi-DLL list with add / remove  
- Stealth mode (`SetWindowDisplayAffinity`)  
- Config save/load (`config.ini`, placed next to the exe)  
- Sidebar pages: Inject, Processes, Modules, Activity, Configs, Settings  
- Flat dark interface with six accent themes; draws only on input, so it idles at no CPU use  
- Fonts embedded in the binary; no runtime DLLs to ship  

The auto-inject option is stored in the config but not implemented yet.

## Project layout

```n.
|-- build.bat           one-click build
|-- CMakeLists.txt
|-- app.manifest        UAC elevation
|-- vendor/imgui/       Dear ImGui
`-- src/
    |-- app/            window, D3D11 setup, resources
    |-- core/           injection and config
    |-- gfx/            embedded fonts
    `-- ui/             draw, widgets, pages, chrome, actions
```
 
