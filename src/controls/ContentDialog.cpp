#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ContentDialog.h"
#include "controls/Button.h"
#include "utils/Easing.h"

namespace ModernDesign {

using Controls::Button;
using Controls::ButtonVariant;

namespace {

constexpr const wchar_t* kFace = L"Segoe UI";

// 按空格贪心断行（无空格文本如中文整段作为一行）
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

        if (w <= maxW || cur.empty()) {
            cur = cand;
        } else {
            out.push_back(cur);
            cur = tok;
        }

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
// 按钮配置
// ============================================================
void ContentDialog::SetButtons(const std::wstring& primary,
                               const std::wstring& secondary,
                               const std::wstring& close) {
    primaryText_ = primary;
    secondaryText_ = secondary;
    closeText_ = close;
}

int ContentDialog::VisibleButtonCount() const {
    int n = 0;
    if (HasPrimary()) ++n;
    if (HasSecondary()) ++n;
    if (HasClose()) ++n;
    return n;
}

// ============================================================
// 开关
// ============================================================
void ContentDialog::Show() {
    if (visible_ && !closing_) return;
    visible_ = true;
    closing_ = false;
    anim_ = true;
    elapsed_ = 0.0f;
    cardT_ = 0.0f;
    overlayT_ = 0.0f;
    pressBtn_ = -1;
    primaryBtn_.OnMouseLeave();
    secondaryBtn_.OnMouseLeave();
    closeBtn_.OnMouseLeave();
    if (onOpened_) onOpened_();
}

void ContentDialog::Hide() { Close(Result::None); }

void ContentDialog::Close(Result result) {
    if (!visible_ || closing_) return;
    closing_ = true;
    anim_ = true;
    elapsed_ = 0.0f;
    pressBtn_ = -1;
    if (onResult_) onResult_(result);
}

// ============================================================
// 布局（规格：ContentDialog.vue）
// ============================================================
float ContentDialog::WrapBody(Renderer& renderer, float maxW, float scale) {
    bodyLines_.clear();
    if (content_.empty()) return 0.0f;
    WrapLines(renderer, content_, maxW, kBodySize * scale, bodyLines_);
    return static_cast<float>(bodyLines_.size()) * kBodyLine * scale;
}

void ContentDialog::Layout(Renderer& renderer, float scale) {
    const float s = scale;
    const float pad = kPad * s;
    const float outer = kOuterPad * s;

    const float availW = FzMx(1.0f, bounds_.w - 2.0f * outer);
    const float availH = FzMx(1.0f, bounds_.h - 2.0f * outer);

    const float w = fullSize_ ? bounds_.w
                              : FzMn(kMaxWidth * s, FzMx(kMinWidth * s, availW));
    const float maxH = fullSize_ ? bounds_.h : FzMn(kMaxHeight * s, availH);

    // 正文换行
    bodyH_ = WrapBody(renderer, w - 2.0f * pad, scale);

    const float titleH = title_.empty() ? 0.0f : kTitleLine * s;
    const float contentNat = pad + titleH + (title_.empty() ? 0.0f : kTitleGap * s)
                             + bodyH_ + contentDrawH_ * s + pad;
    const float contentH = (contentHeight_ > 0.0f) ? contentHeight_ * s : contentNat;
    const float cmdH = (VisibleButtonCount() > 0) ? (pad + kBtnH * s + pad) : 0.0f;

    float h = FzMx(contentH + cmdH, kMinHeight * s);
    if (!fullSize_) h = FzMn(h, maxH);

    const float x = bounds_.x + (bounds_.w - w) * 0.5f;
    const float y = bounds_.y + (bounds_.h - h) * 0.5f;
    card_ = RectF(x, y, w, h);
    contentBox_ = RectF(x, y, w, FzMx(0.0f, h - cmdH));
    cmdBox_ = RectF(x, y + h - cmdH, w, cmdH);

    // ---- 按钮列分配 ----
    // n=3 → Primary | Secondary | Close 三等分（列宽 (rowW-2*8)/3）
    // n=2 → 两个可见按钮左右各一
    // n=1 → 只占右半（列宽 (rowW-8)/2）
    const int n = VisibleButtonCount();
    const float gap = kBtnGap * s;
    const float rowW = FzMx(0.0f, w - 2.0f * pad);
    const float colW = (n == 1) ? (rowW - gap) * 0.5f
                                : ((n > 1) ? (rowW - (n - 1) * gap) / n : 0.0f);
    const float by = cmdBox_.y + pad;

    int idx = 0;
    auto place = [&](Button& b, RectF& out, const std::wstring& text,
                     bool enabled, bool accent) {
        if (text.empty()) {
            out = RectF();
            b.SetBounds(RectF());
            return;
        }
        const int col = (n == 1) ? 1 : idx++;
        out = RectF(x + pad + col * (colW + gap), by, colW, kBtnH * s);
        b.SetText(text);
        b.SetVariant(accent ? ButtonVariant::Accent : ButtonVariant::Standard);
        b.SetEnabled(enabled);
        b.SetBounds(out);
    };

    place(primaryBtn_, primaryRect_, primaryText_, primaryEnabled_,
          defaultButton_ == DefaultButton::Primary);
    place(secondaryBtn_, secondaryRect_, secondaryText_, secondaryEnabled_,
          defaultButton_ == DefaultButton::Secondary);
    place(closeBtn_, closeRect_, closeText_, true,
          defaultButton_ == DefaultButton::Close);
}

// ============================================================
// 动画
// ============================================================
bool ContentDialog::Update(float dt) {
    bool busy = false;

    if (anim_) {
        elapsed_ += dt;

        // 遮罩：83ms 线性
        const float po = FzMn(1.0f, elapsed_ / kOverlayFade);
        overlayT_ = closing_ ? (1.0f - po) : po;

        // 卡片缩放：250ms（开）/167ms（关），cubic-bezier(0,0,0,1)
        const float dur = closing_ ? kCloseDur : kOpenDur;
        const float p = FzMn(1.0f, elapsed_ / dur);
        const float e = EaseStandardOut(p);
        cardT_ = closing_ ? (1.0f - e) : e;

        if (p >= 1.0f) {
            anim_ = false;
            cardT_ = closing_ ? 0.0f : 1.0f;
            overlayT_ = closing_ ? 0.0f : 1.0f;
            if (closing_) {
                closing_ = false;
                visible_ = false;
            }
        }
        busy = true;
    }

    if (HasPrimary()) busy |= primaryBtn_.Update(dt);
    if (HasSecondary()) busy |= secondaryBtn_.Update(dt);
    if (HasClose()) busy |= closeBtn_.Update(dt);
    if (visible_ && !closing_ && contentUpdate_) busy |= contentUpdate_(dt);

    return busy;
}

// ============================================================
// 绘制
// ============================================================
void ContentDialog::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (!visible_ && overlayT_ <= 0.001f) return;

