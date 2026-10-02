#pragma once
#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "utils/FluentIcons.h"
#include <string>
#include <functional>
namespace ModernDesign {
// ============================================================
// SettingsCard — 设置卡片（WinUI SettingsCard）
//
// 规格来源：WinUIonWeb SettingsCard.vue（照抄）
//   - 外框 1px --card-stroke、圆角 4、margin-bottom 4、min-height 68
//   - 内层 padding 16、flex space-between、gap 16、align center
//   - Header 区（左）：HeaderIcon 20×20（margin 0 20 0 2）
//       + Header 14/20（primary）
//       + Description 12/16（secondary）
//   - Content 区（右）：默认 slot，flex align center、gap 8
//   - ActionIcon：右侧 13px chevron（secondary），margin-left -2
//   - clickable：hover→fill --ctrl-fill-secondary + border --ctrl-border
//                （light 下边线 accent / dark 上边线 accent）
//                active→fill --ctrl-fill-tertiary、文字/图标→secondary
//   - 背景 --card-bg（浅 0.70 白 / 深 0.05 白）
//
// 容器语义：本控件负责背景/边框/Header/图标/描述/右侧 ActionIcon/
//   hover 状态；右侧 Content 区用 ContentRect() 给出，由调用方放置
//   子控件（ToggleSwitch / ComboBox 等）并自行 Update/Draw。
//   若只需纯文本右侧内容，用 SetContent() 显示（右对齐）。
// ============================================================
class SettingsCard {
public:
    SettingsCard() = default;
    ~SettingsCard() = default;
    // ---- 配置 ----
    void SetHeader(const std::wstring& t) { header_ = t; }
    const std::wstring& GetHeader() const { return header_; }
    void SetDescription(const std::wstring& t) { description_ = t; }
    const std::wstring& GetDescription() const { return description_; }
    // 右侧纯文本内容（可选；空则不画，ContentRect 仍可用于放子控件）
    void SetContent(const std::wstring& t) { content_ = t; }
    const std::wstring& GetContent() const { return content_; }
    // Header 图标（FluentIcon）；kNone 表示无
    static constexpr int kNoIcon = -1;
    void SetHeaderIcon(int iconId) { headerIcon_ = iconId; }
    bool HasHeaderIcon() const { return headerIcon_ != kNoIcon; }
    // 是否可点击（决定 hover/active 态与 cursor）
    void SetClickable(bool clickable) { clickable_ = clickable; }
    bool IsClickable() const { return clickable_; }
    // 是否显示右侧 ActionIcon（chevron 右向）
    void SetActionIconVisible(bool visible) { actionIcon_ = visible; }
    bool IsActionIconVisible() const { return actionIcon_; }
    // 固定高度（DIP）；0 = 自适应（max(68, 内容)）
    void SetFixedHeight(float h) { fixedH_ = h; }
    // 右侧子控件固有宽度（DIP）：用于 space-between 布局，避免与 Header 文字重叠
    void SetContentWidth(float w) { contentW_ = w; }
    // ---- 布局 ----
    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }
    // 计算所需高度（DIP，不含 scale）：max(68, 16 + 文字区高 + 16)
    float MeasureHeight() const { return 68.0f; }
    // 右侧 Content 区（DIP，已缩放坐标）：供调用方放子控件
    RectF ContentRect(float scale) const;
    // ---- 状态 ----
    bool IsHot() const { return hot_; }
    bool IsPressed() const { return pressed_; }
    // ---- 输入 ----
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);
    // ---- 动画 ----
    bool Update(float dt);
    // ---- 绘制 ----
    void Draw(Renderer& renderer, const Theme& theme, float scale);
    // ---- 回调 ----
    using ClickCallback = std::function<void()>;
    void SetClickCallback(ClickCallback cb) { onClick_ = std::move(cb); }
private:
    std::wstring header_;
    std::wstring description_;
    std::wstring content_;
    int headerIcon_ = kNoIcon;
    bool clickable_ = false;
    bool actionIcon_ = false;
    float fixedH_ = 0.0f;
    float contentW_ = 0.0f;
    RectF bounds_;
    bool hot_ = false;
    bool pressed_ = false;
    float hoverT_ = 0.0f;
    float pressT_ = 0.0f;
    ClickCallback onClick_;
    // 常量（DIP）
    static constexpr float kRadius = 4.0f;
    static constexpr float kPad = 16.0f;
    static constexpr float kGap = 16.0f;      // header 与 content 之间
    static constexpr float kIconSize = 20.0f;
    static constexpr float kIconGap = 20.0f;  // icon 与文字之间（margin 右）
    static constexpr float kTitleFS = 14.0f;
    static constexpr float kDescFS = 12.0f;
    static constexpr float kActionFS = 13.0f;
    static constexpr float kMinH = 68.0f;
};

} // namespace ModernDesign
