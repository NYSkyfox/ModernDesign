#pragma once

#include "Geometry.h"
#include "controls/PopupPlacement.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "utils/FluentIcons.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// MenuFlyout — 菜单浮出控件（规格来源：WinUIonWeb MenuFlyout.vue）
//
// 面板：min-w 96 / max-w min(320, 窗口宽-16)、max-h 600、padding 2 0
//       圆角 8、描边 1px flyout-border、阴影 0 8 24 rgba(0,0,0,.14)
// 菜单项：min-h 32、margin 2 4、padding 4 11 5 11、圆角 4、字号 14
//         悬停 --subtle-secondary；禁用 text-disabled
//         图标/勾选槽 16×16 + 右间距 12
//         快捷键文本 12/16、次要色、右对齐、左间距 24
// 分隔线：3px 带 + 1px 线（上下各 1px 余量），颜色 stroke-divider
// 动画：打开 250ms cubic-bezier(0,0,0,1)（从锚点侧揭示）+ 83ms 淡入；
//       关闭 83ms 线性淡出（MenuFlyout.vue 默认的 is-closing）
// 交互：点面板外关闭（light dismiss）、Esc 关闭
//
// 暂未实现：SubItem / SplitItem（二级菜单与分裂按钮）
// ============================================================
class MenuFlyout {
public:
    enum class ItemKind { Item, Separator, Toggle, Radio };

    struct Item {
        std::wstring text;
        std::wstring accelerator;
        FluentIcon   icon = FluentIcon::Home;
        bool         hasIcon = false;
        ItemKind     kind = ItemKind::Item;
        bool         enabled = true;
        bool         checked = false;   // Toggle / Radio 用
    };

    MenuFlyout() = default;

    void SetBounds(const RectF& windowBounds) { bounds_ = windowBounds; }
    void SetAnchor(const RectF& anchor) { anchor_ = anchor; }
    void SetPlacement(PopupPlacement p) { placement_ = p; }

    int  AddItem(const std::wstring& text, const std::wstring& accelerator = std::wstring(),
                 bool enabled = true);
    int  AddItemWithIcon(const std::wstring& text, FluentIcon icon,
                         const std::wstring& accelerator = std::wstring(),
                         bool enabled = true);
    int  AddToggle(const std::wstring& text, bool checked,
                   const std::wstring& accelerator = std::wstring());
    int  AddRadio(const std::wstring& text, bool checked,
                  const std::wstring& accelerator = std::wstring());
    void AddSeparator();
    void Clear();

    void SetItemEnabled(int index, bool enabled);
    void SetItemChecked(int index, bool checked);
    bool IsItemChecked(int index) const;

    int  CheckedToggleIndex() const;     // -1 = 无
    int  CheckedRadioIndex() const;      // -1 = 无
    void SetInvokedCallback(std::function<void(int)> cb) { onInvoked_ = std::move(cb); }
    void SetClosedCallback(std::function<void()> cb) { onClosed_ = std::move(cb); }

    void Show();
    void Hide();
    bool IsOpen() const { return visible_; }
    bool IsAnimating() const { return anim_; }

    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    bool OnMouseMove(float x, float y);
    bool OnMouseDown(float x, float y);
    bool OnMouseUp(float x, float y);
    void OnMouseLeave();
    bool OnKeyDown(int vk);

    const RectF& PanelRect() const { return panel_; }
    int ItemCount() const { return static_cast<int>(items_.size()); }

private:
    void Layout(Renderer& renderer, float scale);
    void Close();
    int  HitTest(float x, float y) const;          // 返回 item 索引，-1 = 无
    RectF ItemRect(int index) const;

    RectF bounds_;
    RectF anchor_;
    RectF panel_;
    PopupPlacement placement_ = PopupPlacement::Bottom;

    std::vector<Item> items_;
    std::vector<RectF> itemRects_;   // 与 items_ 一一对应（分隔线也有）

    bool  visible_ = false;
    bool  closing_ = false;
    bool  anim_ = false;
    float animT_ = 0.0f;
    float elapsed_ = 0.0f;
    bool  above_ = false;
    int   hotIndex_ = -1;
    int   pressedIndex_ = -1;

    std::function<void(int)> onInvoked_;
    std::function<void()>    onClosed_;

    // ---- 规格常量（DIP）----
    static constexpr float kMinWidth    = 96.0f;
    static constexpr float kMaxWidth    = 320.0f;
    static constexpr float kMaxHeight   = 600.0f;
    static constexpr float kPadV        = 2.0f;    // 面板上下 padding
    static constexpr float kItemH       = 32.0f;   // min-height
    static constexpr float kItemM       = 2.0f;    // 上下 margin
    static constexpr float kItemPadL    = 11.0f;
    static constexpr float kItemPadR    = 11.0f;
    static constexpr float kItemRadius  = 4.0f;
    static constexpr float kSepH        = 3.0f;
    static constexpr float kSlot        = 16.0f;   // 图标/勾选槽
    static constexpr float kSlotGap     = 12.0f;
    static constexpr float kAccelSize   = 12.0f;
    static constexpr float kAccelLine   = 16.0f;
    static constexpr float kAccelGap    = 24.0f;
    static constexpr float kFontSize    = 14.0f;
    static constexpr float kRadius      = 8.0f;
    static constexpr float kGap         = 6.0f;
    static constexpr float kMargin      = 8.0f;
    static constexpr float kShadowY     = 8.0f;
    static constexpr float kShadowBlur  = 24.0f;
    static constexpr float kShadowA     = 0.14f;
    static constexpr float kOpenDur     = 0.250f;
    static constexpr float kFadeDur     = 0.083f;
};

} // namespace ModernDesign