    Layout(renderer, scale);

    const float s = scale;
    const float pad = kPad * s;
    const float radius = kCorner * s;

    // ---- 1) 遮罩 ----
    const float smoke = kSmokeAlpha * Clamp01(overlayT_);
    if (smoke > 0.002f)
        renderer.FillRect(bounds_, Color(0.0f, 0.0f, 0.0f, smoke));

    // ---- 2) 卡片（含 scale(1.05 → 1.0) 动画）----
    const float k = LerpF(1.0f, kScaleFrom, 1.0f - Clamp01(cardT_));
    renderer.PushScale(k, card_.CenterX(), card_.CenterY());

    renderer.FillDropShadow(card_, radius, kShadowOffY * s, kShadowBlur * s,
                            kShadowAlpha * Clamp01(cardT_));
    renderer.FillRoundedRect(card_, radius, theme.DialogBg());

    const float pad2 = pad * 2.0f;

    // 内容区（上圆角、下直角）
    const Color contentBg = theme.DialogContentBg();
    if (contentBg.a > 0.001f && contentBox_.h > 1.0f) {
        renderer.FillRoundedRect(contentBox_, radius, contentBg);
        renderer.FillRect(RectF(contentBox_.x, contentBox_.Bottom() - radius,
                                contentBox_.w, radius), contentBg);
    }

    // 内容区与命令区的分隔线
    if (cmdBox_.h > 0.5f && contentBox_.h > 1.0f)
        renderer.FillRect(RectF(contentBox_.x, contentBox_.Bottom() - 1.0f * s,
                                contentBox_.w, 1.0f * s), theme.CardBorder());

    // 命令区（下圆角、上直角）
    if (cmdBox_.h > 0.5f) {
        const Color cmdBg = theme.DialogCmdBg();
        renderer.FillRoundedRect(cmdBox_, radius, cmdBg);
        renderer.FillRect(RectF(cmdBox_.x, cmdBox_.y, cmdBox_.w, radius), cmdBg);
    }

