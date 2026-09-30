#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Button.h"

namespace ModernDesign {
namespace Controls {

// WinUI 3 规格（参考 FluentZero 验证过的参数）
// 圆角 4px、高度 32px、字号 14px
constexpr float kBtnRadius = 4.0f;
constexpr float kBtnHeight = 32.0f;
constexpr float kBtnFontSize = 14.0f;

Button::Button() = default;

void Button::SetText(const std::wstring& text) { text_ = text; }

void Button::SetVariant(ButtonVariant variant) { variant_ = variant; }

void Button::SetEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled_) {
        hot_ = false;
        pressed_ = false;
    }
}

void Button::SetBounds(const RectF& bounds) { bounds_ = bounds; }

void Button::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    bool inside = bounds_.Contains(x, y);
    if (inside != hot_) {
        hot_ = inside;
    }
}

void Button::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
}

void Button::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        pressed_ = true;
    }
}

void Button::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    bool wasPressed = pressed_;
    pressed_ = false;
    if (wasPressed && bounds_.Contains(x, y) && onClick_) {
        onClick_();
    }
}

bool Button::Update(float dt) {
    bool animating = false;

    // hover 缓入缓出
    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);
    if (std::abs(hoverT_ - hoverTarget) > 0.005f) animating = true;

    // press 缓入
    float pressTarget = (pressed_ && enabled_) ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);
    if (std::abs(pressT_ - pressTarget) > 0.005f) animating = true;

    // reveal（首次出现渐入）
    if (revealT_ < 1.0f) {
        revealT_ = Clamp01(revealT_ + dt * 4.0f);
        animating = true;
    }

    return animating;
}

void Button::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    float radius = kBtnRadius * s;
    float fontSize = kBtnFontSize * s;
    float revealAlpha = 0.6f + 0.4f * revealT_;   // 渐入

    Color bg = theme.ButtonFill();
    Color fg = theme.ButtonText();
    Color border = theme.ButtonBorder();
    bool drawBorder = (variant_ == ButtonVariant::Standard);
    bool drawBg = (variant_ != ButtonVariant::Subtle);

    if (!enabled_) {
        // ---- Disabled ----
        if (drawBg) {
            bg = theme.lightMode ? Color(0.0f, 0.0f, 0.0f, 0.04f)
                                 : Color(1.0f, 1.0f, 1.0f, 0.06f);
        }
        fg = theme.lightMode ? Color(0.0f, 0.0f, 0.0f, 0.36f)
                             : Color(1.0f, 1.0f, 1.0f, 0.36f);
        drawBorder = false;
    } else if (variant_ == ButtonVariant::Accent) {
        // ---- Accent（强调色）----
        bg = theme.Accent();
        if (pressT_ > 0.0f) {
            bg = bg.Darken(0.08f * pressT_);
        } else if (hoverT_ > 0.0f) {
            bg = bg.Lighten(0.04f * hoverT_);
        }
        fg = theme.TextOnAccent();
        drawBorder = false;
    } else if (variant_ == ButtonVariant::Standard) {
        // ---- Standard ----
        if (pressT_ > 0.0f) {
            bg = theme.ButtonFill().Darken(0.08f * pressT_);
        } else if (hoverT_ > 0.0f) {
            bg = theme.ButtonFill().Darken(0.03f * hoverT_);
        }
    } else {
        // ---- Subtle（仅 hover 淡底）----
        if (hot_ || pressed_) {
            float t = FzMx(hoverT_, pressT_);
            bg = theme.lightMode ? Color(0.0f, 0.0f, 0.0f, 0.05f * t)
                                 : Color(1.0f, 1.0f, 1.0f, 0.08f * t);
            drawBg = true;
        } else {
            drawBg = false;
        }
    }

    // 底色
    if (drawBg) {
        renderer.FillRoundedRect(bounds_, radius, bg);
    }

    // 边框
    if (drawBorder) {
        renderer.StrokeRoundedRect(bounds_, radius, 1.0f * s, border);
    }

    // 文字：水平 + 垂直均交给 DirectWrite 对齐属性（不依赖 Measure）
    if (!text_.empty()) {
        renderer.DrawTextCentered(text_, bounds_, L"Segoe UI", fontSize,
                                  DWRITE_FONT_WEIGHT_NORMAL, fg);
    }
}

}} // namespace ModernDesign::Controls