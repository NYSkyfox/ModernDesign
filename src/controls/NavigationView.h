#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "utils/FluentIcons.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// NavigationView — 导航窗格（严格对齐 WinUI 3 / WinUIonWeb 规格）
//
// 显示模式：
//   Left         展开/折叠内联（320 <-> 48，内容区跟随位移）
//   LeftCompact  常驻 48 图标栏，展开时以覆盖方式叠在内容上方
//   LeftMinimal  不占位，仅左上角汉堡按钮，展开时覆盖
//   Top          顶部横向导航（图标项 48x48 + 底部 16x3 指示条）
//
// 左面板结构（DIP，全部来自 NavigationView.vue）：
//   command row 40 —— hamburger 40x36(radius4) + PaneTitle(14/20,600)
//   ItemHeader   40 —— padding 0 16, 14/20 w600, text-secondary
//   Item         36 + margin 2 0（行距 40），padding 0 12，radius 4
//                    图标 16x16 + margin-right 16；label 14/20
//                    状态：hover=subtle-secondary、pressed=subtle-tertiary、
//                          selected=subtle-secondary（文字仍为 text-primary）
//   分组         父项右侧 chevron（40x36，rotate 180 = 展开）
//                子项 padding-left 44，indicator left 36
//   Separator     1px，margin 3 0 4，stroke-divider
//   Settings      固定底部
//   indicator     3x16 radius2 accent（顶层 left 4）
//
// 内容区：radius 8 0 0 0 + 上/左 1px card-stroke，
//         底色 --NavigationViewContentBackground（#F9F9F9 / #282828）
//
// 动画契约：
//   Inline   open 200 / close 200，cubic-bezier(0, 0.35, 0.15, 1)
//   Overlay  open 350 / close 120，cubic-bezier(0.1, 0.9, 0.2, 1)
// ============================================================
class NavigationView {
public:
    enum class DisplayMode { Left, LeftCompact, LeftMinimal, Top };
    enum class ItemKind { Item, Header, Separator, Settings };

    struct Item {
        std::wstring label;
        FluentIcon icon = FluentIcon::Home;   // 官方 Fluent 图标
        ItemKind kind = ItemKind::Item;
        bool   isChild = false;      // 分组子项（padding-left 44 / indicator 36）
        int    group = -1;           // 所属分组父项索引（-1 = 顶层）
        bool   expandable = false;   // 分组父项（右侧 chevron）
        bool   expanded = true;
        bool   selectsOnInvoked = true; // false = 点击只切换展开
        bool   enabled = true;
    };

    NavigationView() = default;

