#pragma once
// ============================================================
// DemoPage — 演示页面基类（解耦架构）
//
// 每个页面是一个独立类（一个文件），自治负责：
//   布局 / 动画推进 / 绘制 / 鼠标输入 / 主题变化
// 通过 owner_（DemoWindow*）访问全局资源：
//   导航(nav_)、浮层(flyout_/menu_/tip_)、几何(ContX/ContW/PageTop)、
//   主题(GetTheme)、重绘(Invalidate)、DPI(DpiScale)。
//
// 页面不再直接持有 DemoWindow 的私有控件；跨页资源（弹窗、浮层、
// 触发按钮）由 DemoWindow 统一持有并路由。
// ============================================================
#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "Geometry.h"
#include <string>

class DemoWindow;   // 前置声明

namespace ModernDesign::Demo {

// 共享布局常量（DIP）
constexpr float kRowHeight = 32.0f;
constexpr float kRowGap = 6.0f;
constexpr float kGroupGap = 26.0f;

class DemoPage {
public:
    explicit DemoPage(DemoWindow* owner) : owner_(owner) {}
    virtual ~DemoPage() = default;

    // 页面标题（用于 DrawPageHeader）
    virtual std::wstring Title() const = 0;

    // 首次绑定（控件构造后、布局前调用一次）
    virtual void Bind() {}
    // 每帧布局（DIP 坐标；owner 已算好几何）
    virtual void Layout() {}
    // 每帧动画推进：返回 true 表示仍在动画
    virtual bool Update(float dt) { (void)dt; return false; }
    // 绘制
    virtual void Draw() {}
    // 鼠标
    virtual void OnMouseMove(float, float) {}
    virtual void OnMouseDown(float, float) {}
    virtual void OnMouseUp(float, float) {}
    virtual void OnMouseLeave() {}
    // 主题变化
    virtual void OnThemeChanged() {}

protected:
    DemoWindow* owner_;
};

} // namespace ModernDesign::Demo