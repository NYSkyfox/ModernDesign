#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <string>
#include <functional>

namespace ModernDesign {
namespace Controls {

// ============================================================
// Button — 按钮控件
// 变体：Standard / Accent / Subtle
// 状态：Normal / Hover / Pressed / Disabled
// ============================================================

enum class ButtonVariant {
    Standard,
    Accent,
    Subtle
};

class Button {
public:
    Button();
    ~Button();

    // ---- 配置 ----
    void SetText(const std::wstring& text);
    const std::wstring& GetText() const { return text_; }

    void SetVariant(ButtonVariant variant);
    ButtonVariant GetVariant() const { return variant_; }

    void SetEnabled(bool enabled);
    bool IsEnabled() const { return enabled_; }

    // ---- 布局 ----
    void SetBounds(const RectF& bounds);
    const RectF& GetBounds() const { return bounds_; }

    // ---- 状态 ----
    bool IsHot() const { return hot_; }
    bool IsPressed() const { return pressed_; }

    // ---- 输入事件 ----
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    // ---- 动画 ----
    // 返回 true 表示仍在动画中（需要继续重绘）
    bool Update(float dt);

    // ---- 绘制 ----
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    // ---- 回调 ----
    using ClickCallback = std::function<void()>;
    void SetClickCallback(ClickCallback cb) { onClick_ = cb; }

private:
    // 计算当前应绘制的状态
    int GetCurrentState() const;

    // 成员
    std::wstring text_;
    ButtonVariant variant_ = ButtonVariant::Standard;
    bool enabled_ = true;

    RectF bounds_;

    // 交互状态
    bool hot_ = false;       // 鼠标悬停
    bool pressed_ = false;   // 鼠标按下

    // 动画进度（0~1）
    float hoverT_ = 0.0f;    // hover 渐入
    float pressT_ = 0.0f;    // press 渐入
    float revealT_ = 0.0f;   // 首次显示渐入

    ClickCallback onClick_;
};

}} // namespace ModernDesign::Controls