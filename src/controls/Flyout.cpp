#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Flyout.h"
#include "utils/Easing.h"

namespace ModernDesign {

namespace {

constexpr const wchar_t* kFace = L"Segoe UI";

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

void Flyout::Show() {
    if (visible_ && !closing_) return;
    visible_ = true;
    closing_ = false;
    anim_ = true;
    elapsed_ = 0.0f;
    animT_ = 0.0f;
}

void Flyout::Hide() { Close(); }

void Flyout::Close() {
    if (!visible_ || closing_) return;
    closing_ = true;
    anim_ = true;
    elapsed_ = 0.0f;
}

float Flyout::WrapBody(Renderer& renderer, float maxW, float scale) {
    lines_.clear();
    if (content_.empty()) return 0.0f;
    WrapLines(renderer, content_, maxW, kFontSize * scale, lines_);
    return static_cast<float>(lines_.size()) * kLineHeight * scale;
}

void Flyout::Layout(Renderer& renderer, float scale) {
    const float s = scale;

    const float maxW = FzMn(kMaxWidth * s, FzMx(0.0f, bounds_.w - 16.0f * s));
    const float maxH = FzMn(kMaxHeight * s, FzMx(0.0f, bounds_.h - 16.0f * s));

    // 内容宽度：文本自然宽度 + padding，且不小于锚点宽度
    float textW = 0.0f;
    if (!content_.empty()) {
        float w = 0.0f, h = 0.0f;
        renderer.MeasureText(content_, 100000.0f, kFace, kFontSize * s,
                             DWRITE_FONT_WEIGHT_NORMAL, &w, &h);
        textW = w;
    }
    float w = FzMx(FzMx(textW + (kPadL + kPadR) * s, anchor_.w), kMinWidth * s);
    w = FzMn(w, maxW);

    const float wrapW = FzMx(1.0f, w - (kPadL + kPadR) * s);
    contentH_ = WrapBody(renderer, wrapW, scale) + contentDrawH_ * s;

    float h = FzMx(contentH_ + (kPadT + kPadB) * s, kMinHeight * s);
    h = FzMn(h, maxH);

    const PopupLayout pl = PlacePopup(placement_, anchor_, w, h, bounds_,
                                      kGap * s, kMargin * s);
    panel_ = pl.rect;
    above_ = pl.above;
}

bool Flyout::Update(float dt) {
    bool busy = false;
    if (anim_) {
        elapsed_ += dt;
        const float dur = closing_ ? kCloseDur : kOpenDur;
        const float p = FzMn(1.0f, elapsed_ / dur);
        const float e = closing_ ? EaseFlyoutClose(p) : EaseFlyoutOpen(p);
        animT_ = closing_ ? (1.0f - e) : e;
        if (p >= 1.0f) {
            anim_ = false;
            animT_ = closing_ ? 0.0f : 1.0f;
            if (closing_) {
                closing_ = false;
                visible_ = false;
                if (onClosed_) onClosed_();
            }
        }
        busy = true;
    }
    if (visible_ && !closing_) {
        if (contentUpdate_) busy |= contentUpdate_(dt);
    }
    return busy;
}

void Flyout::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (!visible_ && animT_ <= 0.001f) return;

    Layout(renderer, scale);

    const float s = scale;
    const float vis = Clamp01(animT_);

    // 完全展开时的裁剪 = 面板 + 32px 留出阴影（Flyout.vue 的 --flyout-shadow-bleed）
    const float bleed = kShadowBleed * s * vis;
    const float band = FzMx(1.0f, panel_.h * vis);
    RectF clip = above_
        ? RectF(panel_.x - bleed, panel_.Bottom() - band - bleed,
                panel_.w + bleed * 2.0f, band + bleed)
        : RectF(panel_.x - bleed, panel_.y - bleed,
                panel_.w + bleed * 2.0f, band + bleed);

    // 位移：打开时从锚点侧滑入 ±16；关闭时上移 4
    float dy = 0.0f;
    if (closing_) dy = -kCloseShift * s * (1.0f - vis);
    else dy = (above_ ? kSlide : -kSlide) * s * (1.0f - vis);
    clip.y -= dy;

    renderer.PushTranslate(0.0f, dy);
    renderer.PushClip(clip);

    // 面板（阴影 + 底 + 描边）
    const float fade = closing_
        ? (1.0f - FzMn(1.0f, elapsed_ / kFadeDur))
        : FzMn(1.0f, elapsed_ / kFadeDur);

    renderer.FillDropShadow(panel_, kRadius * s, kShadowY * s, kShadowBlur * s,
                            kShadowA * fade);
    renderer.FillRoundedRect(panel_, kRadius * s, theme.FlyoutBg().WithAlpha(theme.FlyoutBg().a * fade));
    renderer.StrokeRoundedRect(panel_, kRadius * s, 1.0f * s,
                               theme.FlyoutBorder().WithAlpha(theme.FlyoutBorder().a * fade));

    // 文本内容
    const Color fg = theme.TextPrimary().WithAlpha(fade);
    float ty = panel_.y + kPadT * s;
    for (const std::wstring& line : lines_) {
        renderer.DrawText(line, panel_.x + kPadL * s, ty,
                          FzMx(0.0f, panel_.w - (kPadL + kPadR) * s), kLineHeight * s,
                          kFace, kFontSize * s, DWRITE_FONT_WEIGHT_NORMAL, fg,
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        ty += kLineHeight * s;
    }

    // 自定义内容（排在文本下方）
    if (contentDraw_) {
        const RectF inner(panel_.x + kPadL * s, panel_.y + kPadT * s + lines_.size() * kLineHeight * s,
                          FzMx(0.0f, panel_.w - (kPadL + kPadR) * s), contentDrawH_ * s);
        contentDraw_(renderer, theme, s, inner);
    }

    renderer.PopClip();
    renderer.PopTransform();
}

// ============================================================
// 输入
// ============================================================
bool Flyout::OnMouseMove(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (contentMove_) contentMove_(x, y);
    return true;
}

bool Flyout::OnMouseDown(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (panel_.Contains(x, y)) {
        if (contentDown_) contentDown_(x, y);
    } else if (lightDismiss_) {
        Close();
    }
    return true;
}

bool Flyout::OnMouseUp(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    if (panel_.Contains(x, y) && contentUp_) contentUp_(x, y);
    return true;
}

void Flyout::OnMouseLeave() {
    if (!visible_) return;
    if (contentLeave_) contentLeave_(0.0f, 0.0f);
}

bool Flyout::OnKeyDown(int vk) {
    if (!visible_) return false;
    if (anim_) return true;
    if (vk == VK_ESCAPE) {
        Close();
        return true;
    }
    return false;   // 非模态：其余按键继续下传
}

} // namespace ModernDesign