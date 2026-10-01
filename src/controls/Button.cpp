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
Button::~Button() = default;

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

    const bool disabled = !enabled_;
    const bool hovering = hot_ && !disabled;
    const bool pressing = pressed_ && !disabled;

    // ===== WinUIonWeb 精确规格 (Button.vue + theme.css) =====
    // min-height 32, font 14, corner 4, border 1px
    // 状态优先级: disabled > pressed > hover > default
    // Standard: bg --ctrl-fill-{default,tertiary,secondary}；border 上 --ctrl-border、下 --ctrl-border-accent
    // Accent:   bg --accent-{base,pressed,hover}；fg --accent-text；pressed/disabled 边框透明
    // Subtle:   bg --subtle-{transparent,tertiary,secondary}；无描边
    Color accent = theme.Accent();
    bool light = theme.lightMode;

    // accent 派生（跟 accent 色值走）
    Color accentHover   = accent.WithAlpha(0.90f);
    Color accentPressed = accent.WithAlpha(0.80f);
    Color accentText    = light ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
    Color accentTextSec = light ? Color(1, 1, 1, 0.70f) : Color(0, 0, 0, 0.50f);
    Color accentFillDis = light ? Color(0, 0, 0, 0.22f) : Color(1, 1, 1, 0.16f);
    Color accentBorder  = light ? Color(1, 1, 1, 0.08f) : Color(0, 0, 0, 0.1373f);

    // 标准 / 中性色（逐条对应 theme.css 的 rgba）
    Color fillDefault = light ? Color(1, 1, 1, 0.70f)        : Color(1, 1, 1, 0.0605f);
    Color fillHover   = light ? Color(0.976f, 0.976f, 0.976f, 0.50f) : Color(1, 1, 1, 0.0837f);
    Color fillPressed = light ? Color(0.976f, 0.976f, 0.976f, 0.30f) : Color(1, 1, 1, 0.0326f);
    Color fillDis     = light ? Color(0.976f, 0.976f, 0.976f, 0.30f) : Color(1, 1, 1, 0.0419f);
    Color subtleHover = light ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f);
    Color subtlePress = light ? Color(0, 0, 0, 0.0241f) : Color(1, 1, 1, 0.0419f);
    Color borderTop   = light ? Color(0, 0, 0, 0.06f) : Color(1, 1, 1, 0.0706f);   // --ctrl-border
    Color borderBottom = light ? Color(0, 0, 0, 0.16f) : Color(1, 1, 1, 0.0941f);  // --ctrl-border-accent
    Color textPrimary = theme.TextPrimary();
    Color textSecond  = light ? Color(0, 0, 0, 0.62f) : Color(1, 1, 1, 0.77f);
    Color textDis     = light ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.36f);
    Color transparent = Color(1, 1, 1, 0);

    Color bg, border, fg;
    bool isAccent = (variant_ == ButtonVariant::Accent);

    if (isAccent) {
        if (disabled)       { bg = accentFillDis; border = transparent;      fg = textDis; }
        else if (pressing)  { bg = accentPressed; border = transparent;      fg = accentTextSec; }
        else if (hovering)  { bg = accentHover;   border = accentBorder;     fg = accentText; }
        else                { bg = accent;        border = accentBorder;     fg = accentText; }
    } else if (variant_ == ButtonVariant::Subtle) {
        // 无描边
        border = transparent;
        if (disabled)       { bg = transparent;  fg = textDis; }
        else if (pressing)  { bg = subtlePress;  fg = textSecond; }
        else if (hovering)  { bg = subtleHover;  fg = textPrimary; }
        else                { bg = transparent;  fg = textPrimary; }
    } else {
        // Standard
        border = Color::Lerp(borderTop, borderBottom, 0.5f);
        if (disabled)       { bg = fillDis;     border = transparent; fg = textDis; }
        else if (pressing)  { bg = fillPressed; border = border;       fg = textSecond; }
        else if (hovering)  { bg = fillHover;   border = border;       fg = textPrimary; }
        else                { bg = fillDefault; border = border;       fg = textPrimary; }
    }

    // 底色
    if (bg.a > 0.001f) {
        renderer.FillRoundedRect(bounds_, radius, bg);
    }
    // 边框（上/下渐变用平均色近似）
    if (border.a > 0.001f) {
        renderer.StrokeRoundedRect(bounds_, radius, 1.0f * s, border);
    }
    // 文字
    if (!text_.empty()) {
        renderer.DrawTextCentered(text_, bounds_, L"Segoe UI", fontSize,
                                  DWRITE_FONT_WEIGHT_NORMAL, fg);
    }
}

}} // namespace ModernDesign::Controls