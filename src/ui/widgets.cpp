#include "ui/common.h"

namespace vui {

// ─── Widgets ──────────────────────────────────────────────────────────────────
// Invisible hit box at an absolute position.
bool Hit(const char* id, ImVec2 a, ImVec2 b, bool* hovered, bool* held) {
    ImGui::SetCursorScreenPos(a);
    bool clicked = ImGui::InvisibleButton(id, b - a);
    if (hovered) *hovered = ImGui::IsItemHovered();
    if (held) *held = ImGui::IsItemActive();
    return clicked;
}

bool Button(ImDrawList* dl, const char* id, ImVec2 a, ImVec2 b, unsigned cp, const char* label) {
    bool hovered, held;
    bool clicked = Hit(id, a, b, &hovered, &held);
    ImGui::PushID(id);
    float h = Anim("hover", hovered ? 1.0f : 0.0f);
    ImGui::PopID();

    dl->AddRectFilled(a, b, White((held ? 0.14f : 0.06f) + 0.05f * h), D(8));
    float iconW = D(22);
    float x = (a.x + b.x - iconW - Measure(fValue, label).x) * 0.5f, cy = (a.y + b.y) * 0.5f;
    Icon(dl, fIconSmall, cp, ImVec2(x + D(7), cy), White(0.6f + 0.4f * h));
    TextL(dl, fValue, x + iconW, cy, Gray(236), label);
    return clicked;
}

// Round-cornered icon-only button.
bool IconButton(ImDrawList* dl, const char* id, ImVec2 c, unsigned cp, float idle) {
    bool hovered;
    bool clicked = Hit(id, c - D(13, 13), c + D(13, 13), &hovered);
    if (hovered) dl->AddRectFilled(c - D(13, 13), c + D(13, 13), White(0.08f), D(7));
    Icon(dl, fIconSmall, cp, c, White(hovered ? 0.95f : idle));
    return clicked;
}

// The theme-colored primary action.
bool PrimaryButton(ImDrawList* dl, const char* id, ImVec2 a, ImVec2 b, const char* label) {
    bool hovered, held;
    bool clicked = Hit(id, a, b, &hovered, &held);
    ImGui::PushID(id);
    float h = Anim("hover", hovered ? 1.0f : 0.0f);
    ImGui::PopID();

    float press = held ? D(1.5f) : 0.0f;
    ImVec2 pa = a + ImVec2(press, press), pb = b - ImVec2(press, press);
    if (h > 0.01f) Glow(dl, pa, pb, D(20), Col(g_colA, 0.35f * h));
    GradientRect(dl, pa, pb, D(10), 0.88f + 0.12f * h);

    // Dark label on light themes, white on saturated ones.
    float lum = 0.15f * (g_colA.x + g_colB.x) + 0.30f * (g_colA.y + g_colB.y) + 0.05f * (g_colA.z + g_colB.z);
    ImU32 ink = lum > 0.52f ? Gray(14) : White(1);
    float iconW = D(26);
    float x = (a.x + b.x - iconW - Measure(fBig, label).x) * 0.5f, cy = (a.y + b.y) * 0.5f;
    Icon(dl, fIcon, icon::Bolt, ImVec2(x + D(8), cy), ink);
    TextL(dl, fBig, x + iconW, cy, ink, label);
    return clicked;
}

void Toggle(ImDrawList* dl, const char* id, bool* value, ImVec2 p) {
    ImVec2 size = D(36, 20), q = p + size;
    if (Hit(id, p, q)) *value = !*value;
    ImGui::PushID(id);
    float e = Anim("on", *value ? 1.0f : 0.0f);
    ImGui::PopID();

    float r = size.y * 0.5f;
    dl->AddRectFilled(p, q, White(0.07f * (1.0f - e)), r);
    if (e > 0.01f) GradientRect(dl, p, q, r, e);
    // On the white theme the knob turns dark so it stays visible on the track.
    float lum = 0.3f * g_colB.x + 0.6f * g_colB.y + 0.1f * g_colB.z;
    float on = lum > 0.5f && g_theme == 0 ? 35.0f : 255.0f;
    dl->AddCircleFilled(ImVec2(Lerp(p.x + r, q.x - r, OutCubic(e)), p.y + r), D(6.5f),
                        Gray((int)Lerp(120.0f, on, e)), 24);
}

// Applies the wheel to a scrolled region and eases toward the target.
void UpdateScroll(Scroll& s, ImVec2 a, ImVec2 b, float contentH) {
    float maxScroll = ImMax(0.0f, contentH - (b.y - a.y));
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseHoveringRect(a, b)) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) s.target -= wheel * D(80);
    }
    s.target = ImClamp(s.target, 0.0f, maxScroll);
    s.cur = Approach(s.cur, s.target, 14.0f);

    if (maxScroll > 0.0f) {
        float view = b.y - a.y;
        float barH = ImMax(D(28), view * view / contentH);
        float y = a.y + (view - barH) * Saturate(s.cur / maxScroll);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(ImVec2(b.x + D(4), y), ImVec2(b.x + D(7), y + barH), White(0.18f), D(2));
    }
}

// ─── Cards ────────────────────────────────────────────────────────────────────
// Draws the card chrome and header; EndCard applies the pop-in to everything drawn.

CardScope BeginCard(ImDrawList* dl, int order, ImVec2 a, ImVec2 b, const char* title, const char* desc) {
    CardScope card{ dl->VtxBuffer.Size, (a + b) * 0.5f, g_pageT - 0.07f * order };
    ImGui::PushID(title);
    float hover = Anim("card", ImGui::IsMouseHoveringRect(a, b) ? 1.0f : 0.0f, 10.0f);
    dl->AddRectFilled(a, b, IM_COL32(22, 22, 27, 255), D(10));
    dl->AddRectFilled(a, b, White(Lerp(0.04f, 0.065f, hover)), D(10));
    TextL(dl, fTitle, a.x + D(18), a.y + D(28), Gray(240), title);
    TextL(dl, fDesc, a.x + D(18), a.y + D(49), Gray(205, 0.62f), desc);
    return card;
}

void EndCard(ImDrawList* dl, const CardScope& card) {
    ImGui::PopID();
    float k = Saturate(card.t / 0.30f);
    if (k >= 1.0f) return;
    int ve = dl->VtxBuffer.Size;
    ScaleVerts(dl, card.vs, ve, card.center, Lerp(0.75f, 1.0f, OutBack(k, 0.9f)));
    AlphaVerts(dl, card.vs, ve, Saturate(card.t / 0.16f));
}

} // namespace vui
