#pragma once

#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>

namespace ModernDesign {

// ============================================================
// App — 应用外壳
// 职责：窗口生命周期 + 消息循环 + 渲染调度 + 主题切换
// 使用方式：继承本类，重写 OnRender / OnInput
// ============================================================

class App : public Renderer {
public:
    App();
    ~App() override;

    // 初始化（COM + DPI + 窗口 + 渲染）
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);

    // 消息循环
    int Run();

    // 请求重绘
    void Invalidate() const;

    // DPI 缩放因子（1.0 = 100%）
    float DpiScale() const { return dpiScale_; }

    // 客户端区尺寸（DIP）
    float ClientWidth() const { return clientWidth_; }
    float ClientHeight() const { return clientHeight_; }

    // 主题
    Theme& GetTheme() { return theme_; }
    const Theme& GetTheme() const { return theme_; }
    void ToggleTheme();

    // 子类重写点
protected:
    // 布局（尺寸/DPI 变化时调用，DIP 坐标）
    virtual void OnLayout() {}
    // 每帧动画推进：返回 true 表示仍在动画（App 会持续重绘）
    virtual bool OnUpdate(float dt) { (void)dt; return false; }
    // 渲染每帧内容
    virtual void OnRender() = 0;
    // 鼠标移动（DIP 坐标）
    virtual void OnMouseMove(float x, float y) { (void)x; (void)y; }
    // 鼠标按下
    virtual void OnMouseDown(float x, float y) { (void)x; (void)y; }
    // 鼠标抬起
    virtual void OnMouseUp(float x, float y) { (void)x; (void)y; }
    // 鼠标离开窗口
    virtual void OnMouseLeave() {}
    // 主题变化
    virtual void OnThemeChanged() {}
    // 键盘按下（virtual key）
    virtual void OnKeyDown(int vk) { (void)vk; }

    // 窗口句柄
    HWND GetHwnd() const { return hwnd_; }
    HINSTANCE GetInstance() const { return hInstance_; }

    // 标记需要重绘 / 有动画
    void MarkDirty() { needsDraw_ = true; }

    // 实际执行绘制（子类可在自己合适时机调用）
    void DrawNow();

private:
    // 窗口类与消息处理
    static LRESULT CALLBACK WndProcThunk(HWND h, UINT m, WPARAM w, LPARAM l);
    LRESULT HandleMessage(HWND h, UINT m, WPARAM w, LPARAM l);

    bool RegisterWindowClass(HINSTANCE hInstance);
    HRESULT CreateMainWindow(HINSTANCE hInstance, int nCmdShow);
    void UpdateClientSize();
    void UpdateDpiScale();
    // 从 Windows 注册表读取强调色并应用（读不到则用默认墨绿），返回是否变化
    bool ApplySystemAccent();
    // 让系统标题栏跟随应用主题（DWMWA_USE_IMMERSIVE_DARK_MODE）
    void ApplyTitleBarTheme();

    // 消息循环辅助
    static void RequestQuit(HWND hwnd);

    HINSTANCE hInstance_ = nullptr;
    HWND hwnd_ = nullptr;
    float dpiScale_ = 1.0f;
    float clientWidth_ = 0.0f;
    float clientHeight_ = 0.0f;

    bool quit_ = false;
    bool needsDraw_ = true;
    bool animating_ = false;
    bool themeManual_ = false;   // 用户手动切换主题后，停止自动跟随系统
    ULONGLONG lastFrameMs_ = 0;
    ULONGLONG lastThemePollMs_ = 0;
    bool currentLight_ = true;
    float lastAccentR_ = -1.0f;
    float lastAccentG_ = -1.0f;
    float lastAccentB_ = -1.0f;

    Theme theme_;
};

} // namespace ModernDesign