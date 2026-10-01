#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ToolTip.h"
#include "utils/Color.h"

namespace ModernDesign {

namespace {

constexpr const wchar_t* kFace = L"Segoe UI";

// 按空格贪心断行（换行符强制断行）；与 Flyout 同策略
void WrapLines(Renderer& r, const std::wstring& text, float maxW, float fontSize,
               std::vector<std::wstring>& out) {
    out.clear();
    if (text.empty() || maxW <= 1.0f) return;
    std::wstring cur;
    size_t i = 0;
    while (i <= text.size()) {
        const size_t sp = text.find_first_of(L" \n", i);
        const bool done = (sp == std::wstring::npos);
        const size_t end = done ? text.size() : sp;
        const std::wstring tok = text.substr(i, end - i);
        const bool hardBreak = (!done && text[sp] == L'\n');
        const std::wstring cand = cur.empty() ? tok : (cur + L" " + tok);
        float w = 0.0f, h = 0.0f;
        r.MeasureText(cand, 100000.0f, kFace, fontSize, DWRITE_FONT_WEIGHT_NORMAL, &w, &h);
        if (w <= maxW || cur.empty()) cur = cand;
        else { out.push_back(cur); cur = tok; }
        if (hardBreak || done) {
            if (!cur.empty() || hardBreak) out.push_back(cur);
            cur.clear();
        }
        if (done) break;
        i = sp + 1;
    }
}

} // namespace

// ============================================================
// 显示 / 隐藏
// ============================================================
void ToolTip::RequestShow(float delay) {
    if (!enabled_ || content_.empty()) return;
    if (visible_ && !closing_) return;          // 已显示

    waiting_ = false;
    if (delay <= 0.0f) {
        BeginOpen();
    } else {
        waiting_ = true;
        waitLeft_ = delay;
    }
}

void ToolTip::BeginOpen() {
    animFrom_ = animT_;
    visible_ = true;
    closing_ = false;
    anim_ = true;
    elapsed_ = 0.0f;
}

void ToolTip::BeginClose() {
    if (!visible_ || closing_) return;
    animFrom_ = animT_;
    closing_ = true;
    anim_ = true;
    elapsed_ = 0.0f;
}

void ToolTip::Show(bool immediate) { RequestShow(immediate ? 0.0f : initialDelay_); }

void ToolTip::Hide(bool force) {
    if (!force && (hoverTarget_ || hoverTip_)) return;
    waiting_ = false;
    if (!visible_) return;
    BeginClose();
}

// ============================================================
// 指针交互
// ============================================================
void ToolTip::OnPointerEnter(const RectF& target, float x, float y) {
    if (!enabled_ || content_.empty()) return;
    hoverTarget_ = true;
    anchor_ = target;
    pointer_ = RectF(x, y, 0.0f, 0.0f);

    // BetweenShowDelay：刚关掉一个提示就移到下一个目标 → 立即显示
    const float sinceClose = (lastCloseTime_ >= 0.0f) ? (timeAccum_ - lastCloseTime_) : 1e9f;
    RequestShow(sinceClose <= betweenDelay_ ? 0.0f : initialDelay_);
}

void ToolTip::OnPointerMove(float x, float y) {
    if (!hoverTarget_ || visible_) return;      // 已显示后位置固定（与参考实现一致）
    pointer_ = RectF(x, y, 0.0f, 0.0f);
}

void ToolTip::OnPointerLeave() {
    hoverTarget_ = false;
    if (!visible_) { waiting_ = false; return; }
    Hide(false);                                // 仍悬停在提示本体上则保持
}

void ToolTip::OnPointerEnterTip() { hoverTip_ = true; }

void ToolTip::OnPointerLeaveTip() {
    hoverTip_ = false;
    if (!hoverTarget_) Hide(false);
}

// ============================================================
// 更新 / 布局
// ============================================================
bool ToolTip::Update(float dt) {
    timeAccum_ += dt;
    bool busy = false;

    if (waiting_) {
        waitLeft_ -= dt;
        if (waitLeft_ <= 0.0f) {
            waiting_ = false;
            BeginOpen();
        }
        busy = true;
    }

    if (anim_) {
        elapsed_ += dt;
        const float p = FzMn(1.0f, elapsed_ / kFadeDur);
        animT_ = LerpF(animFrom_, closing_ ? 0.0f : 1.0f, p);
        if (p >= 1.0f) {
            anim_ = false;
            animT_ = closing_ ? 0.0f : 1.0f;
            if (closing_) {
                closing_ = false;
                visible_ = false;
                lastCloseTime_ = timeAccum_;
            }
        }
        busy = true;
    }
    return busy;
}

void ToolTip::WrapBody(Renderer& renderer, float maxW, float scale) {
    WrapLines(renderer, content_, maxW, kFontSize * scale, lines_);
}

void ToolTip::Layout(Renderer& renderer, float scale) {
    const float s = scale;
    const float maxW = FzMn(kMaxWidth * s, FzMx(0.0f, bounds_.w - 16.0f * s));

    // 宽度：随内容（min-width max-content），上限 maxW
    float textW = 0.0f;
    if (!content_.empty()) {
        float w = 0.0f, h = 0.0f;
        renderer.MeasureText(content_, 100000.0f, kFace, kFontSize * s,
                             DWRITE_FONT_WEIGHT_NORMAL, &w, &h);
        textW = w;
    }
    float w = FzMn(textW + (kPadL + kPadR) * s, maxW);
    w = FzMx(w, 1.0f);

    const float wrapW = FzMx(1.0f, w - (kPadL + kPadR) * s);
    WrapBody(renderer, wrapW, scale);
    float h = static_cast<float>(lines_.size()) * kLineHeight * s + (kPadT + kPadB) * s;
    h = FzMx(h, 1.0f);

    const float gap = kGap * s;
    const float mg = kMargin * s;

    float x = 0.0f, y = 0.0f;

    if (placement_ == ToolTipPlacement::Mouse) {
        // 指针模式：优先上方，其次下方，最后夹回
        const float left = pointer_.x - w * 0.5f;
        const float aboveTop = pointer_.y - h - gap;
        const float belowTop = pointer_.y + gap;
        if (aboveTop >= bounds_.y + mg)                  y = aboveTop;
        else if (belowTop + h <= bounds_.Bottom() - mg)  y = belowTop;
        else                                             y = aboveTop;
        x = left;
    } else {
        // 目标模式：四方向候选 + 就近回退顺序
        const float cx = anchor_.x + (anchor_.w - w) * 0.5f;
        const float cy = anchor_.y + (anchor_.h - h) * 0.5f;
        const float candX[4] = { cx, cx, anchor_.x - w - gap, anchor_.Right() + gap };
        const float candY[4] = { anchor_.y - h - gap, anchor_.Bottom() + gap, cy, cy };
        const bool fits[4] = {
            (candY[0] >= bounds_.y + mg),
            (candY[1] + h <= bounds_.Bottom() - mg),
            (candX[2] >= bounds_.x + mg),
            (candX[3] + w <= bounds_.Right() - mg)
        };
        // 索引：0=Top 1=Bottom 2=Left 3=Right
        int order[4];
        switch (placement_) {
            case ToolTipPlacement::Top:    order[0] = 0; order[1] = 1; order[2] = 2; order[3] = 3; break;
            case ToolTipPlacement::Bottom: order[0] = 1; order[1] = 0; order[2] = 2; order[3] = 3; break;
            case ToolTipPlacement::Left:   order[0] = 2; order[1] = 3; order[2] = 0; order[3] = 1; break;
            default:                       order[0] = 3; order[1] = 2; order[2] = 0; order[3] = 1; break;
        }
        int chosen = order[0];
        for (int k = 0; k < 4; ++k) {
            if (fits[order[k]]) { chosen = order[k]; break; }
        }
        x = candX[chosen];
        y = candY[chosen];
    }

    x = ClampF(x, bounds_.x + mg, FzMx(bounds_.x + mg, bounds_.Right() - mg - w));
    y = ClampF(y, bounds_.y + mg, FzMx(bounds_.y + mg, bounds_.Bottom() - mg - h));
    panel_ = RectF(x, y, w, h);
}

// ============================================================
// 绘制
// ============================================================
void ToolTip::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (!visible_ && animT_ <= 0.001f) return;