    // ---- 3) 文本：Title 20/28 w600（下间距 12）+ Body 14/20 ----
    const Color fg = theme.TextPrimary();
    float ty = card_.y + pad;
    if (!title_.empty()) {
        renderer.DrawText(title_, card_.x + pad, ty,
                          FzMx(0.0f, card_.w - pad2), kTitleLine * s,
                          kFace, kTitleSize * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, fg,
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        ty += (kTitleLine + kTitleGap) * s;
    }
    for (const std::wstring& line : bodyLines_) {
        renderer.DrawText(line, card_.x + pad, ty,
                          FzMx(0.0f, card_.w - pad2), kBodyLine * s,
                          kFace, kBodySize * s, DWRITE_FONT_WEIGHT_NORMAL, fg,
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        ty += kBodyLine * s;
    }

    // ---- 4) 自定义内容（排在正文下方，占用内容区去掉 padding 的剩余空间）----
    if (contentDraw_) {
        const RectF inner(card_.x + pad, card_.y + pad + bodyH_,
                          FzMx(0.0f, card_.w - pad2),
                          FzMx(0.0f, contentBox_.h - pad2 - bodyH_));
        contentDraw_(renderer, theme, s, inner);
    }

    // ---- 5) 按钮 ----
    if (!primaryRect_.IsEmpty()) primaryBtn_.Draw(renderer, theme, s);
    if (!secondaryRect_.IsEmpty()) secondaryBtn_.Draw(renderer, theme, s);
    if (!closeRect_.IsEmpty()) closeBtn_.Draw(renderer, theme, s);

    // ---- 6) 卡片描边（最后画，压住内容/按钮毛边）----
    renderer.StrokeRoundedRect(card_, radius, kBorderW * s, theme.DialogBorder());

    renderer.PopTransform();
}

// ============================================================
// 输入（模态：可见时吞掉一切）
// ============================================================
bool ContentDialog::OnMouseMove(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (contentMove_) contentMove_(x, y);
    if (!primaryRect_.IsEmpty()) primaryBtn_.OnMouseMove(x, y);
    if (!secondaryRect_.IsEmpty()) secondaryBtn_.OnMouseMove(x, y);
    if (!closeRect_.IsEmpty()) closeBtn_.OnMouseMove(x, y);
    return true;
}

bool ContentDialog::OnMouseDown(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (contentDown_) contentDown_(x, y);

    pressBtn_ = -1;
    if (!primaryRect_.IsEmpty() && primaryRect_.Contains(x, y)) pressBtn_ = 0;
    else if (!secondaryRect_.IsEmpty() && secondaryRect_.Contains(x, y)) pressBtn_ = 1;
    else if (!closeRect_.IsEmpty() && closeRect_.Contains(x, y)) pressBtn_ = 2;

    // 让按钮进入 pressed 视觉状态（超出边界的由 Button 自己忽略）
    if (!primaryRect_.IsEmpty()) primaryBtn_.OnMouseDown(x, y);
    if (!secondaryRect_.IsEmpty()) secondaryBtn_.OnMouseDown(x, y);
    if (!closeRect_.IsEmpty()) closeBtn_.OnMouseDown(x, y);
    return true;
}

bool ContentDialog::OnMouseUp(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (contentUp_) contentUp_(x, y);

    int hit = -1;
    if (!primaryRect_.IsEmpty() && primaryRect_.Contains(x, y)) hit = 0;
    else if (!secondaryRect_.IsEmpty() && secondaryRect_.Contains(x, y)) hit = 1;
    else if (!closeRect_.IsEmpty() && closeRect_.Contains(x, y)) hit = 2;

    if (!primaryRect_.IsEmpty()) primaryBtn_.OnMouseUp(x, y);
    if (!secondaryRect_.IsEmpty()) secondaryBtn_.OnMouseUp(x, y);
    if (!closeRect_.IsEmpty()) closeBtn_.OnMouseUp(x, y);

    const int prev = pressBtn_;
    pressBtn_ = -1;

    if (prev >= 0 && prev == hit) {
        if (prev == 0) Close(Result::Primary);
        else if (prev == 1) Close(Result::Secondary);
        else Close(Result::None);
    } else if (lightDismiss_ && !card_.Contains(x, y)) {
        Close(Result::None);
    }
    return true;
}

void ContentDialog::OnMouseLeave() {
    if (!visible_) return;
    if (contentLeave_) contentLeave_(0.0f, 0.0f);
    primaryBtn_.OnMouseLeave();
    secondaryBtn_.OnMouseLeave();
    closeBtn_.OnMouseLeave();
}

bool ContentDialog::OnKeyDown(int vk) {
    if (!visible_) return false;
    if (anim_) return true;

    if (vk == VK_ESCAPE) {
        Close(Result::None);
        return true;
    }
    if (vk == VK_RETURN) {
        switch (defaultButton_) {
        case DefaultButton::Primary:
            if (HasPrimary() && primaryEnabled_) Close(Result::Primary);
            break;
        case DefaultButton::Secondary:
            if (HasSecondary() && secondaryEnabled_) Close(Result::Secondary);
            break;
        case DefaultButton::Close:
            if (HasClose()) Close(Result::None);
            break;
        default:
            break;
        }
        return true;
    }
    return true;   // 模态：其余按键也不下传
}

} // namespace ModernDesign