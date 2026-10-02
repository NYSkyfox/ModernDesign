// ============================================================
// ModernDesign DemoApp — 入口
//
//   解耦架构：
//     DemoApp.h     — DemoWindow 外壳（窗口/导航/全局浮层/页面宿主）
//     DemoPage.h    — 页面基类
//     pages/        — 每个页面一个文件（Home / Expander / Settings）
//
//   左侧：NavigationView 侧边栏（hamburger 折叠 + 菜单项 + accent 指示条）
//   右侧：根据选中项切换页面（Home / Expander / Settings）
//
//   鼠标：悬停 / 点击 / 拖拽    键盘：Space 切换深浅主题
// ============================================================
#include "pch.h"
#include "DemoApp.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    DemoWindow app;
    HRESULT hr = app.Initialize(hInstance, nCmdShow);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"ModernDesign init failed (requires Win10 1809+)",
                    L"ModernDesign", MB_ICONERROR);
        return 1;
    }
    return app.Run();
}