    Layout(renderer, scale);

    const float s = scale;
    const float a = Clamp01(animT_);
    if (a <= 0.001f) return;

    // 阴影：主投影 + 2px 描边光晕（ToolTip.vue 的两层 box-shadow）
    renderer.FillDropShadow(panel_, kRadius * s, kShadowY * s, kShadowBlur * s, kShadowA * a);
    renderer.FillDropShadow(panel_, kRadius * s, 0.0f, kEdgeBlur * s, kEdgeA * a);

    renderer.FillRoundedRect(panel_, kRadius * s, theme.TipBg().WithAlpha(theme.TipBg().a * a));
    renderer.StrokeRoundedRect(panel_, kRadius * s, 1.0f * s,
                               theme.FlyoutBorder().WithAlpha(theme.FlyoutBorder().a * a));

    const Color fg = theme.TextPrimary().WithAlpha(a);
    float ty = panel_.y + kPadT * s;
    for (const std::wstring& line : lines_) {
        renderer.DrawText(line, panel_.x + kPadL * s, ty,
                          FzMx(0.0f, panel_.w - (kPadL + kPadR) * s), kLineHeight * s,
                          kFace, kFontSize * s, DWRITE_FONT_WEIGHT_NORMAL, fg,
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        ty += kLineHeight * s;
    }
}

} // namespace ModernDesign