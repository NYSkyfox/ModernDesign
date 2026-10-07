#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/RatingControl.h"
#include "utils/FluentIcons.h"

namespace ModernDesign {

// WinUI 3 RatingControl token
static constexpr float kStarSize = 20.0f;   // FontSizeForRendering
static constexpr float kItemGap = 2.0f;     // RatingControlItemSpacing

void RatingControl::OnMouseMove(float x, float y) {
    if (readOnly_ || bounds_.IsEmpty()) { hover_ = 0; return; }
    if (!bounds_.Contains(x, y)) { hover_ = 0; return; }
    float total = max_ * kStarSize + (max_ - 1) * kItemGap;
    float cx = bounds_.x + bounds_.w * 0.5f - total * 0.5f;
    float lx = x - cx;
    int v = 0;
    for (int i = 0; i < max_; ++i) {
        float s0 = i * (kStarSize + kItemGap);
        if (lx < s0) break;
        float local = (lx - s0) / kStarSize;
        if (local > 1.0f) local = 1.0f;
        v = i + 1;
        if (local < 0.5f) { /* 左半：半星，整数表示为 i（不填满）*/ v = i; }
    }
    hover_ = v;
}

void RatingControl::OnMouseLeave() { hover_ = 0; }

void RatingControl::OnMouseUp(float x, float y) {
    if (readOnly_) return;
    if (bounds_.IsEmpty() || !bounds_.Contains(x, y)) return;
    float total = max_ * kStarSize + (max_ - 1) * kItemGap;
    float cx = bounds_.x + bounds_.w * 0.5f - total * 0.5f;
    float lx = x - cx;
    int v = 0;
    for (int i = 0; i < max_; ++i) {
        float s0 = i * (kStarSize + kItemGap);
        if (lx < s0) break;
        float local = (lx - s0) / kStarSize;
        if (local > 1.0f) local = 1.0f;
        v = (local < 0.5f) ? i : (i + 1);
    }
    if (v != value_) {
        value_ = v;
        if (cb_) cb_(v);
    }
}

void RatingControl::Draw(Renderer& r, const Theme& t, float s) {
    if (bounds_.IsEmpty()) return;

    Color fill = hasFg_ ? fg_ : t.Accent();
    Color empty = t.lightMode ? Color(0, 0, 0, 0.40f) : Color(1, 1, 1, 0.40f);

    const float size = kStarSize * s, gap = kItemGap * s;
    float total = max_ * size + (max_ - 1) * gap;
    float cx = bounds_.x + bounds_.w * 0.5f - total * 0.5f;
    float cy = bounds_.y + bounds_.h * 0.5f;

    int shown = hover_ > 0 ? hover_ : value_;

    for (int i = 0; i < max_; ++i) {
        RectF box(cx + i * (size + gap), cy - size * 0.5f, size, size);
        bool on = (i < shown);
        if (on) {
            DrawFluentIconCentered(r, FluentIcon::StarFilled, box, size, fill);
        } else {
            DrawFluentIconCentered(r, FluentIcon::Star, box, size, empty);
        }
    }
}

} // namespace ModernDesign