#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// NavigationView — 导航窗格 / 侧边栏（WinUIonWeb 规格）
// 结构（左面板）：
//   hamburger 行(40)  + 菜单项列表(每项 36, margin2)  + 底部 settings
//   选中项左侧 accent indicator(3x16 圆角2)
//   hamburger 点击 → 面板 320 <-> 48 折叠动画（compact）
// bounds_ 设为整个窗口客户区（DIP*scale 的最终坐标）。
//   左侧面板宽 = PaneWidthDip()*scale；右侧内容区由 caller 绘制。
// ============================================================
class NavigationView {
public:
    struct Item {
        std::wstring label;
        int    icon = 0;          // 0 home, 1 grid, 2 profile, 3 gear
        bool   isSettings = false; // 固定在底部
    };

    NavigationView() = default;

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    // 由 caller 在 OnLayout 设置 DPI 缩放（内部尺寸均 *scale）
    void SetScale(float s) { sc_ = s; }

    void SetPaneTitle(const std::wstring& t) { paneTitle_ = t; }
    void AddItem(const Item& it) { items_.push_back(it); }
    void SetSettings(const std::wstring& label) {
        Item it; it.label = label; it.icon = 3; it.isSettings = true;
        items_.push_back(it);
    }

    int  SelectedIndex() const { return selected_; }
    void SetSelectedIndex(int i) { if (i >= 0 && i < (int)items_.size()) selected_ = i; }
    void SetSelectionCallback(std::function<void(int)> cb) { onSelection_ = std::move(cb); }

    bool IsCompact() const { return compact_; }
    void TogglePane() { compact_ = !compact_; }

    // 面板宽度（DIP，未缩放；caller *scale 得最终坐标）
    float PaneWidthDip() const;

    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    RectF HamburgerRect() const;
    RectF ItemRect(int i) const;

    std::wstring paneTitle_ = L"Modern Design";
    std::vector<Item> items_;
    int  selected_ = 0;
    bool compact_ = false;
    RectF bounds_;
    std::function<void(int)> onSelection_;

    float sc_ = 1.0f;
    float paneT_ = 1.0f;        // 1=展开(320) 0=紧凑(48)
    bool  hot_ = false;
    int   hotIndex_ = -1;
    float hotT_ = 0.0f;
    bool  hamburgerHot_ = false;
    float hamburgerT_ = 0.0f;

    static constexpr float kOpenW    = 320.0f;
    static constexpr float kCompactW = 48.0f;
    static constexpr float kPanePad  = 4.0f;
    static constexpr float kHamH     = 36.0f;
    static constexpr float kHamRowH  = 40.0f;
    static constexpr float kItemH    = 36.0f;
    static constexpr float kItemM    = 2.0f;
    static constexpr float kItemPitch= 40.0f;
};

} // namespace ModernDesign
