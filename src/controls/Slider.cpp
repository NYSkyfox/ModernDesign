#include "pch.h"
#include "controls/Slider.h"

namespace ModernDesign {

static constexpr float kSliderTrackHeight = 4.0f;
static constexpr float kSliderThumbSize = 16.0f;
static constexpr float kSliderThumbRadius = 8.0f;

void Slider::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
    if (dragging_) {
        float t = Clamp01((x - bounds_.x) / bounds_.w);
        value_ = t;
        if (onValue_) onValue_(value_);
    }
}

void Slider::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        dragging_ = true;
        float t = Clamp01((x - bounds_.x) / bounds_.w);
        value_ = t;
        if (onValue_) onValue_(value_);
    }
}

void Slider::OnMouseUp(float x, float y) {
    (void)x; (void)y;
    dragging_ = false;
}

bool Slider::Update(float dt) {
    float target = (hot_ || dragging_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, target, dt, 18.0f);
    return std::abs(hoverT_ - target) > 0.005f;
}

void Slider::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();
    Color trackBg = theme.lightMode ? Color(0,0,0,0.12f) : Color(1,1,1,0.24f);
    Color thumbColor = dragging_ || hot_ ? accent.Lighten(0.1f) : accent;

    // 轨道
    float trackY = bounds_.y + bounds_.h * 0.5f - kSliderTrackHeight * s * 0.5f;
    RectF trackRect(bounds_.x, trackY, bounds_.w, kSliderTrackHeight * s);
    renderer.FillRoundedRect(trackRect, kSliderTrackHeight * s * 0.5f, trackBg);

    // 已完成部分
    float fillW = bounds_.w * value_;
    if (fillW > 0.0f) {
        RectF fillRect(bounds_.x, trackY, fillW, kSliderTrackHeight * s);
        renderer.FillRoundedRect(fillRect, kSliderTrackHeight * s * 0.5f, accent);
    }

    // 滑块
    float thumbX = bounds_.x + bounds_.w * value_ - kSliderThumbSize * s * 0.5f;
    float thumbY = bounds_.y + bounds_.h * 0.5f - kSliderThumbSize * s * 0.5f;
    RectF thumbRect(thumbX, thumbY, kSliderThumbSize * s, kSliderThumbSize * s);
    renderer.FillEllipse(thumbRect, thumbColor);

    // 滑块描边
    renderer.StrokeEllipse(thumbRect, 1.0f * s,
                            theme.lightMode ? Color(1,1,1,1.0f) : Color(0,0,0,1.0f));
}

} // namespace ModernDesign