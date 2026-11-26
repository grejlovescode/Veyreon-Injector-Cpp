#pragma once
#include <windows.h>
#include "imgui/imgui.h"

namespace UI {
    // Loads fonts for the given UI scale. Returns false if a font fails to load.
    bool   Init(float scale);
    void   Render(HWND hwnd);
    void   Shutdown();

    ImVec2 MenuSize();                         // window size in pixels
    float  IntroOpacity();                     // 0-1 launch fade
    bool   TitleDragging();                    // caller moves the OS window while true
}
