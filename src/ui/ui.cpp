// Public entry points of the interface.
#include "ui/common.h"
#include "gfx/fonts_embedded.h"
#include "ui/ui.h"

using namespace vui;

// ─── Public API ───────────────────────────────────────────────────────────────
namespace UI {

bool Init(float scale) {
    S = scale;
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    auto load = [&](const unsigned char* data, int bytes, float size) {
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;
        Font font;
        font.size = size * S;
        font.f = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(data), bytes, font.size, &cfg);
        return font;
    };

    fBody      = load(g_font_inter_600, g_font_inter_600_size, 16.5f);
    fTitle     = load(g_font_inter_600, g_font_inter_600_size, 17.0f);
    fDesc      = load(g_font_inter_400, g_font_inter_400_size, 15.5f);
    fValue     = load(g_font_inter_500, g_font_inter_500_size, 15.5f);
    fNav       = load(g_font_inter_500, g_font_inter_500_size, 17.5f);
    fHead      = load(g_font_inter_500, g_font_inter_500_size, 14.5f);
    fLogo      = load(g_font_inter_600, g_font_inter_600_size, 18.0f);
    fTiny      = load(g_font_inter_400, g_font_inter_400_size, 13.5f);
    fBig       = load(g_font_inter_600, g_font_inter_600_size, 18.0f);
    fIcon      = load(g_font_fa_solid, g_font_fa_solid_size, 18.0f);
    fIconSmall = load(g_font_fa_solid, g_font_fa_solid_size, 14.0f);
    fIconBig   = load(g_font_fa_solid, g_font_fa_solid_size, 30.0f);
    for (const Font* f : { &fBody, &fTitle, &fDesc, &fValue, &fNav, &fHead, &fLogo, &fTiny, &fBig,
                           &fIcon, &fIconSmall, &fIconBig })
        if (!f->f) return false;
    io.FontDefault = fValue.f;

    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowPadding    = ImVec2(0, 0);
    st.WindowBorderSize = 0;
    st.FrameRounding    = D(6);
    st.FontSizeBase     = fValue.size;

    g_colA = Hex(kThemes[g_theme].a);
    g_colB = Hex(kThemes[g_theme].b);
    return true;
}

ImVec2 MenuSize() { return D(1108, 768); }
bool TitleDragging() { return g_drag; }

float IntroOpacity() {
    float k = Saturate((g_introT - 0.1f) / 0.7f);
    return k * k * (3.0f - 2.0f * k);
}

void Render(HWND hwnd) {
    g_hwnd = hwnd;

    // Apply stealth mode if toggled
    static bool prevStealth = false;
    if (g_stealthMode != prevStealth) {
        SetWindowDisplayAffinity(hwnd, g_stealthMode ? 0x00000011u : 0u);
        prevStealth = g_stealthMode;
    }

    // Clamp so a blocking file dialog or injection does not skip the animations.
    float dt = std::min(Dt(), 0.05f);
    g_introT += dt;
    g_pageT += dt;
    ImVec4 a = Hex(kThemes[g_theme].a), b = Hex(kThemes[g_theme].b);
    g_colA = ImLerp(g_colA, a, 1.0f - expf(-7.0f * Dt()));
    g_colB = ImLerp(g_colB, b, 1.0f - expf(-7.0f * Dt()));

    ImVec2 size = MenuSize();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("##root", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(ImVec2(0, 0), size, IM_COL32(13, 13, 16, 255), D(10));

    switch (g_page) {
    case PageInject:    PageInjectDraw(dl); break;
    case PageProcesses: PageProcessesDraw(dl); break;
    case PageModules:   PageModulesDraw(dl); break;
    case PageActivity:  PageActivityDraw(dl); break;
    case PageConfigs:   PageConfigsDraw(dl); break;
    case PageSettings:  PageSettingsDraw(dl); break;
    }

    DrawSidebar(dl, size);
    DrawTitlebar(dl, size);
    ImGui::End();
}

void Shutdown() {}

} // namespace UI
