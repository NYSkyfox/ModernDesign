#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <string>
#include <functional>

namespace ModernDesign {
namespace Controls {

// ============================================================
// HyperlinkButton — 超链接按钮
//
// 规格来源：WinUIonWeb HyperlinkButton.vue
//   - 无背景、无描边；文字 accent 色、无下划线
//   - min-height 32、圆角 4（ControlCornerRadius）、字号 14
//   - padding 5 11 6 11（水平 11）
//   - hover  → subtle-secondary 背景，文字保持 accent
//   - pressed→ subtle-tertiary 背景，文字保持 accent
//   - disabled→ accent-disabled（≈ text-disabled）
//   - 宽度贴合内容（inline）
//
// 与普通 Button 的 Subtle 变体区别：文字恒为 accent 色（hover/pressed 不变色）。
// ============================================================
class HyperlinkButton {
public:
    HyperlinkButton() = default;
    ~HyperlinkButton() = default;

    // ---- 配置 ----
    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return enabled_; }
    void SetNavigateUri(const std::wstring& uri) { navigateUri_ = uri; }
    const std::wstring& GetNavigateUri() const { return navigateUri_; }

    // ---- 布局 ----
    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    // 自适应宽度（DIP）：文本宽 + 左右 padding；供调用方给 bounds 用
    float MeasureWidth(Renderer& renderer, float scale) const;

    // ---- 状态 ----
    bool IsHot() const { return hot_; }
    bool IsPressed() const { return pressed_; }

    // ---- 输入事件 ----
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
    std::wstring text_;
    std::wstring navigateUri_;
    bool enabled_ = true;
    RectF bounds_;

    bool hot_ = false;
    bool pressed_ = false;
    float hoverT_ = 0.0f;
    float pressT_ = 0.0f;
    float revealT_ = 0.0f;

    ClickCallback onClick_;

    static constexpr float kRadius = 4.0f;
    static constexpr float kFontSize = 14.0f;
    static constexpr float kPadX = 11.0f;
};

}} // namespace ModernDesign::Controls