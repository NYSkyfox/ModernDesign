#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ToggleSwitch.h"

namespace ModernDesign {

static constexpr float kToggleFontSize = 14.0f;
static constexpr float kTogglePad = 12.0f;

void ToggleSwitch::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void ToggleSwitch::OnMouseLeave() { hot_ = false; }

void ToggleSwitch::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) pressed_ = true;
}

void ToggleSwitch::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    bool wasPressed = pressed_;
    pressed_ = false;
    if (wasPressed && bounds_.Contains(x, y)) {
        isOn_ = !isOn_;
        if (onChanged_) onChanged_(isOn_);
    }
}

bool ToggleSwitch::Update(float dt) {
    float toggleTarget = (isOn_ && enabled_) ? 1.0f : 0.0f;
    toggleT_ = Approach(toggleT_, toggleTarget, dt, 20.0f);

    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);

    float pressTarget = (pressed_ && enabled_) ? 1.0f : 0.0f;
    pressedT_ = Approach(pressedT_, pressTarget, dt, 30.0f);

    return std::abs(toggleT_ - toggleTarget) > 0.005f
        || std::abs(hoverT_ - hoverTarget) > 0.005f
        || std::abs(pressedT_ - pressTarget) > 0.005f;
}

void ToggleSwitch::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    float hT = EaseOut(hoverT_);
    float tT = EaseOut(toggleT_);
    float pT = EaseOut(pressedT_);

    const bool isOn       = isOn_;
    const bool disabled   = !enabled_;
    const bool hovering   = hot_ && !disabled;
    const bool pressing   = pressed_ && !disabled;

    // ===== WinUIonWeb 精确规格 =====
    // Track: 40×20, border-radius 10 (= trackH/2)
    // Knob container: 20×20, translateX 0→20
    // Thumb: 12×12 圆, hover 14×14, pressed 17×14 圆角矩形
    const float trackW = 40.0f * s, trackH = 20.0f * s;
    float tx = bounds_.x + bounds_.w - trackW - kTogglePad * s;
    float ty = bounds_.y + (bounds_.h - trackH) * 0.5f;
    float radius = trackH * 0.5f;

    Color accent = theme.Accent();

    // ---- WinUIonWeb theme.css 精确色值 ----
    // 浅色模式
    //   --subtle-secondary:  rgba(0,0,0,0.0373)
    //   --subtle-tertiary:   rgba(0,0,0,0.0241)
    //   --subtle-pressed:    rgba(0,0,0,0.06)
    //   --toggle-border:     rgba(0,0,0,0.45)
    //   --text-primary:      rgba(0,0,0,0.89)
    //   --toggle-thumb:      rgba(0,0,0,0.61)
    //   --toggle-thumb-hover:rgba(0,0,0,0.89)
    //   --toggle-on-thumb:   #FFFFFF
    //   --ctrl-strong-stroke-disabled: rgba(0,0,0,0.22)
    //   --accent-fill-disabled:        rgba(0,0,0,0.22)
    //   --text-disabled:               rgba(0,0,0,0.36)
    //   --accent-hover:  rgba(accent, 0.90)
    //   --accent-pressed:rgba(accent, 0.80)
    // 深色模式
    //   --subtle-secondary:  rgba(255,255,255,0.0605)
    //   --subtle-tertiary:   rgba(255,255,255,0.0419)
    //   --subtle-pressed:    rgba(255,255,255,0.03)
    //   --toggle-border:     rgba(255,255,255,0.54)
    //   --text-primary:      #FFFFFF
    //   --toggle-thumb:      rgba(255,255,255,0.79)
    //   --toggle-thumb-hover:#FFFFFF
    //   --toggle-on-thumb:   #000000
    //   --ctrl-strong-stroke-disabled: rgba(255,255,255,0.16)
    //   --accent-fill-disabled:        rgba(255,255,255,0.16)
    //   --text-disabled:               rgba(255,255,255,0.36)

    Color offBg, offBorder, offBgHover, offBorderHover, offBgPressed;
    Color onBgHover, onBgPressed;
    Color disBorder, disOffBg, disOnBg;
    Color knobOff, knobHover, knobOn;
    Color disKnob, disKnobOn;

    if (theme.lightMode) {
        offBg          = Color(0, 0, 0, 0.0373f);
        offBorder      = Color(0, 0, 0, 0.45f);
        offBgHover     = Color(0, 0, 0, 0.0241f);
        offBorderHover = Color(0, 0, 0, 0.89f);
        offBgPressed   = Color(0, 0, 0, 0.06f);
        onBgHover      = Color(accent.r, accent.g, accent.b, 0.90f);
        onBgPressed    = Color(accent.r, accent.g, accent.b, 0.80f);
        disBorder      = Color(0, 0, 0, 0.22f);
        disOffBg       = Color(1, 1, 1, 0);   // transparent
        disOnBg        = Color(0, 0, 0, 0.22f);
        knobOff        = Color(0, 0, 0, 0.61f);
        knobHover      = Color(0, 0, 0, 0.89f);
        knobOn         = Color(1, 1, 1, 1);
        disKnob        = Color(0, 0, 0, 0.22f);
        disKnobOn      = Color(0, 0, 0, 0.36f);
    } else {
        offBg          = Color(1, 1, 1, 0.0605f);
        offBorder      = Color(1, 1, 1, 0.54f);
        offBgHover     = Color(1, 1, 1, 0.0419f);
        offBorderHover = Color(1, 1, 1, 1);
        offBgPressed   = Color(1, 1, 1, 0.03f);
        onBgHover      = Color(accent.r, accent.g, accent.b, 0.90f);
        onBgPressed    = Color(accent.r, accent.g, accent.b, 0.80f);
        disBorder      = Color(1, 1, 1, 0.16f);
        disOffBg       = Color(1, 1, 1, 0);   // transparent
        disOnBg        = Color(1, 1, 1, 0.16f);
        knobOff        = Color(1, 1, 1, 0.79f);
        knobHover      = Color(1, 1, 1, 1);
        knobOn         = Color(0, 0, 0, 1);
        disKnob        = Color(1, 1, 1, 0.16f);
        disKnobOn      = Color(1, 1, 1, 0.36f);
    }

    // ---- 选轨道色 ----
    Color trackBg, trackBorder;
    if (disabled) {
        trackBg     = isOn ? disOnBg : disOffBg;
        trackBorder = disBorder;
    } else if (isOn) {
        // ON: accent 底, 边框 transparent
        trackBg     = pressing ? onBgPressed : (hovering ? onBgHover : accent);
        trackBorder = Color(1, 1, 1, 0);   // transparent
    } else if (pressing) {
        trackBg     = offBgPressed;
        trackBorder = offBorder;
    } else if (hovering) {
        trackBg     = offBgHover;
        trackBorder = offBorderHover;
    } else {
        trackBg     = offBg;
        trackBorder = offBorder;
    }

    // 画轨道
    RectF trackRect(tx, ty, trackW, trackH);
    renderer.FillRoundedRect(trackRect, radius, trackBg);
    if (trackBorder.a > 0.001f) {
        renderer.StrokeRoundedRect(trackRect, radius, 1.0f, trackBorder);
    }

    // ---- 滑钮（knob container 20×20, 内嵌 thumb 圆）----
    const float knobSize = 20.0f * s;
    float knobX = tx + tT * (trackW - knobSize);   // 0 → 20px
    float knobY = ty;

    // thumb 尺寸：12 → 14(hover) → 17×14(pressed)
    float thumbW, thumbH;
    Color thumbColor;
    if (disabled) {
        thumbW = thumbH = 12.0f * s;
        thumbColor = isOn ? disKnobOn : disKnob;
    } else if (pressing) {
        thumbW = 17.0f * s;
        thumbH = 14.0f * s;
        thumbColor = isOn ? knobOn : knobHover;
    } else if (hovering) {
        thumbW = thumbH = 14.0f * s;
        thumbColor = isOn ? knobOn : knobHover;
    } else {
        thumbW = thumbH = 12.0f * s;
        thumbColor = isOn ? knobOn : knobOff;
    }

    // thumb 在 knob 内居中
    float thumbX = knobX + (knobSize - thumbW) * 0.5f;
    float thumbY = knobY + (knobSize - thumbH) * 0.5f;
    // pressed 偏移：OFF → +1.5px, ON → -1.5px（WinUIonWeb CSS）
    if (pressing && !disabled) {
        thumbX += (isOn ? -1.5f : 1.5f) * s;
    }

    RectF thumbRect(thumbX, thumbY, thumbW, thumbH);

    if (pressing && !disabled) {
        // pressed: 17×14 圆角矩形 (border-radius 7)
        renderer.FillRoundedRect(thumbRect, thumbH * 0.5f, thumbColor);
    } else {
        // 正圆
        float d = FzMn(thumbW, thumbH);
        float cx = thumbX + thumbW * 0.5f;
        float cy = thumbY + thumbH * 0.5f;
        renderer.FillEllipse(RectF(cx - d * 0.5f, cy - d * 0.5f, d, d), thumbColor);
    }

    // ---- 文字标签 ----
    Color textCol = theme.TextPrimary();
    if (disabled) textCol = theme.lightMode ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.36f);
    RectF textRect(bounds_.x, bounds_.y,
                   bounds_.w - (trackW + kTogglePad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kToggleFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign