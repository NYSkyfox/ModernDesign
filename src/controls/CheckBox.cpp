#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/CheckBox.h"

namespace ModernDesign {

static constexpr float kCheckSize = 18.0f;
static constexpr float kCheckRadius = 2.0f;
static constexpr float kCheckFontSize = 14.0f;
static constexpr float kCheckPad = 8.0f;

void CheckBox::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void CheckBox::OnMouseLeave() { hot_ = false; }

void CheckBox::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        checked_ = !checked_;
        if (checkCallback_) checkCallback_(checked_);
    }
}

void CheckBox::OnMouseUp(float x, float y) {
    (void)x; (void)y;
}

bool CheckBox::Update(float dt) {
    float target = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, target, dt, 18.0f);
    animating_ = std::abs(hoverT_ - target) > 0.005f;
    return animating_;
}

void CheckBox::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();
    Color textCol = theme.TextPrimary();
    if (!enabled_) textCol = theme.lightMode ? Color(0,0,0,0.36f) : Color(1,1,1,0.36f);

    // 复选框区域（左上）
    RectF boxRect(bounds_.x, bounds_.y + bounds_.h * 0.5f - kCheckSize * s * 0.5f,
                  kCheckSize * s, kCheckSize * s);

    // 绘制复选框
    Color boxBg = accent;
    Color boxStroke = accent;
    if (!checked_) {
        boxBg = Color(0, 0, 0, 0.0f);
        boxStroke = theme.lightMode ? Color(0, 0, 0, 0.54f) : Color(1, 1, 1, 0.54f);
    }

    // 复选框背景
    renderer.FillRoundedRect(boxRect, kCheckRadius * s, boxBg);
    // 复选框边框
    renderer.StrokeRoundedRect(boxRect, kCheckRadius * s, 1.0f * s, boxStroke);

    // 勾选标记（如果选中）
    if (checked_) {
        // 勾号
        float margin = kCheckSize * s * 0.2f;
        float checkSize = kCheckSize * s * 0.6f;
        // 简单绘制：两条线
        Color checkColor = checked_ ? Color(1, 1, 1, 1.0f) : boxStroke;
        renderer.DrawLine(
            boxRect.x + margin,
            boxRect.y + boxRect.h * 0.5f,
            boxRect.x + boxRect.h * 0.35f,
            boxRect.y + boxRect.h * 0.75f,
            2.0f * s, checkColor);
        renderer.DrawLine(
            boxRect.x + boxRect.h * 0.35f,
            boxRect.y + boxRect.h * 0.75f,
            boxRect.x + boxRect.w * 0.8f,
            boxRect.y + margin,
            2.0f * s, checkColor);
    }

    // 文字：单行垂直居中 + 左对齐
    RectF textRect(
        bounds_.x + kCheckSize * s + kCheckPad * s,
        bounds_.y,
        bounds_.w - (kCheckSize * s + kCheckPad * s),
        bounds_.h);

    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kCheckFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign