#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/CheckBox.h"

namespace ModernDesign {

static constexpr float kCheckSize = 20.0f;   // WinUIonWeb .checkbox-box 20x20
static constexpr float kCheckRadius = 4.0f;  // border-radius 4px
static constexpr float kCheckFontSize = 14.0f;
static constexpr float kCheckPad = 8.0f;     // gap 8px

void CheckBox::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void CheckBox::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
}

void CheckBox::OnMouseDown(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) pressed_ = true;
}

void CheckBox::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    bool wasPressed = pressed_;
    pressed_ = false;
    if (!wasPressed || !bounds_.Contains(x, y)) return;

    if (triState_) {
        // 三态循环：unchecked -> checked -> indeterminate -> unchecked
        if (!checked_ && !indeterminate_) {
            checked_ = true;
            if (checkCallback_) checkCallback_(true);
        } else if (checked_) {
            checked_ = false;
            indeterminate_ = true;
            if (checkCallback_) checkCallback_(false);
        } else {
            indeterminate_ = false;
            if (checkCallback_) checkCallback_(false);
        }
    } else {
        checked_ = !checked_;
        if (checkCallback_) checkCallback_(checked_);
    }
}

bool CheckBox::Update(float dt) {
    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);

    float pressTarget = (pressed_ && enabled_) ? 1.0f : 0.0f;
    pressedT_ = Approach(pressedT_, pressTarget, dt, 30.0f);

    float checkTarget = ((checked_ || indeterminate_) && enabled_) ? 1.0f : 0.0f;
    checkedT_ = Approach(checkedT_, checkTarget, dt, 20.0f);

    animating_ = std::abs(hoverT_ - hoverTarget) > 0.005f
        || std::abs(pressedT_ - pressTarget) > 0.005f
        || std::abs(checkedT_ - checkTarget) > 0.005f;
    return animating_;
}

void CheckBox::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    const bool isOn       = checked_ || indeterminate_;   // 选中/不确定都用 accent 底
    const bool disabled   = !enabled_;
    const bool hovering   = hot_ && !disabled;
    const bool pressing   = pressed_ && !disabled;

    Color accent = theme.Accent();

    // ===== WinUIonWeb 精确规格 (CheckBox.vue + theme.css) =====
    // box 20x20, radius 4, border 1px, glyph accent-text (check/indeterminate)
    // unchecked 框:  fill --ctrl-fill-{default,secondary,tertiary}, stroke --ctrl-strong-stroke
    // checked/indeterminate 框: fill+stroke --accent-{base,hover,pressed}
    // disabled: 底 --ctrl-fill-disabled（未选）/ --accent-fill-disabled（选中），边框 --ctrl-strong-stroke-disabled
    // 文字: --text-primary / --text-disabled(36%)
    bool light = theme.lightMode;

    Color fillDefault = light ? Color(1, 1, 1, 0.70f) : Color(1, 1, 1, 0.0605f);
    Color fillHover   = light ? Color(0.976f, 0.976f, 0.976f, 0.50f) : Color(1, 1, 1, 0.0837f);
    Color fillPressed = light ? Color(0.976f, 0.976f, 0.976f, 0.30f) : Color(1, 1, 1, 0.0326f);
    Color fillDis     = light ? Color(0.976f, 0.976f, 0.976f, 0.30f) : Color(1, 1, 1, 0.0419f);
    Color strongStroke = light ? Color(0, 0, 0, 0.45f) : Color(1, 1, 1, 0.54f);      // unchecked 边框
    Color strongStrokeDis = light ? Color(0, 0, 0, 0.22f) : Color(1, 1, 1, 0.16f);   // disabled 边框
    Color accentHover   = accent.WithAlpha(0.90f);
    Color accentPressed = accent.WithAlpha(0.80f);
    Color accentFillDis = light ? Color(0, 0, 0, 0.22f) : Color(1, 1, 1, 0.16f);
    Color accentText    = light ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
    Color accentTextSec = light ? Color(1, 1, 1, 0.70f) : Color(0, 0, 0, 0.50f);
    Color textPrimary   = theme.TextPrimary();
    Color textDis       = light ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.36f);

    Color boxBg, boxStroke, glyphColor;
    if (disabled) {
        boxBg     = isOn ? accentFillDis : fillDis;
        boxStroke = strongStrokeDis;
        glyphColor = textDis;
    } else if (isOn) {
        // checked / indeterminate：accent 底
        boxBg = pressing ? accentPressed : (hovering ? accentHover : accent);
        boxStroke = boxBg;
        glyphColor = pressing ? accentTextSec : accentText;
    } else if (pressing) {
        boxBg = fillPressed;
        boxStroke = strongStroke;   // WinUIonWeb: unchecked:active stroke 用 disabled 值，视觉接近，保留 strong 更清晰
        glyphColor = accentText;
    } else if (hovering) {
        boxBg = fillHover;
        boxStroke = strongStroke;
        glyphColor = accentText;
    } else {
        boxBg = fillDefault;
        boxStroke = strongStroke;
        glyphColor = accentText;
    }

    // ---- 框（20x20 圆角矩形）----
    float size = kCheckSize * s;
    float bx = bounds_.x;
    float by = bounds_.y + (bounds_.h - size) * 0.5f;
    RectF boxRect(bx, by, size, size);
    renderer.FillRoundedRect(boxRect, kCheckRadius * s, boxBg);
    renderer.StrokeRoundedRect(boxRect, kCheckRadius * s, 1.0f * s, boxStroke);

    // ---- glyph（对勾渐进描边 / 横杠淡入）----
    float glyphP = EaseOut(checkedT_);
    if (isOn && glyphP > 0.001f) {
        float cx = bx + size * 0.5f;
        float cy = by + size * 0.5f;
        if (indeterminate_) {
            // 横杠（indeterminate）：淡入
            float hw = size * 0.30f;
            renderer.DrawLine(cx - hw, cy, cx + hw, cy, 2.0f * s,
                              glyphColor.WithAlpha(glyphP));
        } else {
            // 对勾：两段线按进度"生长"（先短腿后长腿，模拟 WinUI AnimatedIcon 描边动画）
            const float x1 = cx - size * 0.28f, y1 = cy + size * 0.02f;
            const float xm = cx - size * 0.08f, ym = cy + size * 0.20f;
            const float x2 = cx + size * 0.28f, y2 = cy - size * 0.18f;
            float p1 = FzMn(1.0f, glyphP * 2.0f);                 // 段1：前半
            float p2 = FzMn(1.0f, FzMx(0.0f, glyphP * 2.0f - 1.0f)); // 段2：后半
            if (p1 > 0.001f)
                renderer.DrawLine(x1, y1, LerpF(x1, xm, p1), LerpF(y1, ym, p1), 2.0f * s, glyphColor);
            if (p2 > 0.001f)
                renderer.DrawLine(xm, ym, LerpF(xm, x2, p2), LerpF(ym, y2, p2), 2.0f * s, glyphColor);
        }
    }

    // ---- 文字 ----
    Color textCol = disabled ? textDis : textPrimary;
    RectF textRect(bounds_.x + size + kCheckPad * s,
                   bounds_.y,
                   bounds_.w - (size + kCheckPad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kCheckFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign