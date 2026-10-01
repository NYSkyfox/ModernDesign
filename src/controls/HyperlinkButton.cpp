#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/HyperlinkButton.h"

namespace ModernDesign {
namespace Controls {

namespace {
constexpr const wchar_t* kFace = L"Segoe UI";
}

void HyperlinkButton::SetEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled_) { hot_ = false; pressed_ = false; }
}

float HyperlinkButton::MeasureWidth(Renderer& renderer, float scale) const {
    if (text_.empty()) return 2.0f * kPadX * scale;
    float tw = 0.0f, th = 0.0f;
    renderer.MeasureText(text_, 100000.0f, kFace, kFontSize * scale,
                         DWRITE_FONT_WEIGHT_NORMAL, &tw, &th);
    return tw + 2.0f * kPadX * scale;
}

void HyperlinkButton::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void HyperlinkButton::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
}

void HyperlinkButton::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) pressed_ = true;
}

void HyperlinkButton::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    const bool wasPressed = pressed_;
    pressed_ = false;
    if (wasPressed && bounds_.Contains(x, y) && onClick_) onClick_();
}

bool HyperlinkButton::Update(float dt) {
    bool animating = false;
    const float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);
    if (std::abs(hoverT_ - hoverTarget) > 0.005f) animating = true;

    const float pressTarget = (pressed_ && enabled_) ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);
    if (std::abs(pressT_ - pressTarget) > 0.005f) animating = true;

    if (revealT_ < 1.0f) {
        revealT_ = Clamp01(revealT_ + dt * 4.0f);
        animating = true;
    }
    return animating;
}

void HyperlinkButton::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    const float s = scale;
    const bool disabled = !enabled_;
    const bool pressing = pressed_ && !disabled;

    // ===== WinUIonWeb HyperlinkButton.vue 规格 =====
    // 文字恒为 accent（hover/pressed 不变色）；背景 hover→subtle-secondary、pressed→subtle-tertiary
    // 派生值与 Button.cpp 的 subtle-* 令牌一致（同一套 rgba）
    const Color accent      = theme.Accent();
    const Color accentDis   = theme.TextDisabled();
    const Color subtleHover = (theme.lightMode ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f));
    const Color subtlePress = (theme.lightMode ? Color(0, 0, 0, 0.0241f) : Color(1, 1, 1, 0.0419f));

    const float reveal = revealT_;

    // 背景（pressed 叠加在 hover 之上）
    const float bgT = pressing ? FzMx(pressT_, hoverT_) : hoverT_;
    if (bgT > 0.001f && !disabled) {
        const Color bg = pressing
            ? Color::Lerp(subtleHover, subtlePress, pressT_)
            : subtleHover;
        renderer.FillRoundedRect(bounds_, kRadius * s, bg.WithAlpha(bg.a * bgT * reveal));
    }

    // 文字（accent 色；hover/pressed 保持 accent；disabled → accent-disabled）
    const Color fg = disabled ? accentDis : accent;
    if (!text_.empty()) {
        const float fs = kFontSize * s;
        const RectF textRect(bounds_.x + kPadX * s, bounds_.y,
                             FzMx(0.0f, bounds_.w - 2.0f * kPadX * s), bounds_.h);
        renderer.DrawText(text_, textRect.x, textRect.y, textRect.w, textRect.h,
                          kFace, fs, DWRITE_FONT_WEIGHT_NORMAL,
                          fg.WithAlpha(reveal),
                          DWRITE_TEXT_ALIGNMENT_NEAR,
                          DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
}

}} // namespace ModernDesign::Controls