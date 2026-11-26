#include "ui/common.h"

namespace vui {

// ─── Chrome ───────────────────────────────────────────────────────────────────
struct Nav { const char* label; unsigned icon; float y; };
const Nav kNav[PageCount] = {
    { "Inject",    icon::Syringe, 116 },
    { "Processes", icon::Chip,    159 },
    { "Modules",   icon::Cubes,   201 },
    { "Activity",  icon::File,    297 },
    { "Configs",   icon::Folder,  339 },
    { "Settings",  icon::Gear,    382 },
};

void DrawSidebar(ImDrawList* dl, ImVec2 size) {
    dl->AddRectFilled(D(0, 56), ImVec2(D(202), size.y), IM_COL32(20, 20, 24, 255), D(10), ImDrawFlags_RoundCornersBottomLeft);
    TextL(dl, fHead, D(18), D(78), White(0.33f), "INJECTOR");
    TextL(dl, fHead, D(18), D(259), White(0.33f), "WORKSPACE");

    for (int i = 0; i < PageCount; i++) {
        const Nav& n = kNav[i];
        float cy = D(n.y);
        ImVec2 a(D(18), cy - D(21)), b(D(182), cy + D(21));
        ImGui::PushID(i);
        bool hovered;
        if (Hit("##nav", a, b, &hovered)) GoTo(i);
        float hover = Anim("hover", hovered ? 1.0f : 0.0f);
        float sel = Anim("sel", g_page == i ? 1.0f : 0.0f);
        ImGui::PopID();

        float pill = ImMax(0.06f * sel, 0.035f * hover);
        if (pill > 0.002f) dl->AddRectFilled(a, b, White(pill), D(10));
        ImVec4 gray(0.72f, 0.72f, 0.72f, 0.82f);
        Icon(dl, fIcon, n.icon, ImVec2(D(42), cy), Col(ImLerp(gray, g_colA, sel)));
        TextL(dl, fNav, D(65), cy, White(Lerp(0.66f, 1.0f, ImMax(sel, hover * 0.6f))), n.label);
        if (sel > 0.02f) Icon(dl, fIconSmall, icon::Right, ImVec2(D(162), cy), White(0.3f * sel));
    }

    DrawCredits(dl);
}

// Developer card pinned to the bottom of the sidebar, with the theme cycle button.
void DrawCredits(ImDrawList* dl) {
    ImVec2 a = D(12, 694), b = D(190, 756);
    dl->AddRectFilled(a, b, White(0.04f), D(12));

    // Avatar: a theme-colored disc with the developer's initial.
    ImVec2 c = D(40, 725);
    float r = D(18);
    GradientRect(dl, c - ImVec2(r, r), c + ImVec2(r, r), r);
    float lum = 0.15f * (g_colA.x + g_colB.x) + 0.30f * (g_colA.y + g_colB.y) + 0.05f * (g_colA.z + g_colB.z);
    TextC(dl, fBig, c.x, c.y, lum > 0.52f ? Gray(14) : White(1), "G");

    TextL(dl, fTiny, D(68), D(715), White(0.40f), "Developed by");
    TextL(dl, fBody, D(68), D(735), Gray(242), "Grej");

    ImVec2 rc = D(168, 725);
    bool hovered;
    if (Hit("##cycle", rc - D(14, 14), rc + D(14, 14), &hovered)) {
        SetTheme((g_theme + 1) % IM_ARRAYSIZE(kThemes));
        g_spinTarget += 1.0f;
    }
    if (hovered) {
        dl->AddRectFilled(rc - D(14, 14), rc + D(14, 14), White(0.08f), D(8));
        ImGui::SetTooltip("Next theme");
    }
    g_spin = Approach(g_spin, g_spinTarget, 7.0f);
    int vs = dl->VtxBuffer.Size;
    Icon(dl, fIconSmall, icon::Rotate, rc, White(hovered ? 1.0f : 0.5f));
    RotateVerts(dl, vs, dl->VtxBuffer.Size, rc, g_spin * IM_PI * 2.0f);
}

void DrawLogo(ImDrawList* dl, ImVec2 o) {
    // Two blades forming a V, on a 30x24 box.
    auto P = [&](float x, float y) { return ImVec2(o.x + D(x), o.y + D(y)); };
    int vs = dl->VtxBuffer.Size;
    ImVec2 left[]  = { P(1, 1), P(9.5f, 1), P(16, 17), P(13, 24), P(11.5f, 24) };
    ImVec2 right[] = { P(29, 0), P(16.5f, 24), P(14, 24), P(17, 16), P(18, 8) };
    dl->AddConvexPolyFilled(left, IM_ARRAYSIZE(left), White(1));
    dl->AddConvexPolyFilled(right, IM_ARRAYSIZE(right), White(0.82f));
    Gradient(dl, vs, P(0, 0), P(30, 24));
}

void DrawTitlebar(ImDrawList* dl, ImVec2 size) {
    ImVec2 b(size.x, D(56));
    dl->AddRectFilled(ImVec2(0, 0), b, IM_COL32(23, 23, 28, 255), D(10), ImDrawFlags_RoundCornersTop);

    TextL(dl, fValue, D(20), D(28), White(0.45f), "v1.1.0  C++");

    float nameW = Measure(fLogo, "Veyreon").x;
    float x = (size.x - D(36) - nameW) * 0.5f;
    DrawLogo(dl, ImVec2(x, D(16)));
    TextL(dl, fLogo, x + D(36), D(28), Gray(245), "Veyreon");

    if (IconButton(dl, "##min", D(1050, 28), icon::Minus)) ShowWindow(g_hwnd, SW_MINIMIZE);
    bool overButton = ImGui::IsItemHovered();
    if (IconButton(dl, "##close", D(1082, 28), icon::Close)) PostQuitMessage(0);
    overButton |= ImGui::IsItemHovered();

    if (ImGui::IsMouseClicked(0) && !overButton && ImGui::IsWindowHovered() &&
        ImGui::IsMouseHoveringRect(ImVec2(0, 0), b))
        g_drag = true;
    if (!ImGui::IsMouseDown(0)) g_drag = false;
}

} // namespace vui
