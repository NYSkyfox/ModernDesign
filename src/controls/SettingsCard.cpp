#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/SettingsCard.h"
#include "utils/FluentIcons.h"

namespace ModernDesign {

namespace {
constexpr const wchar_t* kFace = L"Segoe UI";
constexpr float kTitleLineH = 20.0f;
constexpr float kDescLineH  = 16.0f;
constexpr float kContentGap = 8.0f;   // .win-settings-card-content gap
constexpr float kActionGap  = 8.0f;   // content 与 action icon 之间

// 右侧内容区宽度的估算（DIP，未缩放）。
// 调用方用 SetContentWidth() 给出子控件固有宽；未设置时按 content 文本宽估算。
} // namespace

// ============================================================
// 输入
// ============================================================
void SettingsCard::OnMouseMove(float x, float y) {
    if (!clickable_) return;
    hot_ = bounds_.Contains(x, y);
}

void SettingsCard::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
}

void SettingsCard::OnMouseDown(float x, float y) {
    if (!clickable_) return;
    if (bounds_.Contains(x, y)) pressed_ = true;
}

void SettingsCard::OnMouseUp(float x, float y) {
    if (!clickable_) return;
    const bool wasPressed = pressed_;
    pressed_ = false;
    if (wasPressed && bounds_.Contains(x, y) && onClick_) onClick_();
}

// ============================================================
// 动画
// ============================================================
bool SettingsCard::Update(float dt) {
    if (!clickable_) return false;
    bool animating = false;
    const float hoverTarget = (hot_ && !pressed_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);
    if (std::abs(hoverT_ - hoverTarget) > 0.005f) animating = true;
    const float pressTarget = (pressed_) ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);
    if (std::abs(pressT_ - pressTarget) > 0.005f) animating = true;
    return animating;
}

// ============================================================
// 布局：右侧 Content 区
// ============================================================
RectF SettingsCard::ContentRect(float scale) const {
    if (bounds_.IsEmpty()) return RectF();
    const float s = scale;
    const float right = bounds_.x + bounds_.w - kPad * s;
    const float h = bounds_.h;
    const float cy = bounds_.y + h * 0.5f;

    // 右侧占用：action icon + content
    float rightBlock = 0.0f;
    if (actionIcon_) rightBlock += kActionFS * s + kActionGap * s;
    float cw = contentW_ * s;
    if (cw <= 0.0f && !content_.empty()) {
        // 文本内容：用字号估算（调用方一般通过 SetContentWidth 给精确值）
        cw = content_.size() * kDescFS * 0.6f * s;
    }
    float x = right - cw - rightBlock;
    float w = FzMx(cw, 0.0f);
    float itemH = FzMn(h - 2.0f * kPad * s, 32.0f * s);
    (void)kContentGap;
    return RectF(x, cy - itemH * 0.5f, w, itemH);
}

