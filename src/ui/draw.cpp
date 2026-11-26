#include "ui/common.h"

namespace vui {

// ─── Drawing ──────────────────────────────────────────────────────────────────
ImVec2 Measure(const Font& f, const char* s) {
    return f.f->CalcTextSizeA(f.size, FLT_MAX, 0.0f, s);
}
void TextL(ImDrawList* dl, const Font& f, float x, float cy, ImU32 col, const char* s) {
    dl->AddText(f.f, f.size, ImVec2(IM_ROUND(x), IM_ROUND(cy - f.size * 0.5f)), col, s);
}
void TextR(ImDrawList* dl, const Font& f, float x, float cy, ImU32 col, const char* s) {
    TextL(dl, f, x - Measure(f, s).x, cy, col, s);
}
void TextC(ImDrawList* dl, const Font& f, float cx, float cy, ImU32 col, const char* s) {
    TextL(dl, f, cx - Measure(f, s).x * 0.5f, cy, col, s);
}
// Left aligned text cut off with an ellipsis at maxW.
void TextFit(ImDrawList* dl, const Font& f, float x, float cy, float maxW, ImU32 col, const std::string& s) {
    if (Measure(f, s.c_str()).x <= maxW) { TextL(dl, f, x, cy, col, s.c_str()); return; }
    std::string cut = s;
    while (!cut.empty()) {
        // Drop one whole UTF-8 character.
        while (!cut.empty() && ((unsigned char)cut.back() & 0xC0) == 0x80) cut.pop_back();
        if (!cut.empty()) cut.pop_back();
        if (Measure(f, (cut + "...").c_str()).x <= maxW) break;
    }
    TextL(dl, f, x, cy, col, (cut + "...").c_str());
}
void Icon(ImDrawList* dl, const Font& f, unsigned cp, ImVec2 c, ImU32 col) {
    char b[8] = {};
    ImTextCharToUtf8(b, cp);
    ImVec2 sz = Measure(f, b);
    dl->AddText(f.f, f.size, ImVec2(IM_ROUND(c.x - sz.x * 0.5f), IM_ROUND(c.y - sz.y * 0.5f)), col, b);
}

void ScaleVerts(ImDrawList* dl, int vs, int ve, ImVec2 c, float s) {
    for (int i = vs; i < ve; i++) {
        ImDrawVert& v = dl->VtxBuffer[i];
        v.pos.x = c.x + (v.pos.x - c.x) * s;
        v.pos.y = c.y + (v.pos.y - c.y) * s;
    }
}
void AlphaVerts(ImDrawList* dl, int vs, int ve, float a) {
    if (a >= 0.999f) return;
    for (int i = vs; i < ve; i++) {
        ImU32& col = dl->VtxBuffer[i].col;
        ImU32 alpha = (ImU32)(((col >> IM_COL32_A_SHIFT) & 0xFF) * Saturate(a));
        col = (col & ~IM_COL32_A_MASK) | (alpha << IM_COL32_A_SHIFT);
    }
}
void RotateVerts(ImDrawList* dl, int vs, int ve, ImVec2 c, float angle) {
    float cs = cosf(angle), sn = sinf(angle);
    for (int i = vs; i < ve; i++) {
        ImDrawVert& v = dl->VtxBuffer[i];
        float x = v.pos.x - c.x, y = v.pos.y - c.y;
        v.pos.x = c.x + x * cs - y * sn;
        v.pos.y = c.y + x * sn + y * cs;
    }
}

// Soft halo built from a few faint rounded rectangles.
void Glow(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 col) {
    const ImU32 alpha = ((col >> IM_COL32_A_SHIFT) & 0xFF) / 5;
    const ImU32 faint = (col & ~IM_COL32_A_MASK) | (alpha << IM_COL32_A_SHIFT);
    for (int i = 1; i <= 3; i++) {
        const float e = r * 0.15f * i;
        dl->AddRectFilled(ImVec2(a.x - e, a.y - e), ImVec2(b.x + e, b.y + e), faint, D(10) + e);
    }
}
void Gradient(ImDrawList* dl, int vs, ImVec2 a, ImVec2 b) {
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(dl, vs, dl->VtxBuffer.Size, a, b, Col(g_colA), Col(g_colB));
}
void GradientRect(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha) {
    int vs = dl->VtxBuffer.Size;
    dl->AddRectFilled(a, b, White(alpha), rounding);
    Gradient(dl, vs, a, b);
}

} // namespace vui
