#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/RadioButton.h"

namespace ModernDesign {

static constexpr float kRadioSize = 16.0f;
static constexpr float kRadioFontSize = 14.0f;
static constexpr float kRadioPad = 8.0f;

void RadioButton::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void RadioButton::OnMouseLeave() { hot_ = false; }

void RadioButton::OnMouseDown(float x, float y) {
    (void)x; (void)y;
}

void RadioButton::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y) && !selected_) {
        selected_ = true;
        if (onSelected_) onSelected_();
    }
}

bool RadioButton::Update(float dt) {
    float target = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, target, dt, 18.0f);
    return std::abs(hoverT_ - target) > 0.005f;
}

void RadioButton::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();
    Color textCol = theme.TextPrimary();
    if (!enabled_) textCol = theme.lightMode ? Color(0,0,0,0.36f) : Color(1,1,1,0.36f);

    // 圆形区域
    float radioY = bounds_.y + bounds_.h * 0.5f - kRadioSize * s * 0.5f;
    RectF radioRect(bounds_.x, radioY, kRadioSize * s, kRadioSize * s);

    // 外圈
    Color outer = selected_ ? accent : (theme.lightMode ? Color(0,0,0,0.54f) : Color(1,1,1,0.54f));
    renderer.StrokeEllipse(radioRect, 1.0f * s, outer);

    // 内圆（选中时）
    if (selected_) {
        float inner = kRadioSize * s * 0.4f;
        float ix = bounds_.x + (kRadioSize * s - inner) * 0.5f;
        float iy = radioY + (kRadioSize * s - inner) * 0.5f;
        renderer.FillEllipse(RectF(ix, iy, inner, inner), accent);
    }

    // 文字
    RectF textRect(bounds_.x + kRadioSize * s + kRadioPad * s,
                   bounds_.y,
                   bounds_.w - (kRadioSize * s + kRadioPad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kRadioFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign