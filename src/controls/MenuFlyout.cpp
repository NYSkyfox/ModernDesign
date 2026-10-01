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

MenuFlyout::MenuFlyout() = default;

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

int MenuFlyout::AddToggle(const std::wstring& text, bool checked, const std::wstring& accelerator) {
    const int i = AddItem(text, accelerator, true);
    items_[i].kind = ItemKind::Toggle;
    items_[i].checked = checked;
    return i;
}

int MenuFlyout::AddRadio(const std::wstring& text, bool checked, const std::wstring& accelerator) {
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

int MenuFlyout::AddSubItem(const std::wstring& text, const std::wstring& accelerator, bool enabled) {
    const int i = AddItem(text, accelerator, enabled);
    items_[i].kind = ItemKind::SubItem;
    return i;
}

int MenuFlyout::AddSubItemWithIcon(const std::wstring& text, FluentIcon icon, bool enabled) {
    const int i = AddSubItem(text, std::wstring(), enabled);
    items_[i].icon = icon;
    items_[i].hasIcon = true;
    return i;
}

int MenuFlyout::AddSplitItem(const std::wstring& text, const std::wstring& accelerator, bool enabled) {
    const int i = AddItem(text, accelerator, enabled);
    items_[i].kind = ItemKind::SplitItem;
    return i;
}

void MenuFlyout::AddChild(int parentIndex, const std::wstring& text, const std::wstring& accelerator) {
    if (parentIndex < 0 || parentIndex >= static_cast<int>(items_.size())) return;
    Item it;
    it.text = text;
    it.accelerator = accelerator;
    items_[parentIndex].children.push_back(it);
}

void MenuFlyout::AddChildIcon(int parentIndex, const std::wstring& text, FluentIcon icon) {
    const int c = [&] {
        if (parentIndex < 0 || parentIndex >= static_cast<int>(items_.size())) return -1;
        items_[parentIndex].children.push_back(Item());
        return static_cast<int>(items_[parentIndex].children.size()) - 1;
    }();
    if (c < 0) return;
    items_[parentIndex].children[c].text = text;
    items_[parentIndex].children[c].icon = icon;
    items_[parentIndex].children[c].hasIcon = true;
}

void MenuFlyout::ImportItem(const Item& src) {
    items_.push_back(src);
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

void MenuFlyout::ExpandSubitem(int index) {
    if (index < 0 || index >= static_cast<int>(items_.size())) return;
    const Item& it = items_[index];
    if (it.kind != ItemKind::SubItem && it.kind != ItemKind::SplitItem) return;
    if (it.children.empty()) return;
    if (!visible_) return;
    if (itemRects_.empty() || itemRects_[index].IsEmpty()) {
        pendingExpand_ = index;   // 首帧尚未布局：交给下一次 Layout 展开
        return;
    }
    OpenSubmenu(index);
}

void MenuFlyout::Close() {
    if (!visible_ || closing_) return;
    closing_ = true;
    anim_ = true;
    elapsed_ = 0.0f;
    hotIndex_ = -1;
    pressedIndex_ = -1;
    openSubIndex_ = -1;
    subPendingIndex_ = -1;
    subOpenTimer_ = 0.0f;
    subCloseTimer_ = 0.0f;
}

RectF MenuFlyout::ItemRect(int index) const {
    if (index < 0 || index >= static_cast<int>(itemRects_.size())) return RectF();
    return itemRects_[index];
}

// 是否任何项带图标/勾选槽/子项 chevron（决定整体是否预留前导槽）
static bool AnyIconColumn(const std::vector<MenuFlyout::Item>& items) {
    for (const auto& it : items)
        if (it.hasIcon || it.kind == MenuFlyout::ItemKind::Toggle ||
            it.kind == MenuFlyout::ItemKind::Radio) return true;
    return false;
}

// 单项宽度（文本 + 前导槽 + 子项 chevron + 快捷键）
static float MeasureItemWidth(Renderer& renderer, const MenuFlyout::Item& it,
                              float s, bool iconCol) {
    float tw = 0.0f, th = 0.0f;
    renderer.MeasureText(it.text, 100000.0f, L"Segoe UI",
                         14.0f * s, DWRITE_FONT_WEIGHT_NORMAL, &tw, &th);
    float w = (11.0f + tw + 11.0f) * s;
    if (iconCol) w += (16.0f + 12.0f) * s;
    if (it.kind == MenuFlyout::ItemKind::SubItem || it.kind == MenuFlyout::ItemKind::SplitItem) {
        w += (it.kind == MenuFlyout::ItemKind::SplitItem ? 38.0f : 36.0f) * s;
    } else if (!it.accelerator.empty()) {
        float aw = 0.0f, ah = 0.0f;
        renderer.MeasureText(it.accelerator, 100000.0f, L"Segoe UI", 12.0f * s,
                             DWRITE_FONT_WEIGHT_NORMAL, &aw, &ah);
        w += (24.0f + aw) * s;
    }
    return w;
}

void MenuFlyout::Layout(Renderer& renderer, float scale) {
    scale_ = scale;
    const float s = scale;
    const float maxW = FzMn(kMaxWidth * s, FzMx(0.0f, bounds_.w - 16.0f * s));
    const float maxH = FzMn(kMaxHeight * s, FzMx(0.0f, bounds_.h - 16.0f * s));

    const bool anyIcon = AnyIconColumn(items_);

    float contentW = 0.0f;
    for (const Item& it : items_) {
        if (it.kind == ItemKind::Separator) continue;
        contentW = FzMx(contentW, MeasureItemWidth(renderer, it, s, anyIcon));
    }

    float w = FzMx(FzMx(contentW + 8.0f * s, anchor_.w), kMinWidth * s);
    w = FzMn(w, maxW);

    // 高度：4 + 项数×36 + 分隔线数×3（MenuFlyout.vue estimateFlyoutHeight）
    int itemCount = 0, sepCount = 0;
    for (const Item& it : items_) {
        if (it.kind == ItemKind::Separator) ++sepCount;
        else ++itemCount;
    }
    float h = (4.0f + itemCount * 36.0f + sepCount * 3.0f) * s;
    h = FzMn(FzMx(h, 36.0f * s), maxH);

    if (topAligned_) {
        // 子菜单：顶对齐锚点，向右展开（MenuFlyout.vue getSubmenuStyle）
        float x = anchor_.Right() + kGap * s;
        float y = FzMn(anchor_.y - 4.0f * s, FzMx(bounds_.y + kMargin * s,
                                                   bounds_.Bottom() - kMargin * s - h));
        x = ClampF(x, bounds_.x + kMargin * s,
                   FzMx(bounds_.x + kMargin * s, bounds_.Right() - kMargin * s - w));
        y = ClampF(y, bounds_.y + kMargin * s,
                   FzMx(bounds_.y + kMargin * s, bounds_.Bottom() - kMargin * s - h));
        panel_ = RectF(x, y, w, h);
        above_ = false;
    } else {
        const PopupLayout pl = PlacePopup(placement_, anchor_, w, h, bounds_,
                                          kGap * s, kMargin * s);
        panel_ = pl.rect;
        above_ = pl.above;
    }

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

    // 布局完成后处理"立即展开"请求（首帧锚点尚未测量时由调用方延后到此）
    if (pendingExpand_ >= 0) {
        const int pi = pendingExpand_;
        pendingExpand_ = -1;
        if (visible_ && !closing_ && pi >= 0 &&
            pi < static_cast<int>(items_.size()) && !itemRects_[pi].IsEmpty())
            OpenSubmenu(pi);
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
    busy |= UpdateSubmenu(dt);
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

    const bool anyIcon = AnyIconColumn(items_);
    const float slotW = (kSlot + kSlotGap) * s;

    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        const Item& it = items_[i];
        const RectF r = itemRects_[i];
        if (r.IsEmpty()) continue;

        if (it.kind == ItemKind::Separator) {
            renderer.FillRect(RectF(panel_.x, r.y + 1.0f * s, panel_.w, 1.0f * s), divCol);
            continue;
        }

        const bool subOpen = (openSubIndex_ == i);
        const bool hot = ((hotIndex_ == i) && it.enabled);
        const bool hotChevron = (subHotChevron_ && it.kind == ItemKind::SplitItem);

        // 背景
        if (it.kind == ItemKind::SplitItem) {
            // 分裂项：主区 + chevron 区各自 hover
            if (hot && !hotChevron) {
                const float cw = kSplitChevron * s;
                renderer.FillRoundedRect(
                    RectF(r.x, r.y, FzMx(0.0f, r.w - cw), r.h),
                    kItemRadius * s, subtleHov.WithAlpha(subtleHov.a * fade));
            }
            if (hotChevron || subOpen) {
                const float cw = kSplitChevron * s;
                renderer.FillRoundedRect(
                    RectF(r.Right() - cw, r.y, cw, r.h),
                    kItemRadius * s, subtleHov.WithAlpha(subtleHov.a * fade));
            }
        } else {
            if (hot || subOpen)
                renderer.FillRoundedRect(r, kItemRadius * s, subtleHov.WithAlpha(subtleHov.a * fade));
        }

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
            textX = slotX + slotW;
        }

        // 右侧占位：子项 chevron（24 margin + 12 图标）/ 分裂 chevron 区
        float textRight = r.Right() - kItemPadR * s;
        if (it.kind == ItemKind::SubItem) {
            const float chev = 12.0f * s;
            const float cx = r.Right() - kItemPadR * s - chev;
            DrawFluentIcon(renderer, FluentIcon::ChevronDown, cx, cy - chev * 0.5f, chev,
                           it.enabled ? fgSec : fgDis,
                           -static_cast<float>(3.14159265f) * 0.5f);   // 旋转 -90° → 朝右
            textRight -= (kAccelGap * s + chev);
        } else if (it.kind == ItemKind::SplitItem) {
            const float cw = kSplitChevron * s;
            const float chev = 12.0f * s;
            const float cx = r.Right() - cw * 0.5f - chev * 0.5f;
            const Color chevCol = (subOpen || hotChevron) ? fg : fgSec;
            DrawFluentIcon(renderer, FluentIcon::ChevronDown, cx, cy - chev * 0.5f, chev,
                           it.enabled ? chevCol : fgDis,
                           -static_cast<float>(3.14159265f) * 0.5f);   // 旋转 -90° → 朝右
            // 1×18 分隔线（chevron 区左缘）
            renderer.FillRect(RectF(r.Right() - cw - 0.5f * s, cy - 9.0f * s,
                                    1.0f * s, 18.0f * s), divCol);
            textRight -= (cw + 24.0f * s);
        }

        // 快捷键（右对齐）
        float textMaxW = textRight - textX;
        if (!it.accelerator.empty() && it.kind != ItemKind::SubItem
            && it.kind != ItemKind::SplitItem) {
            float aw = 0.0f, ah = 0.0f;
            renderer.MeasureText(it.accelerator, 100000.0f, kFace, kAccelSize * s,
                                 DWRITE_FONT_WEIGHT_NORMAL, &aw, &ah);
            const float ax = textRight - aw;
            renderer.DrawText(it.accelerator, ax, r.y, FzMx(0.0f, aw + 1.0f), r.h,
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

    // 子菜单（在主菜单裁剪区之外，最上层）
    if (submenu_ && submenu_->IsOpen())
        submenu_->Draw(renderer, theme, scale);
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

bool MenuFlyout::InChevronBox(int index, float x, float y) const {
    if (index < 0 || index >= static_cast<int>(items_.size())) return false;
    if (items_[index].kind != ItemKind::SplitItem) return false;
    const RectF r = itemRects_[index];
    const float cw = kSplitChevron * scale_;
    return r.Contains(x, y) && x >= (r.Right() - cw);
}

bool MenuFlyout::PointInSubmenu(float x, float y) const {
    return submenu_ && submenu_->PanelRect().Contains(x, y);
}

bool MenuFlyout::OnMouseMove(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;

    // 子菜单打开时，优先交给子菜单
    if (openSubIndex_ >= 0 && submenu_ && submenu_->IsOpen()) {
        if (PointInSubmenu(x, y)) {
            submenu_->OnMouseMove(x, y);
            return true;
        }
        const int i = HitTest(x, y);
        subHotChevron_ = InChevronBox(i, x, y);
        if (i >= 0 && i == openSubIndex_ &&
            items_[openSubIndex_].kind == ItemKind::SplitItem && subHotChevron_) {
            hotIndex_ = openSubIndex_;
            return true;   // 停在 chevron 区：保持子菜单 + 保持高亮
        }
        if (i >= 0) {
            // 主菜单内其它位置：延迟关子菜单，继续正常 hover/切换
            hotIndex_ = (i != openSubIndex_ && items_[i].enabled) ? i : -1;
            if (i != openSubIndex_) QueueCloseSubmenu();
            else QueueCloseSubmenu();
            if (hotIndex_ >= 0) {
                const Item& it = items_[hotIndex_];
                if (it.kind == ItemKind::SubItem) QueueOpenSubmenu(hotIndex_);
                else if (it.kind == ItemKind::SplitItem && subHotChevron_) QueueOpenSubmenu(hotIndex_);
            }
            return true;
        }
        Close();   // 完全移出菜单 = light dismiss（主菜单关闭 → 级联子菜单）
        return true;
    }

    const int i = HitTest(x, y);
    hotIndex_ = (i >= 0 && items_[i].enabled) ? i : -1;
    subHotChevron_ = InChevronBox(hotIndex_, x, y);

    // 悬停触发：SubItem 全项 / SplitItem chevron 区
    if (hotIndex_ >= 0) {
        const Item& it = items_[hotIndex_];
        if (it.kind == ItemKind::SubItem) QueueOpenSubmenu(hotIndex_);
        else if (it.kind == ItemKind::SplitItem && subHotChevron_) QueueOpenSubmenu(hotIndex_);
    }
    return true;
}

bool MenuFlyout::OnMouseDown(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;

    if (openSubIndex_ >= 0 && submenu_ && submenu_->IsOpen()) {
        if (PointInSubmenu(x, y)) { submenu_->OnMouseDown(x, y); return true; }
        const int i = HitTest(x, y);
        const bool keepChevron = (i == openSubIndex_ &&
            items_[i].kind == ItemKind::SplitItem && InChevronBox(i, x, y));
        if (!keepChevron) CloseSubmenu(true);   // 点主菜单其它区 → 立即关子菜单
    }

    const int i = HitTest(x, y);
    if (i >= 0 && items_[i].enabled) {
        pressedIndex_ = i;
        hotIndex_ = i;
        subHotChevron_ = InChevronBox(i, x, y);
    } else if (i < 0) {
        Close();   // 点面板外 = light dismiss
    }
    return true;
}

bool MenuFlyout::OnMouseUp(float x, float y) {
    if (!visible_) return false;
    if (anim_) return true;

    if (openSubIndex_ >= 0 && submenu_ && submenu_->IsOpen()) {
        if (PointInSubmenu(x, y)) { submenu_->OnMouseUp(x, y); return true; }
    }

    const int i = HitTest(x, y);
    const int pressed = pressedIndex_;
    pressedIndex_ = -1;
    if (pressed >= 0 && pressed == i && items_[i].enabled) {
        Item& it = items_[i];
        if (it.kind == ItemKind::SplitItem && InChevronBox(i, x, y)) {
            OpenSubmenu(i);   // chevron 区：展开子菜单，不触发选择
        } else if (it.kind == ItemKind::SubItem) {
            OpenSubmenu(i);   // SubItem 点击：展开
        } else if (it.kind == ItemKind::Toggle) {
            it.checked = !it.checked;
            if (onInvoked_) onInvoked_(i);
            Close();
        } else if (it.kind == ItemKind::Radio) {
            SetItemChecked(i, true);
            if (onInvoked_) onInvoked_(i);
            Close();
        } else {
            if (onInvoked_) onInvoked_(i);
            Close();
        }
    }
    return true;
}

void MenuFlyout::OnMouseLeave() {
    if (!visible_) return;
    hotIndex_ = -1;
    subHotChevron_ = false;
    QueueCloseSubmenu();
}

bool MenuFlyout::OnKeyDown(int vk) {
    if (!visible_) return false;
    if (anim_) return true;
    if (vk == VK_ESCAPE) {
        if (openSubIndex_ >= 0) { CloseSubmenu(true); return true; }
        Close();
        return true;
    }
    return false;
}

// ============================================================
// 子菜单状态机
// ============================================================
void MenuFlyout::OpenSubmenu(int index) {
    if (index < 0 || index >= static_cast<int>(items_.size())) return;
    const Item& it = items_[index];
    if (it.kind != ItemKind::SubItem && it.kind != ItemKind::SplitItem) return;
    if (it.children.empty()) return;
    if (openSubIndex_ == index) return;

    subPendingIndex_ = -1;
    subOpenTimer_ = 0.0f;
    if (!submenu_) submenu_ = std::make_unique<MenuFlyout>();
    submenu_->Clear();
    for (const Item& c : it.children) submenu_->ImportItem(c);
    submenu_->SetBounds(bounds_);
    submenu_->SetTopAligned(true);
    submenu_->SetPlacement(PopupPlacement::Right);
    submenu_->SetAnchor(itemRects_[index]);
    submenu_->SetInvokedCallback([this](int ci) {
        if (onInvoked_) onInvoked_(openSubIndex_ * 1000 + ci);   // 复合索引：父*1000+子
        Close();
    });
    submenu_->SetClosedCallback([this] { openSubIndex_ = -1; });
    submenu_->Show();
    openSubIndex_ = index;
}

void MenuFlyout::CloseSubmenu(bool immediate) {
    subPendingIndex_ = -1;
    subOpenTimer_ = 0.0f;
    if (openSubIndex_ < 0) return;
    if (immediate && submenu_) {
        submenu_->Hide();
        openSubIndex_ = -1;
    } else {
        subCloseTimer_ = kSubDelay;
    }
}

void MenuFlyout::QueueOpenSubmenu(int index) {
    if (openSubIndex_ == index) return;
    subPendingIndex_ = index;
    subOpenTimer_ = kSubDelay;
    subCloseTimer_ = 0.0f;
}

void MenuFlyout::QueueCloseSubmenu(bool immediate) {
    if (openSubIndex_ < 0) return;
    if (immediate) { subCloseTimer_ = 0.0f; return; }
    subCloseTimer_ = kSubDelay;
    subOpenTimer_ = 0.0f;
}

bool MenuFlyout::UpdateSubmenu(float dt) {
    bool busy = false;

    // 打开延迟
    if (subPendingIndex_ >= 0 && subOpenTimer_ > 0.0f) {
        subOpenTimer_ -= dt;
        if (subOpenTimer_ <= 0.0f) {
            const int i = subPendingIndex_;
            subPendingIndex_ = -1;
            subCloseTimer_ = 0.0f;
            OpenSubmenu(i);
        }
        busy = true;
    }

    // 关闭延迟
    if (openSubIndex_ >= 0 && subCloseTimer_ > 0.0f) {
        subCloseTimer_ -= dt;
        if (subCloseTimer_ <= 0.0f) {
            if (submenu_) submenu_->Hide();
            openSubIndex_ = -1;
        }
        busy = true;
    }

    // 子菜单自身动画
    if (submenu_ && submenu_->IsOpen())
        busy |= submenu_->Update(dt);

    return busy;
}

} // namespace ModernDesign