// ============================================================
// 绘制
// ============================================================
void SettingsCard::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;
    const float s = scale;
    const bool light = theme.lightMode;

    // ===== WinUIonWeb styles/theme.css 精确色值 =====
    const Color cardBg     = light ? Color(1, 1, 1, 0.70f)       : Color(1, 1, 1, 0.0512f);
    const Color cardStroke = light ? Color(0, 0, 0, 0.06f)       : Color(0, 0, 0, 0.10f);
    const Color ctrlBorder = light ? Color(0, 0, 0, 0.06f)       : Color(1, 1, 1, 0.0706f);
    const Color ctrlBorderAccent = light ? Color(0, 0, 0, 0.16f) : Color(1, 1, 1, 0.0941f);
    const Color fillHover = light ? Color(0.976f, 0.976f, 0.976f, 0.50f)
                                  : Color(1, 1, 1, 0.0837f);
    const Color fillPress = light ? Color(0.976f, 0.976f, 0.976f, 0.30f)
                                  : Color(1, 1, 1, 0.0326f);
    const Color textPrimary   = theme.TextPrimary();
    const Color textSecondary = theme.TextSecondary();

    const float radius = kRadius * s;
    const bool pressing = pressed_ && clickable_;
    const float bgT = pressing ? FzMx(pressT_, hoverT_) : hoverT_;

    // ---- 1. 背景（card-bg；hover→fillHover、active→fillPress）----
    {
        Color bg = cardBg;
        if (clickable_ && bgT > 0.001f) {
            const Color target = pressing ? fillPress : fillHover;
            bg = Color::Lerp(cardBg, target, pressing ? pressT_ : hoverT_);
        }
        renderer.FillRoundedRect(bounds_, radius, bg);
    }

    // ---- 2. 边框（card-stroke；clickable hover/active → ctrl-border）----
    {
        Color border = cardStroke;
        if (clickable_ && bgT > 0.001f) border = ctrlBorder;
        renderer.StrokeRoundedRect(bounds_, radius, 1.0f * s, border);
        // 单击强调边：light 下边线 / dark 上边线（ctrl-border-accent）
        if (clickable_ && bgT > 0.001f) {
            float y = light ? bounds_.Bottom() - 1.0f * s : bounds_.Top();
            renderer.DrawLine(bounds_.x + 1.0f * s, y,
                              bounds_.Right() - 1.0f * s, y,
                              1.0f * s, ctrlBorderAccent.WithAlpha(bgT));
        }
    }

    // ---- 3. Header 区（左）----
    const float padX = kPad * s;
    // 右侧占用（content + action icon + gap）
    const RectF contentBox = ContentRect(s);
    float rightReserved = (bounds_.Right() - kPad * s) - contentBox.x + kGap * s;
    float mainRight = bounds_.x + bounds_.w - padX - rightReserved;

    // 图标
    float mainLeft = bounds_.x + padX;
    RectF iconBox;
    if (HasHeaderIcon()) {
        float d = kIconSize * s;
        iconBox = RectF(mainLeft + 2.0f * s,
                        bounds_.y + (bounds_.h - d) * 0.5f, d, d);
        mainLeft = iconBox.x + d + kIconGap * s;
    }

    // 文字（active → secondary；否则 primary）
    const Color titleCol = pressing ? textSecondary : textPrimary;
    const Color descCol  = textSecondary;
    const float textW = FzMx(0.0f, mainRight - mainLeft);
    const float lineH1 = kTitleLineH * s;
    const float lineH2 = kDescLineH * s;

    if (!description_.empty()) {
        float blockH = lineH1 + lineH2;
        float top = bounds_.y + (bounds_.h - blockH) * 0.5f;
        renderer.DrawText(header_, mainLeft, top, textW, lineH1,
                          kFace, kTitleFS * s, DWRITE_FONT_WEIGHT_NORMAL, titleCol,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        renderer.DrawText(description_, mainLeft, top + lineH1, textW, lineH2,
                          kFace, kDescFS * s, DWRITE_FONT_WEIGHT_NORMAL, descCol,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    } else if (!header_.empty()) {
        float top = bounds_.y + (bounds_.h - lineH1) * 0.5f;
        renderer.DrawText(header_, mainLeft, top, textW, lineH1,
                          kFace, kTitleFS * s, DWRITE_FONT_WEIGHT_NORMAL, titleCol,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    // 图标（secondary；active → secondary 保持不变）
    if (HasHeaderIcon()) {
        DrawFluentIcon(renderer, static_cast<FluentIcon>(headerIcon_),
                       iconBox.x, iconBox.y, iconBox.w, textSecondary);
    }

    // ---- 4. 右侧 Content 纯文本（右对齐）----
    if (!content_.empty()) {
        renderer.DrawText(content_, contentBox.x, contentBox.y, contentBox.w, contentBox.h,
                          kFace, kTitleFS * s, DWRITE_FONT_WEIGHT_NORMAL,
                          pressing ? textSecondary : textPrimary,
                          DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    // ---- 5. ActionIcon（chevron 右向，13px，secondary）----
    if (actionIcon_) {
        float d = kActionFS * s;
        float ax = bounds_.x + bounds_.w - padX - d;
        float ay = bounds_.y + (bounds_.h - d) * 0.5f;
        // ChevronDown 默认朝下 → 旋转 -90° 朝右
        DrawFluentIcon(renderer, FluentIcon::ChevronDown,
                       ax, ay, d, textSecondary, -0.5f * 3.14159265f);
    }
}

} // namespace ModernDesign