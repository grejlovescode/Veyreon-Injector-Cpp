#include "ui/common.h"
#include <cstdio>
#include "core/config.h"

namespace vui {

// ─── Lists ────────────────────────────────────────────────────────────────────
void DrawDllList(ImDrawList* dl, ImVec2 a, ImVec2 b, Scroll& scroll) {
    dl->AddRectFilled(a, b, Black(0.28f), D(8));
    if (g_dlls.empty()) {
        float cx = (a.x + b.x) * 0.5f, cy = (a.y + b.y) * 0.5f;
        Icon(dl, fIconBig, icon::Cubes, ImVec2(cx, cy - D(18)), White(0.18f));
        TextC(dl, fDesc, cx, cy + D(20), White(0.35f), "No modules yet. Use Add DLL.");
        return;
    }

    const float itemH = D(52);
    ImVec2 ia = a + D(6, 6), ib = b - D(6, 6);
    UpdateScroll(scroll, ia, ib, itemH * g_dlls.size());

    int remove = -1;
    dl->PushClipRect(ia, ib, true);
    ImGui::PushClipRect(ia, ib, true);
    for (int i = 0; i < (int)g_dlls.size(); i++) {
        float y = ia.y + itemH * i - scroll.cur;
        if (y + itemH < ia.y || y > ib.y) continue;
        ImVec2 ra(ia.x, y + D(2)), rb(ib.x, y + itemH - D(2));
        float cy = (ra.y + rb.y) * 0.5f;
        ImGui::PushID(i);

        bool xClicked = IconButton(dl, "##x", ImVec2(rb.x - D(22), cy), icon::Close);
        bool xHovered = ImGui::IsItemHovered();
        bool hovered;
        if (Hit("##row", ra, ImVec2(rb.x - D(40), rb.y), &hovered)) g_selDll = i;
        if (xClicked) remove = i;

        bool sel = g_selDll == i;
        if (sel || hovered || xHovered) dl->AddRectFilled(ra, rb, White(sel ? 0.09f : 0.045f), D(7));
        if (sel) GradientRect(dl, ImVec2(ra.x, ra.y + D(10)), ImVec2(ra.x + D(3), rb.y - D(10)), D(2));

        std::string path = WideToUtf8(g_dlls[i]);
        Icon(dl, fIcon, icon::File, ImVec2(ra.x + D(24), cy), Col(sel ? g_colA : ImVec4(0.65f, 0.65f, 0.68f, 1)));
        float textW = rb.x - ra.x - D(90);
        TextFit(dl, fValue, ra.x + D(46), cy - D(9), textW, Gray(240), FileName(path));
        TextFit(dl, fTiny, ra.x + D(46), cy + D(10), textW, White(0.38f), path);
        // Redraw the close glyph above the row highlight.
        Icon(dl, fIconSmall, icon::Close, ImVec2(rb.x - D(22), cy), White(xHovered ? 0.95f : 0.35f));
        ImGui::PopID();
    }
    ImGui::PopClipRect();
    dl->PopClipRect();
    RemoveDll(remove);
}

void DrawLog(ImDrawList* dl, ImVec2 a, ImVec2 b, Scroll& scroll) {
    if (g_log.empty()) {
        TextL(dl, fDesc, a.x, a.y + D(12), White(0.32f), "Nothing has happened yet.");
        return;
    }
    const float itemH = D(30);
    UpdateScroll(scroll, a, b, itemH * g_log.size());
    dl->PushClipRect(a, b, true);
    for (int i = 0; i < (int)g_log.size(); i++) {
        float cy = a.y + itemH * (i + 0.5f) - scroll.cur;
        if (cy + itemH < a.y || cy - itemH > b.y) continue;
        const LogEntry& e = g_log[i];
        dl->AddCircleFilled(ImVec2(a.x + D(4), cy), D(3.5f), ToneColor(e.tone), 16);
        TextL(dl, fTiny, a.x + D(18), cy, White(0.38f), e.time.c_str());
        TextFit(dl, fDesc, a.x + D(84), cy, b.x - a.x - D(84), Gray(225), e.text);
    }
    dl->PopClipRect();
}

// ─── Pages ────────────────────────────────────────────────────────────────────
// Content area: two columns (218..649, 660..1090) below the 56px titlebar.
const float kLeft = 218, kRight = 660, kFarRight = 1090, kTop = 72, kBottom = 752;

void OptionCard(ImDrawList* dl, int order, float y, const char* title, const char* desc, bool* value) {
    ImVec2 a = D(kRight, y), b = D(kFarRight, y + 74);
    CardScope card = BeginCard(dl, order, a, b, title, desc);
    Toggle(dl, "##opt", value, ImVec2(b.x - D(54), a.y + D(27)));
    EndCard(dl, card);
}

void PageInjectDraw(ImDrawList* dl) {
    // Target process
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(649, 258), "Target process",
                               "The running process that receives the modules.");
    {
        float cy = D(176);
        TextL(dl, fBody, D(236), cy - D(13), Gray(236), "Process");
        TextL(dl, fBody, D(236), cy + D(13), Gray(236), "Process ID");
        if (g_hasProcess) {
            TextFit(dl, fValue, D(360), cy - D(13), D(270), Gray(240), g_selProcess.nameUtf8);
            TextL(dl, fValue, D(360), cy + D(13), Gray(240), std::to_string(g_selProcess.pid).c_str());
        } else {
            TextL(dl, fValue, D(360), cy - D(13), White(0.30f), "None selected");
            TextL(dl, fValue, D(360), cy + D(13), White(0.30f), "-");
        }
        if (Button(dl, "##select", D(236, 208), D(631, 242), icon::Search, "Select process"))
            GoTo(PageProcesses);
    }
    EndCard(dl, card);

    // Modules
    card = BeginCard(dl, 1, D(kLeft, 270), D(649, kBottom), "Modules", "DLLs to inject, in list order.");
    {
        char count[32];
        snprintf(count, sizeof count, "%d loaded", (int)g_dlls.size());
        TextR(dl, fValue, D(631), D(298), White(0.5f), count);
        DrawDllList(dl, D(236, 344), D(631, 686), g_scrollMini);
        if (Button(dl, "##add", D(236, 700), D(428, 734), icon::Plus, "Add DLL")) AddDlls();
        if (Button(dl, "##rem", D(439, 700), D(631, 734), icon::Minus, "Remove")) RemoveDll(g_selDll);
    }
    EndCard(dl, card);

    OptionCard(dl, 1, 72, "Stealth mode", "Hide the window from screen capture.", &g_stealthMode);
    OptionCard(dl, 2, 158, "Close after injection", "Exit once every module has loaded.", &g_closeAfterInject);
    OptionCard(dl, 3, 244, "Auto-inject", "Saved with your config. No effect yet.", &g_autoInject);

    // Execute
    card = BeginCard(dl, 4, D(kRight, 330), D(kFarRight, 506), "Execute", "Inject every module into the target.");
    {
        if (PrimaryButton(dl, "##inject", D(678, 406), D(1072, 458), "Execute injection")) Execute();
        dl->AddCircleFilled(D(682, 482), D(4), ToneColor(g_statusTone), 16);
        TextFit(dl, fDesc, D(696), D(482), D(374), ToneColor(g_statusTone), g_statusText);
    }
    EndCard(dl, card);

    // Recent activity
    card = BeginCard(dl, 5, D(kRight, 518), D(kFarRight, kBottom), "Recent activity", "Latest events from this session.");
    DrawLog(dl, D(678, 596), D(1068, 736), g_scrollLog);
    EndCard(dl, card);
}

void PageProcessesDraw(ImDrawList* dl) {
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(kFarRight, kBottom), "Processes",
                               "Click a process to target it. Double-click to return.");
    char count[48];
    snprintf(count, sizeof count, "%d processes", (int)g_filteredProcs.size());
    TextR(dl, fValue, D(1030), D(109), White(0.5f), count);
    if (IconButton(dl, "##refresh", D(1058, 109), icon::Rotate, 0.65f)) RefreshProcesses();

    // Search field
    dl->AddRectFilled(D(236, 140), D(1072, 178), Black(0.28f), D(8));
    Icon(dl, fIconSmall, icon::Search, D(256, 159), White(0.35f));
    ImGui::SetCursorScreenPos(ImVec2(D(272), D(159) - (fValue.size + D(10)) * 0.5f));
    ImGui::SetNextItemWidth(D(790));
    ImGui::PushFont(fValue.f, fValue.size);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, D(6, 5));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(g_colA.x, g_colA.y, g_colA.z, 0.35f));
    if (ImGui::InputTextWithHint("##search", "Search by name or PID...", g_searchBuf, sizeof g_searchBuf)) {
        FilterProcesses();
        g_scrollProcs = {};
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::PopFont();

    const float itemH = D(36);
    ImVec2 a = D(236, 190), b = D(1068, 736);
    UpdateScroll(g_scrollProcs, a, b, itemH * g_filteredProcs.size());

    dl->PushClipRect(a, b, true);
    ImGui::PushClipRect(a, b, true);
    int open = -1;
    for (int i = 0; i < (int)g_filteredProcs.size(); i++) {
        float y = a.y + itemH * i - g_scrollProcs.cur;
        if (y + itemH < a.y || y > b.y) continue;
        const ProcessInfo& p = g_filteredProcs[i];
        ImVec2 ra(a.x, y + D(1)), rb(b.x, y + itemH - D(1));
        float cy = (ra.y + rb.y) * 0.5f;

        ImGui::PushID((int)p.pid);
        bool hovered;
        if (Hit("##proc", ra, rb, &hovered)) {
            if (!g_hasProcess || g_selProcess.pid != p.pid) {
                g_selProcess = p;
                g_hasProcess = true;
                SetStatus("Selected: " + p.nameUtf8, Tone::Success);
            }
        }
        if (hovered && ImGui::IsMouseDoubleClicked(0)) open = i;
        ImGui::PopID();

        bool sel = g_hasProcess && g_selProcess.pid == p.pid;
        if (sel || hovered) dl->AddRectFilled(ra, rb, White(sel ? 0.10f : 0.05f), D(7));
        if (sel) GradientRect(dl, ImVec2(ra.x, ra.y + D(8)), ImVec2(ra.x + D(3), rb.y - D(8)), D(2));
        Icon(dl, fIconSmall, icon::Chip, ImVec2(ra.x + D(24), cy),
             Col(sel ? g_colA : ImVec4(0.55f, 0.55f, 0.58f, 1)));
        TextFit(dl, fValue, ra.x + D(46), cy, D(600), Gray(sel ? 250 : 225), p.nameUtf8);
        if (sel) Icon(dl, fIconSmall, icon::Check, ImVec2(rb.x - D(110), cy), Col(g_colA));
        char pid[24];
        snprintf(pid, sizeof pid, "%lu", (unsigned long)p.pid);
        TextR(dl, fValue, rb.x - D(14), cy, White(0.45f), pid);
    }
    ImGui::PopClipRect();
    dl->PopClipRect();
    if (g_filteredProcs.empty())
        TextC(dl, fDesc, D(652), D(240), White(0.35f), "No process matches your search.");
    EndCard(dl, card);
    if (open >= 0) GoTo(PageInject);
}

void PageModulesDraw(ImDrawList* dl) {
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(kFarRight, kBottom), "Modules",
                               "Every DLL queued for injection, with its full path.");
    char count[32];
    snprintf(count, sizeof count, "%d loaded", (int)g_dlls.size());
    TextR(dl, fValue, D(1072), D(109), White(0.5f), count);
    DrawDllList(dl, D(236, 142), D(1072, 686), g_scrollDlls);
    if (Button(dl, "##add", D(236, 700), D(648, 734), icon::Plus, "Add DLL")) AddDlls();
    if (Button(dl, "##clear", D(660, 700), D(1072, 734), icon::Trash, "Clear list")) {
        g_dlls.clear();
        g_selDll = -1;
    }
    EndCard(dl, card);
}

void PageActivityDraw(ImDrawList* dl) {
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(kFarRight, kBottom), "Event log",
                               "Newest first. Cleared when the app closes.");
    if (IconButton(dl, "##clearlog", D(1058, 109), icon::Trash, 0.65f)) g_log.clear();
    DrawLog(dl, D(236, 148), D(1068, 734), g_scrollLog);
    EndCard(dl, card);
}

void PageConfigsDraw(ImDrawList* dl) {
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(649, 206), "Save configuration",
                               "Store the module list and options.");
    if (Button(dl, "##save", D(236, 154), D(631, 188), icon::Save, "Save config")) SaveConfig();
    EndCard(dl, card);

    card = BeginCard(dl, 1, D(kRight, kTop), D(kFarRight, 206), "Load configuration",
                     "Replace the current list and options.");
    if (Button(dl, "##load", D(678, 154), D(1072, 188), icon::Load, "Load config")) LoadConfig();
    EndCard(dl, card);

    card = BeginCard(dl, 2, D(kLeft, 218), D(kFarRight, 332), "Config file", "Kept next to the executable.");
    static const std::string path = WideToUtf8(ConfigIO::GetExeDir()) + "\\config.ini";
    TextFit(dl, fValue, D(236), D(304), D(836), Gray(230), path);
    EndCard(dl, card);
}

void SetTheme(int i) {
    g_theme = i;
}

void PageSettingsDraw(ImDrawList* dl) {
    CardScope card = BeginCard(dl, 0, D(kLeft, kTop), D(649, 396), "Theme",
                               "Accent color for the controls.");
    for (int i = 0; i < IM_ARRAYSIZE(kThemes); i++) {
        float y = 142.0f + 40.0f * i;
        ImVec2 a = D(228, y), b = D(639, y + 36);
        ImGui::PushID(i);
        bool hovered;
        if (Hit("##theme", a, b, &hovered)) SetTheme(i);
        ImGui::PopID();
        if (hovered || g_theme == i) dl->AddRectFilled(a, b, White(g_theme == i ? 0.09f : 0.045f), D(7));
        float cy = (a.y + b.y) * 0.5f;
        dl->AddCircleFilled(ImVec2(a.x + D(20), cy), D(8), Col(Hex(kThemes[i].a)), 24);
        dl->AddCircleFilled(ImVec2(a.x + D(40), cy), D(8), Col(Hex(kThemes[i].b)), 24);
        TextL(dl, fValue, a.x + D(62), cy, Gray(235), kThemes[i].name);
        if (g_theme == i) Icon(dl, fIconSmall, icon::Check, ImVec2(b.x - D(18), cy), White(0.9f));
    }
    EndCard(dl, card);

    card = BeginCard(dl, 1, D(kRight, kTop), D(kFarRight, 186), "About", "Veyreon Injector, C++ edition.");
    TextL(dl, fValue, D(678), D(158), Gray(230), "Version 1.1.0");
    TextR(dl, fValue, D(1072), D(158), White(0.6f), "Developed by Grej");
    EndCard(dl, card);
}

} // namespace vui
