// Shared state, helpers and declarations for the interface sources.
// Layout numbers are design units for a 1108x768 window; D() applies the scale.
#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "core/injector.h"

namespace vui {

// ─── Scale ────────────────────────────────────────────────────────────────────
inline float S = 1.0f;
inline float  D(float v)          { return v * S; }
inline ImVec2 D(float x, float y) { return ImVec2(x * S, y * S); }

// ─── Types ────────────────────────────────────────────────────────────────────
struct Font { ImFont* f = nullptr; float size = 0.0f; };

namespace icon {
    const unsigned Syringe = 0xf48e, Chip = 0xf2db, Cubes = 0xf1b3, File = 0xf15b,
                   Folder = 0xf07b, Gear = 0xf013, Search = 0xf002, Rotate = 0xf021,
                   Plus = 0xf067, Close = 0xf00d, Minus = 0xf068, Check = 0xf00c,
                   Bolt = 0xf0e7, Save = 0xf0c7, Load = 0xf07c, Trash = 0xf1f8,
                   Right = 0xf054;
}

enum Page { PageInject, PageProcesses, PageModules, PageActivity, PageConfigs, PageSettings, PageCount };
enum class Tone { Info, Success, Warn, Error };
struct LogEntry  { std::string time, text; Tone tone; };
struct Scroll    { float target = 0.0f, cur = 0.0f; };
struct Theme     { const char* name; unsigned a, b; };
struct CardScope { int vs; ImVec2 center; float t; };

const Theme kThemes[] = {
    { "Mono",    0xFFFFFF, 0x8C8C8C },
    { "Veyreon", 0x7C5CFF, 0x5CE1FF },
    { "Violet",  0x2500FF, 0xC400FF },
    { "Crimson", 0xFF0044, 0xFF00B0 },
    { "Ocean",   0x0078FF, 0x00FFE6 },
    { "Ember",   0xFF7700, 0xFFEE00 },
};

// ─── State ────────────────────────────────────────────────────────────────────
inline Font fTitle, fBody, fDesc, fValue, fNav, fHead, fLogo, fTiny, fBig;
inline Font fIcon, fIconSmall, fIconBig;

inline int   g_page      = PageInject;
inline float g_pageT     = 0.0f;   // time since the current page was opened
inline float g_introT    = 0.0f;
inline bool  g_drag      = false;
inline HWND  g_hwnd      = nullptr;

inline bool                     g_hasProcess = false;
inline ProcessInfo              g_selProcess = {};
inline std::vector<ProcessInfo> g_allProcs;
inline std::vector<ProcessInfo> g_filteredProcs;
inline char                     g_searchBuf[256] = {};

inline std::vector<std::wstring> g_dlls;
inline int                       g_selDll = -1;

inline bool g_stealthMode      = false;
inline bool g_closeAfterInject = false;
inline bool g_autoInject       = false;

inline std::vector<LogEntry> g_log;
inline std::string           g_statusText = "Ready.";
inline Tone                  g_statusTone = Tone::Info;

inline Scroll g_scrollProcs, g_scrollDlls, g_scrollMini, g_scrollLog;

inline int    g_theme = 0;
inline ImVec4 g_colA(1, 1, 1, 1), g_colB(0.55f, 0.55f, 0.55f, 1);
inline float  g_spin = 0.0f, g_spinTarget = 0.0f;

// ─── Math and color ───────────────────────────────────────────────────────────
inline float Saturate(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float Dt() { return ImGui::GetIO().DeltaTime; }
inline float Approach(float cur, float target, float speed) {
    return cur + (target - cur) * (1.0f - expf(-speed * Dt()));
}
inline float OutCubic(float x) { float k = 1.0f - x; return 1.0f - k * k * k; }
inline float OutBack(float x, float c) {
    float k = x - 1.0f;
    return 1.0f + (c + 1.0f) * k * k * k + c * k * k;
}
// Per-widget animation value, kept in ImGui storage under the current ID stack.
inline float Anim(const char* key, float target, float speed = 12.0f) {
    ImGuiStorage* st = ImGui::GetStateStorage();
    ImGuiID id = ImGui::GetID(key);
    float v = Approach(st->GetFloat(id, target), target, speed);
    st->SetFloat(id, v);
    return v;
}

inline ImVec4 Hex(unsigned rgb) {
    return ImVec4(((rgb >> 16) & 255) / 255.0f, ((rgb >> 8) & 255) / 255.0f, (rgb & 255) / 255.0f, 1);
}
inline ImU32 Col(const ImVec4& c, float a = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * a));
}
inline ImU32 White(float a) { return IM_COL32(255, 255, 255, (int)(Saturate(a) * 255.0f)); }
inline ImU32 Black(float a) { return IM_COL32(0, 0, 0, (int)(Saturate(a) * 255.0f)); }
inline ImU32 Gray(int g, float a = 1.0f) { return IM_COL32(g, g, g, (int)(Saturate(a) * 255.0f)); }

