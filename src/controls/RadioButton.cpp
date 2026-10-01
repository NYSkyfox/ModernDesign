#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/RadioButton.h"

namespace ModernDesign {

static constexpr float kRadioGlyphSize = 20.0f;
static constexpr float kRadioCheckSize = 10.0f;
static constexpr float kRadioFontSize = 14.0f;
static constexpr float kRadioPad = 8.0f;   // glyph 与文字间距 (CSS gap 8px)

void RadioButton::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void RadioButton::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
}

void RadioButton::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) pressed_ = true;
}

void RadioButton::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    bool wasPressed = pressed_;
    pressed_ = false;
    if (wasPressed && bounds_.Contains(x, y) && !selected_) {
        selected_ = true;
        if (onSelected_) onSelected_();
    }
}

bool RadioButton::Update(float dt) {
    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);

    float pressTarget = (pressed_ && enabled_) ? 1.0f : 0.0f;
    pressedT_ = Approach(pressedT_, pressTarget, dt, 30.0f);

    float checkTarget = (selected_ && enabled_) ? 1.0f : 0.0f;
    checkT_ = Approach(checkT_, checkTarget, dt, 20.0f);

    return std::abs(hoverT_ - hoverTarget) > 0.005f
        || std::abs(pressedT_ - pressTarget) > 0.005f
        || std::abs(checkT_ - checkTarget) > 0.005f;
}

void RadioButton::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    const bool isOn       = selected_;
    const bool disabled   = !enabled_;
    const bool hovering   = hot_ && !disabled;
    const bool pressing   = pressed_ && !disabled;

    Color accent = theme.Accent();

    // ===== WinUIonWeb 精确规格 (RadioButton.vue + theme.css) =====
    // glyph 20x20 圆, border 1px, 内嵌 10x10 圆点 (scale 0→1)
    // 浅色:  --radio-border rgba(0,0,0,0.45)  --text-primary rgba(0,0,0,0.89)
    //        --text-secondary rgba(0,0,0,0.62)
    //        --subtle-secondary rgba(0,0,0,0.0373)  --subtle-tertiary rgba(0,0,0,0.0241)
    //        --accent-text #FFFFFF  --ctrl-strong-stroke-disabled rgba(0,0,0,0.22)
    //        --text-disabled rgba(0,0,0,0.36)
    // 深色:  --radio-border rgba(255,255,255,0.54)  --text-primary #FFFFFF
    //        --text-secondary rgba(255,255,255,0.77)
    //        --subtle-secondary rgba(255,255,255,0.0605)  --subtle-tertiary rgba(255,255,255,0.0419)
    //        --accent-text #000000  --ctrl-strong-stroke-disabled rgba(255,255,255,0.16)
    //        --text-disabled rgba(255,255,255,0.36)
    // accent-hover = accent 0.90 alpha, accent-pressed = accent 0.80 alpha

    bool light = theme.lightMode;
    Color disBorder   = light ? Color(0, 0, 0, 0.22f) : Color(1, 1, 1, 0.16f);
    Color textCol     = disabled ? (light ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.36f))
                                 : theme.TextPrimary();
    Color accentText  = light ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);

    Color glyphBorder, glyphBg;
    if (disabled) {
        glyphBorder = disBorder;
        glyphBg     = isOn ? disBorder : Color(1, 1, 1, 0);  // 选中禁用→填充, 未选禁用→透明
    } else if (isOn) {
        glyphBorder = pressing ? Color(accent.r, accent.g, accent.b, 0.80f)
                               : (hovering ? Color(accent.r, accent.g, accent.b, 0.90f) : accent);
        glyphBg     = glyphBorder;
    } else if (pressing) {
        glyphBorder = light ? Color(0, 0, 0, 0.62f) : Color(1, 1, 1, 0.77f);  // --text-secondary
        glyphBg     = light ? Color(0, 0, 0, 0.0241f) : Color(1, 1, 1, 0.0419f);  // --subtle-tertiary
    } else if (hovering) {
        glyphBorder = theme.TextPrimary();
        glyphBg     = light ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f);  // --subtle-secondary
    } else {
        glyphBorder = light ? Color(0, 0, 0, 0.45f) : Color(1, 1, 1, 0.54f);  // --radio-border
        glyphBg     = Color(1, 1, 1, 0);  // transparent
    }

    // ---- 绘制 glyph（20x20 圆）----
    float glyphSize = kRadioGlyphSize * s;
    float gx = bounds_.x;
    float gy = bounds_.y + (bounds_.h - glyphSize) * 0.5f;
    RectF glyphRect(gx, gy, glyphSize, glyphSize);

    if (glyphBg.a > 0.001f) {
        renderer.FillEllipse(glyphRect, glyphBg);
    }
    renderer.StrokeEllipse(glyphRect, 1.0f * s, glyphBorder);

    // ---- 内嵌圆点（10x10, scale 0→1, hover 1.2, pressed 0.8）----
    float checkScale = EaseOut(checkT_);
    if (isOn && hovering)   checkScale *= 1.2f;
    if (isOn && pressing)   checkScale *= 0.8f;
    if (checkScale > 0.001f) {
        float d = kRadioCheckSize * s * checkScale;
        float cx = gx + glyphSize * 0.5f;
        float cy = gy + glyphSize * 0.5f;
        renderer.FillEllipse(RectF(cx - d * 0.5f, cy - d * 0.5f, d, d), accentText);
    }

    // ---- 文字 ----
    RectF textRect(bounds_.x + glyphSize + kRadioPad * s,
                   bounds_.y,
                   bounds_.w - (glyphSize + kRadioPad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kRadioFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign