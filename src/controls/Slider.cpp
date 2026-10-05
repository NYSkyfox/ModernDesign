#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Slider.h"

namespace ModernDesign {

static constexpr float kSliderTrackHeight = 4.0f;  // track 4px
static constexpr float kSliderThumbSize = 24.0f;   // thumb 24px (WinUI SliderHorizontalThumbWidth)
static constexpr float kSliderDotSize = 12.0f;     // 内嵌 accent 点 12px

void Slider::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
    if (dragging_) {
        float t = Clamp01((x - bounds_.x) / FzMx(1.0f, bounds_.w));
        value_ = t;
        if (onValue_) onValue_(value_);
    }
}

void Slider::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        dragging_ = true;
        float t = Clamp01((x - bounds_.x) / FzMx(1.0f, bounds_.w));
        value_ = t;
        if (onValue_) onValue_(value_);
    }
}

void Slider::OnMouseUp(float x, float y) {
    (void)x; (void)y;
    dragging_ = false;
}

void Slider::OnMouseLeave() {
    hot_ = false;
    dragging_ = false;
}

bool Slider::Update(float dt) {
    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);
    float pressTarget = (dragging_ && enabled_) ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);
    return std::abs(hoverT_ - hoverTarget) > 0.005f
        || std::abs(pressT_ - pressTarget) > 0.005f;
}

void Slider::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();

    // ===== WinUIonWeb 精确规格 (Slider.vue + theme.css) =====
    // track 4px radius 2, bg --ctrl-strong-fill(45%黑/54%白), fill accent-base
    // thumb 22px 圆, 底 #fff(浅)/#454545(深), 内嵌 12px accent 点
    // 内点: default scale 0.86 / hover accent-hover scale 1.167 / pressed accent-pressed scale 0.71
    // disabled: 整体 opacity 0.6
    const bool disabled = !enabled_;
    const bool hovering = hot_ && !disabled;
    const bool pressing = dragging_ && !disabled;

    float trackY = bounds_.y + bounds_.h * 0.5f - kSliderTrackHeight * s * 0.5f;
    float trackH = kSliderTrackHeight * s;

    Color trackBg = theme.lightMode ? Color(0, 0, 0, 0.45f) : Color(1, 1, 1, 0.54f);  // --ctrl-strong-fill
    Color trackFill = accent;

    // 轨道底
    RectF trackRect(bounds_.x, trackY, bounds_.w, trackH);
    renderer.FillRoundedRect(trackRect, trackH * 0.5f, trackBg);

    // thumb 中心位置（在轨道上移动，边缘留 thumbR 避免越界）
    float thumbR = kSliderThumbSize * s * 0.5f;
    float travel = FzMx(1.0f, bounds_.w - kSliderThumbSize * s);
    float thumbCX = bounds_.x + thumbR + value_ * travel;
    float thumbCY = bounds_.y + bounds_.h * 0.5f;

    // fill（已完成部分，到 thumb 中心）
    float fillW = thumbCX - bounds_.x;
    if (fillW > 0.0f) {
        RectF fillRect(bounds_.x, trackY, fillW, trackH);
        renderer.FillRoundedRect(fillRect, trackH * 0.5f, trackFill);
    }

    // 外圈 thumb（22px 圆）
    RectF thumbRect(thumbCX - thumbR, thumbCY - thumbR, kSliderThumbSize * s, kSliderThumbSize * s);
    Color thumbBg = theme.lightMode ? Color(1, 1, 1, 1) : Color(0.271f, 0.271f, 0.271f, 1);  // #454545
    renderer.FillEllipse(thumbRect, thumbBg);
    // thumb 边框（上 ctrl-border，下 ctrl-border-accent；用平均色近似）
    Color tbTop  = theme.lightMode ? Color(0, 0, 0, 0.06f) : Color(1, 1, 1, 0.0706f);
    Color tbBot  = theme.lightMode ? Color(0, 0, 0, 0.16f) : Color(1, 1, 1, 0.0941f);
    renderer.StrokeEllipse(thumbRect, 1.0f * s, Color::Lerp(tbTop, tbBot, 0.5f));

    // 内嵌 accent 点（12px，hover/pressed 缩放变色）
    float dotScale;
    Color dotColor;
    if (pressing) {
        dotScale = LerpF(0.86f, 0.71f, EaseOut(pressT_));
        dotColor = accent.WithAlpha(0.80f);   // accent-pressed
    } else if (hovering) {
        dotScale = LerpF(0.86f, 1.167f, EaseOut(hoverT_));
        dotColor = accent.WithAlpha(0.90f);   // accent-hover
    } else {
        dotScale = 0.86f;
        dotColor = accent;
    }
    float dotD = kSliderDotSize * s * dotScale;
    RectF dotRect(thumbCX - dotD * 0.5f, thumbCY - dotD * 0.5f, dotD, dotD);
    renderer.FillEllipse(dotRect, dotColor);

    // disabled：整体压暗（近似 opacity 0.6，重绘一层窗口色半透明覆盖）
    if (disabled) {
        Color veil = theme.WindowBg().WithAlpha(0.4f);
        renderer.FillRect(RectF(bounds_.x, bounds_.y, bounds_.w, bounds_.h), veil);
    }
}

} // namespace ModernDesign