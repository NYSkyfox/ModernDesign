#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/PipsPager.h"

namespace ModernDesign {

// WinUI 3 PipsPager token
static constexpr float kPipW = 20.0f;     // PipsPagerButtonWidth
static constexpr float kPipH = 12.0f;     // PipsPagerButtonHeight
static constexpr float kSelW = 24.0f;     // PipsPagerNavigationButtonWidth
static constexpr float kGap = 6.0f;       // 点间距

void PipsPager::Draw(Renderer& r, const Theme& t, float s) {
    if (bounds_.IsEmpty()) return;

    Color un = hasBg_ ? bg_ : (t.lightMode ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.42f));
    Color sel = hasFg_ ? fg_ : t.Accent();

    const float w = kPipW * s, h = kPipH * s, sw = kSelW * s, gap = kGap * s;
    float total = (pageCount_ - 1) * w + (kSelW - kPipW) * s + (pageCount_ - 1) * gap;
    float cx = bounds_.x + bounds_.w * 0.5f;
    float cy = bounds_.y + bounds_.h * 0.5f;
    float x = cx - total * 0.5f;

    for (int i = 0; i < pageCount_; ++i) {
        bool isSel = (i == selected_);
        float bw = isSel ? sw : w;
        r.FillRoundedRect(RectF(x, cy - h * 0.5f, bw, h), h * 0.5f, isSel ? sel : un);
        x += w + gap;
    }
}

void PipsPager::OnMouseUp(float x, float y) {
    if (bounds_.IsEmpty() || !bounds_.Contains(x, y)) return;
    const float w = kPipW, gap = kGap, sw = kSelW;
    float total = (pageCount_ - 1) * w + (sw - kPipW) + (pageCount_ - 1) * gap;
    float start = bounds_.x + bounds_.w * 0.5f - total * 0.5f;
    float cx = x;
    float px = start;
    int best = 0; float bestD = 1e30f;
    for (int i = 0; i < pageCount_; ++i) {
        float bw = (i == selected_) ? sw : w;
        float center = px + bw * 0.5f;
        float d = std::fabsf(cx - center);
        if (d < bestD) { bestD = d; best = i; }
        px += w + gap;
    }
    if (best != selected_) {
        selected_ = best;
        if (cb_) cb_(best);
    }
}

} // namespace ModernDesign