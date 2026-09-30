#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ToggleSwitch.h"

namespace ModernDesign {

static constexpr float kToggleHeight = 20.0f;
static constexpr float kToggleWidth = 40.0f;
static constexpr float kToggleThumbSize = 16.0f;
static constexpr float kToggleFontSize = 14.0f;
static constexpr float kTogglePad = 12.0f;

void ToggleSwitch::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void ToggleSwitch::OnMouseLeave() { hot_ = false; }

void ToggleSwitch::OnMouseDown(float x, float y) {
    (void)x; (void)y;
}

void ToggleSwitch::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        isOn_ = !isOn_;
        if (onChanged_) onChanged_(isOn_);
    }
}

bool ToggleSwitch::Update(float dt) {
    float target = (isOn_ && enabled_) ? 1.0f : 0.0f;
    toggleT_ = Approach(toggleT_, target, dt, 20.0f);
    return std::abs(toggleT_ - target) > 0.005f;
}

void ToggleSwitch::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();
    Color trackBg = theme.lightMode ? Color(0,0,0,0.12f) : Color(1,1,1,0.24f);
    Color textCol = theme.TextPrimary();
    if (!enabled_) textCol = theme.lightMode ? Color(0,0,0,0.36f) : Color(1,1,1,0.36f);

    // Track（右侧）
    float toggleY = bounds_.y + bounds_.h * 0.5f - kToggleHeight * s * 0.5f;
    RectF trackRect(bounds_.x + bounds_.w - kToggleWidth * s - kTogglePad * s,
                    toggleY,
                    kToggleWidth * s,
                    kToggleHeight * s);

    float radius = kToggleHeight * s * 0.5f;
    renderer.FillRoundedRect(trackRect, radius, trackBg);

    // Thumb（滑钮）
    float thumbSize = kToggleThumbSize * s;
    float offset = (kToggleWidth - kToggleThumbSize) * s * toggleT_;
    RectF thumbRect(trackRect.x + offset,
                    trackRect.y + (trackRect.h - thumbSize) * 0.5f,
                    thumbSize, thumbSize);

    Color thumbColor = isOn_ ? accent : (theme.lightMode ? Color(0,0,0,0.54f) : Color(1,1,1,0.54f));
    renderer.FillRoundedRect(thumbRect, thumbSize * 0.5f, thumbColor);

    // 文字（左侧）
    RectF textRect(bounds_.x, bounds_.y,
                   bounds_.w - (kToggleWidth * s + kTogglePad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kToggleFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign