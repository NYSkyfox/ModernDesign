#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/MenuFlyout.h"
#include "utils/Easing.h"
#include "utils/FluentIcons.h"

namespace ModernDesign {

namespace {
constexpr const wchar_t* kFace = L"Segoe UI";
}

// ============================================================
// 内容维护
// ============================================================
int MenuFlyout::AddItem(const std::wstring& text, const std::wstring& accelerator, bool enabled) {
    Item it;
    it.text = text;
    it.accelerator = accelerator;
    it.enabled = enabled;
    items_.push_back(it);
    return static_cast<int>(items_.size()) - 1;
}

int MenuFlyout::AddItemWithIcon(const std::wstring& text, FluentIcon icon,
                                const std::wstring& accelerator, bool enabled) {
    const int i = AddItem(text, accelerator, enabled);
    items_[i].icon = icon;
    items_[i].hasIcon = true;
    return i;
}

int MenuFlyout::AddToggle(const std::wstring& text, bool checked,
                          const std::wstring& accelerator) {
    const int i = AddItem(text, accelerator, true);
    items_[i].kind = ItemKind::Toggle;
    items_[i].checked = checked;
    return i;
}

int MenuFlyout::AddRadio(const std::wstring& text, bool checked,
                         const std::wstring& accelerator) {
    const int i = AddItem(text, accelerator, true);
    items_[i].kind = ItemKind::Radio;
    items_[i].checked = checked;
    return i;
}

void MenuFlyout::AddSeparator() {
    Item it;
    it.kind = ItemKind::Separator;
    items_.push_back(it);
}

void MenuFlyout::Clear() {
    items_.clear();
    itemRects_.clear();
    hotIndex_ = -1;
    pressedIndex_ = -1;
}

void MenuFlyout::SetItemEnabled(int index, bool enabled) {
    if (index >= 0 && index < static_cast<int>(items_.size())) items_[index].enabled = enabled;
}

void MenuFlyout::SetItemChecked(int index, bool checked) {
    if (index < 0 || index >= static_cast<int>(items_.size())) return;
    Item& it = items_[index];
    it.checked = checked;
    if (checked && it.kind == ItemKind::Radio) {
        for (int i = 0; i < static_cast<int>(items_.size()); ++i)
            if (i != index && items_[i].kind == ItemKind::Radio) items_[i].checked = false;
    }
}

bool MenuFlyout::IsItemChecked(int index) const {
    if (index < 0 || index >= static_cast<int>(items_.size())) return false;
    return items_[index].checked;
}

int MenuFlyout::CheckedToggleIndex() const {
    for (int i = 0; i < static_cast<int>(items_.size()); ++i)
        if (items_[i].kind == ItemKind::Toggle && items_[i].checked) return i;
    return -1;
}

int MenuFlyout::CheckedRadioIndex() const {
    for (int i = 0; i < static_cast<int>(items_.size()); ++i)
        if (items_[i].kind == ItemKind::Radio && items_[i].checked) return i;
    return -1;
}

// ============================================================
// 开关 / 布局
// ============================================================
void MenuFlyout::Show() {
    if (visible_ && !closing_) return;
    visible_ = true;
    closing_ = false;
    anim_ = true;
    elapsed_ = 0.0f;
    animT_ = 0.0f;
    hotIndex_ = -1;
    pressedIndex_ = -1;
}

void MenuFlyout::Hide() { Close(); }

void MenuFlyout::Close() {
    if (!visible_ || closing_) return;
    closing_ = true;
    anim_ = true;
    elapsed_ = 0.0f;
    hotIndex_ = -1;
    pressedIndex_ = -1;
}

RectF MenuFlyout::ItemRect(int index) const {
    if (index < 0 || index >= static_cast<int>(itemRects_.size())) return RectF();
    return itemRects_[index];
}

void MenuFlyout::Layout(Renderer& renderer, float scale) {
    const float s = scale;
    const float maxW = FzMn(kMaxWidth * s, FzMx(0.0f, bounds_.w - 16.0f * s));
    const float maxH = FzMn(kMaxHeight * s, FzMx(0.0f, bounds_.h - 16.0f * s));

    // 宽度：文本 + 图标/勾选槽 + 快捷键（MenuFlyout.vue 的 estimateFlyoutWidth）
    const bool anyIcon = [&] {
        for (const Item& it : items_)
            if (it.hasIcon || it.kind == ItemKind::Toggle || it.kind == ItemKind::Radio) return true;
        return false;
    }();

    float contentW = 0.0f;
    for (const Item& it : items_) {
        if (it.kind == ItemKind::Separator) continue;
        float tw = 0.0f, th = 0.0f;
        renderer.MeasureText(it.text, 100000.0f, kFace, kFontSize * s,
                             DWRITE_FONT_WEIGHT_NORMAL, &tw, &th);
        float w = kItemPadL + tw + kItemPadR;
        if (anyIcon) w += (kSlot + kSlotGap) * s;
        if (!it.accelerator.empty()) {
            float aw = 0.0f, ah = 0.0f;
            renderer.MeasureText(it.accelerator, 100000.0f, kFace, kAccelSize * s,
                                 DWRITE_FONT_WEIGHT_NORMAL, &aw, &ah);
            w += (kAccelGap + aw) * s;
        }
        contentW = FzMx(contentW, w);
    }

    float w = FzMx(FzMx(contentW + 8.0f * s, anchor_.w), kMinWidth * s);
    w = FzMn(w, maxW);

    // 高度：4 + 项数×36 + 分隔线数×3（MenuFlyout.vue:724-725）
    int itemCount = 0, sepCount = 0;
    for (const Item& it : items_) {
        if (it.kind == ItemKind::Separator) ++sepCount;
        else ++itemCount;
    }
    float h = (4.0f + itemCount * 36.0f + sepCount * 3.0f) * s;
    h = FzMn(FzMx(h, 36.0f * s), maxH);

    const PopupLayout pl = PlacePopup(placement_, anchor_, w, h, bounds_,
                                      kGap * s, kMargin * s);
    panel_ = pl.rect;
    above_ = pl.above;

    // 逐项排布
    itemRects_.assign(items_.size(), RectF());
    float y = panel_.y + kPadV * s;
    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        if (items_[i].kind == ItemKind::Separator) {
            itemRects_[i] = RectF(panel_.x, y, panel_.w, kSepH * s);
            y += kSepH * s;
        } else {
            itemRects_[i] = RectF(panel_.x + 4.0f * s, y + kItemM * s,
                                  FzMx(0.0f, panel_.w - 8.0f * s), kItemH * s);
            y += (kItemH + 2.0f * kItemM) * s;
        }
    }
}

bool MenuFlyout::Update(float dt) {
    bool busy = false;
    if (anim_) {
        elapsed_ += dt;
        const float p = FzMn(1.0f, elapsed_ / kOpenDur);
        const float e = EaseStandardOut(p);
        animT_ = closing_ ? (1.0f - FzMn(1.0f, elapsed_ / kFadeDur)) : e;
        if (closing_) animT_ = 1.0f - FzMn(1.0f, elapsed_ / kFadeDur);
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
    return busy;
}

// ============================================================
// 绘制
// ============================================================
void MenuFlyout::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (!visible_ && animT_ <= 0.001f) return;

    Layout(renderer, scale);

    const float s = scale;
    const float fade = Clamp01(closing_ ? (1.0f - elapsed_ / kFadeDur)
                                        : (elapsed_ / kFadeDur));
    const float vis = closing_ ? 1.0f : Clamp01(animT_);

    // 从锚点侧揭示（MenuPopupThemeTransition 的 clip 揭示）
    const float band = FzMx(1.0f, panel_.h * vis);
    const RectF clip = above_
        ? RectF(panel_.x, panel_.Bottom() - band, panel_.w, band)
        : RectF(panel_.x, panel_.y, panel_.w, band);

    renderer.PushClip(clip);

    renderer.FillDropShadow(panel_, kRadius * s, kShadowY * s, kShadowBlur * s,
                            kShadowA * fade);
    renderer.FillRoundedRect(panel_, kRadius * s,
                             theme.FlyoutBg().WithAlpha(theme.FlyoutBg().a * fade));
    renderer.StrokeRoundedRect(panel_, kRadius * s, 1.0f * s,
                               theme.FlyoutBorder().WithAlpha(theme.FlyoutBorder().a * fade));

    const Color fg        = theme.TextPrimary().WithAlpha(fade);
    const Color fgDis     = theme.TextDisabled().WithAlpha(theme.TextDisabled().a * fade);
    const Color fgSec     = theme.TextSecondary().WithAlpha(fade);
    const Color subtleHov = (theme.lightMode ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f));
    const Color divCol    = theme.Divider().WithAlpha(theme.Divider().a * fade);

    const bool anyIcon = [&] {
        for (const Item& it : items_)
            if (it.hasIcon || it.kind == ItemKind::Toggle || it.kind == ItemKind::Radio) return true;
        return false;
    }();

    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        const Item& it = items_[i];
        const RectF r = itemRects_[i];
        if (r.IsEmpty()) continue;

        if (it.kind == ItemKind::Separator) {
            renderer.FillRect(RectF(panel_.x, r.y + 1.0f * s, panel_.w, 1.0f * s), divCol);
            continue;
        }

        const bool hot = (hotIndex_ == i) && it.enabled;
        if (hot)
            renderer.FillRoundedRect(r, kItemRadius * s, subtleHov.WithAlpha(subtleHov.a * fade));

        const float cy = r.CenterY();
        float textX = r.x + kItemPadL * s;

        // 图标 / 勾选槽
        if (anyIcon) {
            const float slotX = r.x + kItemPadL * s;
            const float slotY = cy - kSlot * s * 0.5f;
            if (it.hasIcon) {
                DrawFluentIcon(renderer, it.icon, slotX, slotY, kSlot * s,
                               it.enabled ? fg : fgDis);
            } else if (it.kind == ItemKind::Toggle && it.checked) {
                DrawFluentIcon(renderer, FluentIcon::Checkmark, slotX, slotY, kSlot * s,
                               it.enabled ? fg : fgDis);
            } else if (it.kind == ItemKind::Radio && it.checked) {
                DrawFluentIcon(renderer, FluentIcon::RadioButton, slotX, slotY, kSlot * s,
                               it.enabled ? fg : fgDis);
            }
            textX = slotX + (kSlot + kSlotGap) * s;
        }

        // 快捷键（右对齐）
        float textMaxW = r.Right() - kItemPadR * s - textX;
        if (!it.accelerator.empty()) {
            float aw = 0.0f, ah = 0.0f;
            renderer.MeasureText(it.accelerator, 100000.0f, kFace, kAccelSize * s,
                                 DWRITE_FONT_WEIGHT_NORMAL, &aw, &ah);
            const float ax = r.Right() - kItemPadR * s - aw;
            renderer.DrawText(it.accelerator, ax, r.y,
                              FzMx(0.0f, aw + 1.0f), r.h,
                              kFace, kAccelSize * s, DWRITE_FONT_WEIGHT_NORMAL,
                              (it.enabled ? fgSec : fgDis),
                              DWRITE_TEXT_ALIGNMENT_LEADING,
                              DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            textMaxW -= (kAccelGap * s + aw);
        }

        renderer.DrawText(it.text, textX, r.y, FzMx(0.0f, textMaxW), r.h,
                          kFace, kFontSize * s, DWRITE_FONT_WEIGHT_NORMAL,
                          it.enabled ? fg : fgDis,
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    renderer.PopClip();
}

// ============================================================
// 输入
// ============================================================
int MenuFlyout::HitTest(float x, float y) const {
    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        if (items_[i].kind == ItemKind::Separator) continue;
        if (itemRects_[i].Contains(x, y)) return i;
    }
    return -1;
}

bool MenuFlyout::OnMouseMove(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    const int i = HitTest(x, y);
    hotIndex_ = (i >= 0 && items_[i].enabled) ? i : -1;
    return true;
}

bool MenuFlyout::OnMouseDown(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    const int i = HitTest(x, y);
    if (i >= 0 && items_[i].enabled) {
        pressedIndex_ = i;
        hotIndex_ = i;
    } else if (i < 0) {
        Close();   // 点面板外 = light dismiss
    }
    return true;
}

bool MenuFlyout::OnMouseUp(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;
    const int i = HitTest(x, y);
    const int pressed = pressedIndex_;
    pressedIndex_ = -1;
    if (pressed >= 0 && pressed == i && items_[i].enabled) {
        Item& it = items_[i];
        if (it.kind == ItemKind::Toggle) it.checked = !it.checked;
        else if (it.kind == ItemKind::Radio) SetItemChecked(i, true);
        if (onInvoked_) onInvoked_(i);
        Close();
    }
    return true;
}

void MenuFlyout::OnMouseLeave() {
    if (!visible_) return;
    hotIndex_ = -1;
}

bool MenuFlyout::OnKeyDown(int vk) {
    if (!visible_) return false;
    if (anim_) return true;
    if (vk == VK_ESCAPE) {
        Close();
        return true;
    }
    return false;
}

} // namespace ModernDesign