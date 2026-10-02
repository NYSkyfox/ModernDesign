#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <algorithm>
#include <functional>
#include <string>

namespace ModernDesign {

// ============================================================
// Expander — 可折叠容器
//
// 规格来源：WinUIonWeb/src/components/ExpanderBase.vue + styles/theme.css
//
// 结构：
//   外框 (1px border / radius 4)
//    ├ Header  min-height 48, padding 0 16, gap 16
//    │   ├ [HeaderIcon 20×20, margin 0 20 0 2]
//    │   ├ HeaderText (14/20) + Description (12/16)   ← flex column，整体垂直居中
//    │   ├ [HeaderControls]  右侧, gap 8
//    │   └ Chevron 32×32 (radius 4, hover 底色)
//    └ Content  padding 16, min-height 48, radius 0 0 3 3
//
// 展开态：Header 圆角 → 4 4 0 0（Up 方向相反），Header/Content 之间 1px 分隔线
// 动画：0.2s cubic-bezier(0,0,0,1)（= x³ 求根后的 y = 3t²-2t³）
//
// 内容通过回调注册：caller 在内容区内自行绘制 / 更新 / 处理输入。
// ============================================================
class Expander {
public:
    // 展开方向：Down = 内容在 Header 下方；Up = 内容在 Header 上方
    enum class Direction { Down, Up };
    // HorizontalContentAlignment / VerticalContentAlignment
    enum class HAlign { Left, Center, Right, Stretch };
    enum class VAlign { Top, Center, Bottom, Stretch };

    // box 为控件的绘制区域（DIP 已乘 scale）
    using DrawFn = std::function<void(Renderer&, const Theme&, float scale, const RectF& box)>;
    using UpdateFn = std::function<bool(float dt)>;
    using InputFn = std::function<void(float x, float y)>;

    Expander() = default;

    // ---- 文本 ----
    void SetHeader(const std::wstring& t) { header_ = t; }
    const std::wstring& Header() const { return header_; }
    void SetDescription(const std::wstring& d) { description_ = d; }
    const std::wstring& Description() const { return description_; }

    // ---- 状态 ----
    void SetIsExpanded(bool e) { isExpanded_ = e; }
    bool IsExpanded() const { return isExpanded_; }
    void SetExpandedCallback(std::function<void(bool)> cb) { onExpanded_ = std::move(cb); }

    // ---- 规格 / 外观 ----
    void SetDirection(Direction d) { dir_ = d; }
    Direction GetDirection() const { return dir_; }
    void SetPadding(float dip) { padding_ = dip; }
    float Padding() const { return padding_; }
    void SetHeaderHeight(float dip) { headerH_ = std::max(0.0f, dip); }
    void SetContentAlignment(HAlign h, VAlign v) { alignH_ = h; alignV_ = v; }

    // ---- HeaderIcon（20×20 图标槽）----
    void SetHeaderIcon(DrawFn fn) { iconDraw_ = std::move(fn); }

    // ---- HeaderControls（Header 右侧控件槽）----
    // widthDip 为控件区宽度（<=0 表示不启用）
    void SetHeaderControls(float widthDip, DrawFn fn) {
        controlsW_ = std::max(0.0f, widthDip);
        controlsDraw_ = std::move(fn);
    }
    void SetHeaderControlsInputFn(InputFn down, InputFn up, InputFn move, InputFn leave) {
        controlsDown_ = std::move(down);
        controlsUp_ = std::move(up);
        controlsMove_ = std::move(move);
        controlsLeave_ = std::move(leave);
    }

    // ---- 布局 ----
    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }
    void SetScale(float s) { lastScale_ = s; }

    // 展开后的内容区高度（DIP，含 padding；下限 kMinContentHeight）
    void SetContentHeight(float h) { contentH_ = h; }
    float ContentHeight() const { return EffectiveContentHeight(); }
    float HeaderHeight() const { return headerH_; }
    // 当前可见高度（DIP）= header + 动画进度×内容高度。供流布局补位。
    float VisibleHeightDip() const {
        return headerH_ + EffectiveContentHeight() * EasedProgress();
    }
    // 内容自身自然尺寸（DIP；0 = 撑满），配合 SetContentAlignment 生效
    void SetContentSize(float wDip, float hDip) {
        contentNatW_ = wDip;
        contentNatH_ = hDip;
    }

    // Header 区域（供 caller 布局参考；随方向锚定，与动画无关）
    RectF HeaderBox(float scale) const { return HeaderRect(scale); }
    // HeaderControls 区域（供 caller 放置子控件）
    RectF HeaderControlsBox(float scale) const { return ControlsRect(scale); }

    // ---- 内容注册 ----
    void SetContentDrawFn(DrawFn fn) { contentDraw_ = std::move(fn); }
    void SetContentUpdateFn(UpdateFn fn) { contentUpdate_ = std::move(fn); }
    void SetContentInputFn(InputFn down, InputFn up, InputFn move, InputFn leave) {
        contentDown_ = std::move(down);
        contentUp_ = std::move(up);
        contentMove_ = std::move(move);
        contentLeave_ = std::move(leave);
    }

    // ---- 输入 ----
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    // ---- 动画 / 绘制 ----
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    // ---- 规格常量（WinUIonWeb 实测值）----
    static constexpr float kMinHeaderHeight  = 48.0f;  // header min-height
    static constexpr float kDefaultPadding   = 16.0f;  // padding 0 16 / content 16
    static constexpr float kMinContentHeight = 48.0f;  // content min-height
    static constexpr float kMarginBottom     = 4.0f;   // margin-bottom
    static constexpr float kAnimDuration     = 0.2f;   // 0.2s cubic-bezier(0,0,0,1)

private:
    float EffectiveContentHeight() const {
        return std::max(std::max(contentH_, 0.0f), kMinContentHeight);
    }
    RectF HeaderRect(float scale) const;
    RectF ControlsRect(float scale) const;
    float EasedProgress() const;

    // ---- 内容 / 状态 ----
    std::wstring header_ = L"Expand";
    std::wstring description_;
    bool isExpanded_ = false;
    std::function<void(bool)> onExpanded_;

    // ---- 规格 ----
    Direction dir_ = Direction::Down;
    float padding_ = kDefaultPadding;
    float headerH_ = kMinHeaderHeight;
    HAlign alignH_ = HAlign::Stretch;
    VAlign alignV_ = VAlign::Stretch;

    RectF bounds_;
    float lastScale_ = 1.0f;

    float expandT_ = 0.0f;   // 0..1 线性时间进度（绘制时过 cubic-bezier）
    bool hot_ = false;       // 指针在 Header 上
    bool pressed_ = false;   // 在 Header 空白处按下
    float hotT_ = 0.0f;
    float pressT_ = 0.0f;

    float contentH_ = 0.0f;    // 内容区高度（含 padding）
    float contentNatW_ = 0.0f; // 内容自身自然宽（0 = 撑满）
    float contentNatH_ = 0.0f; // 内容自身自然高（0 = 撑满）

    float controlsW_ = 0.0f;

    DrawFn iconDraw_;
    DrawFn controlsDraw_;
    DrawFn contentDraw_;
    UpdateFn contentUpdate_;
    InputFn contentDown_, contentUp_, contentMove_, contentLeave_;
    InputFn controlsDown_, controlsUp_, controlsMove_, controlsLeave_;
};

} // namespace ModernDesign