// ─── draw.cpp ─────────────────────────────────────────────────────────────────
inline ImU32 ToneColor(Tone t) {
    switch (t) {
    case Tone::Success: return IM_COL32(72, 220, 135, 255);
    case Tone::Warn:    return IM_COL32(255, 188, 70, 255);
    case Tone::Error:   return IM_COL32(255, 92, 100, 255);
    default:            return IM_COL32(165, 165, 172, 255);
    }
}
ImVec2 Measure(const Font& f, const char* s);
void   TextL(ImDrawList* dl, const Font& f, float x, float cy, ImU32 col, const char* s);
void   TextR(ImDrawList* dl, const Font& f, float x, float cy, ImU32 col, const char* s);
void   TextC(ImDrawList* dl, const Font& f, float cx, float cy, ImU32 col, const char* s);
void   TextFit(ImDrawList* dl, const Font& f, float x, float cy, float maxW, ImU32 col, const std::string& s);
void   Icon(ImDrawList* dl, const Font& f, unsigned cp, ImVec2 c, ImU32 col);
void   ScaleVerts(ImDrawList* dl, int vs, int ve, ImVec2 c, float s);
void   AlphaVerts(ImDrawList* dl, int vs, int ve, float a);
void   RotateVerts(ImDrawList* dl, int vs, int ve, ImVec2 c, float angle);
void   Glow(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 col);
void   Gradient(ImDrawList* dl, int vs, ImVec2 a, ImVec2 b);
void   GradientRect(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha = 1.0f);

// ─── widgets.cpp ──────────────────────────────────────────────────────────────
bool      Hit(const char* id, ImVec2 a, ImVec2 b, bool* hovered = nullptr, bool* held = nullptr);
bool      Button(ImDrawList* dl, const char* id, ImVec2 a, ImVec2 b, unsigned cp, const char* label);
bool      IconButton(ImDrawList* dl, const char* id, ImVec2 c, unsigned cp, float idle = 0.35f);
bool      PrimaryButton(ImDrawList* dl, const char* id, ImVec2 a, ImVec2 b, const char* label);
void      Toggle(ImDrawList* dl, const char* id, bool* value, ImVec2 p);
void      UpdateScroll(Scroll& s, ImVec2 a, ImVec2 b, float contentH);
CardScope BeginCard(ImDrawList* dl, int order, ImVec2 a, ImVec2 b, const char* title, const char* desc);
void      EndCard(ImDrawList* dl, const CardScope& card);

// ─── actions.cpp ──────────────────────────────────────────────────────────────
std::string WideToUtf8(const std::wstring& w);
std::string FileName(const std::string& path);
std::string Lower(std::string s);
void SetStatus(const std::string& msg, Tone tone);
void FilterProcesses();
void RefreshProcesses();
void GoTo(int page);
void AddDlls();
void RemoveDll(int i);
void Execute();
void SaveConfig();
void LoadConfig();

// ─── pages.cpp ────────────────────────────────────────────────────────────────
void SetTheme(int i);
void PageInjectDraw(ImDrawList* dl);
void PageProcessesDraw(ImDrawList* dl);
void PageModulesDraw(ImDrawList* dl);
void PageActivityDraw(ImDrawList* dl);
void PageConfigsDraw(ImDrawList* dl);
void PageSettingsDraw(ImDrawList* dl);

// ─── chrome.cpp ───────────────────────────────────────────────────────────────
void DrawSidebar(ImDrawList* dl, ImVec2 size);
void DrawCredits(ImDrawList* dl);
void DrawLogo(ImDrawList* dl, ImVec2 o);
void DrawTitlebar(ImDrawList* dl, ImVec2 size);

} // namespace vui