    // ---- 基本 ----
    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }
    void SetScale(float s) { sc_ = s; }

    void SetPaneTitle(const std::wstring& t) { paneTitle_ = t; }
    void SetDisplayMode(DisplayMode m);
    DisplayMode GetDisplayMode() const { return mode_; }

    // ---- 菜单内容 ----
    int  AddItem(const Item& it);                              // 返回索引
    int  AddHeader(const std::wstring& text);
    int  AddSeparator();
    int  AddGroup(const std::wstring& label, FluentIcon icon,
                  const std::vector<Item>& children, bool expanded = true);
    void SetSettings(const std::wstring& label);
    void ClearItems();

    int  ItemCount() const { return static_cast<int>(items_.size()); }
    const std::wstring& ItemLabel(int i) const { return items_[i].label; }

    int  SelectedIndex() const { return selected_; }
    void SetSelectedIndex(int i);
    void SetSelectionCallback(std::function<void(int)> cb) { onSelection_ = std::move(cb); }

    bool IsGroupExpanded(int groupIndex) const;
    void SetGroupExpanded(int groupIndex, bool expanded);

    // ---- 面板状态 ----
    bool IsCompact() const { return compact_; }
    void SetCompact(bool c);
    void TogglePane();
    bool IsPaneOpen() const { return paneT_ > 0.5f; }

    // ---- 几何 ----
    // 面板可视宽度（DIP，动画中：48 <-> 320）
    float PaneWidthDip() const;
    // 内容区左侧为面板预留的宽度（DIP）
    float RailWidthDip() const;
    // 右侧内容区（最终像素坐标，已乘 scale）
    RectF ContentRect() const;

    // ---- 输入 / 动画 / 绘制 ----
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    struct Placed {
        RectF rect;          // 空矩形 = 未布局（不可见/不可点）
        float alpha = 1.0f;  // 分组折叠时的淡出系数
    };

    void  SyncGroups();
    void  RebuildLayout();
    int   GroupParentOf(int i) const;
    float GroupProgress(int groupIndex) const;
    int   HitItem(float x, float y) const;
    void  ActivateItem(int i, float x);
    void  StartPaneAnim(bool opening);
    float IndicatorTargetX() const;
    float IndicatorTargetY() const;
    RectF HamburgerRect() const;
    RectF TopBarRect() const;
    RectF PlacedRect(int i) const;

    std::wstring paneTitle_ = L"Modern Design";
    std::vector<Item> items_;
    int selected_ = 0;

    DisplayMode mode_ = DisplayMode::Left;
    bool  compact_ = false;
    RectF bounds_;
    std::function<void(int)> onSelection_;

    float sc_ = 1.0f;

    // 面板动画（paneT_：0=收起/紧凑，1=展开）
    float paneT_ = 1.0f;
    float paneFrom_ = 1.0f;
    float paneElapsed_ = 0.0f;
    float paneDur_ = 0.2f;
    bool  paneOpen_ = true;
    bool  paneAnim_ = false;

    // 分组展开动画（每项一个进度）
    std::vector<float> groupT_;

    // indicator 位置平滑（官方：定时 200ms + cubic-bezier(0,0.35,0.15,1) = EaseNavInline）
    float indX_ = -1.0f;
    float indY_ = -1.0f;
    float indFromX_ = 0.0f;
    float indFromY_ = 0.0f;
    float indElapsed_ = 0.0f;
    bool  indAnim_ = false;

    // 交互状态
    bool hot_ = false;
    int  hotIndex_ = -1;
    bool hamburgerHot_ = false;
    bool hamburgerPressed_ = false;
    int  pressIndex_ = -1;
    float hotT_ = 0.0f;        // hover 淡入进度
    float hamburgerT_ = 0.0f;

    std::vector<Placed> placed_;

    // ---- WinUIonWeb 精确规格（DIP）----
    static constexpr float kOpenW      = 320.0f;  // OpenPaneLength
    static constexpr float kCompactW   = 48.0f;   // CompactPaneLength
    static constexpr float kPanePad    = 4.0f;    // 面板 padding 4px
    static constexpr float kHamW       = 40.0f;   // hamburger 宽
    static constexpr float kHamH       = 36.0f;   // hamburger 高
    static constexpr float kHamRowH    = 40.0f;   // command row（hamburger 独占一行）
    static constexpr float kItemH      = 36.0f;   // 菜单项高
    static constexpr float kItemM      = 2.0f;    // 菜单项上下 margin
    static constexpr float kItemPadX   = 12.0f;   // 菜单项左右 padding
    static constexpr float kIconSize   = 16.0f;   // 图标
    static constexpr float kIconGap    = 16.0f;   // 图标右间距
    static constexpr float kChildPadL  = 44.0f;   // 子项 padding-left
    static constexpr float kHeaderH    = 40.0f;   // ItemHeader
    static constexpr float kHeaderPadX = 16.0f;
    static constexpr float kSepH       = 1.0f;    // Separator
    static constexpr float kSepMT      = 3.0f;
    static constexpr float kSepMB      = 4.0f;
    static constexpr float kIndLen     = 16.0f;   // INDICATOR_SIZE
    static constexpr float kIndThick   = 3.0f;
    static constexpr float kIndLeft    = 4.0f;    // 顶层（相对面板）
    static constexpr float kIndChildL  = 36.0f;   // 子项（相对面板）
    static constexpr float kChevW      = 40.0f;   // 分组 chevron 盒
    static constexpr float kChevRH     = 18.0f;   // chevron 中心距行右边缘
    static constexpr float kRadius     = 4.0f;
    static constexpr float kTopBarH    = 48.0f;
    static constexpr float kContentR   = 8.0f;    // 内容区左上圆角
};

} // namespace ModernDesign