#include "pch.h"
#include "app/App.h"
#include <mmsystem.h>   // timeBeginPeriod / timeEndPeriod
// 老 SDK 的 dwmapi.h 可能未定义 DWMWA_USE_IMMERSIVE_DARK_MODE（Win10 18985+ 引入）
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
namespace ModernDesign {

namespace {

// DPI 感知（必须在创建任何窗口之前调用）
void EnableDpiAwareness() {
    // Win10 1703+ 首选：SetProcessDpiAwarenessContext(PER_MONITOR_AWARE_V2)
    using SetCtxFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        auto fn = reinterpret_cast<SetCtxFn>(
            GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (fn) {
            if (fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) return;
        }
    }
    // 回退：shcore 的 SetProcessDpiAwareness（1703+）
    if (HMODULE shcore = LoadLibraryW(L"shcore.dll")) {
        using SetFn = HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS);
        auto fn = reinterpret_cast<SetFn>(GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (fn) {
            if (SUCCEEDED(fn(PROCESS_PER_MONITOR_DPI_AWARE))) {
                FreeLibrary(shcore);
                return;
            }
        }
        FreeLibrary(shcore);
    }
    // 最后回退：Vista+ 的 SetProcessDPIAware
    SetProcessDPIAware();
}

// 读取系统深浅主题（AppsUseLightTheme 注册表）
bool SystemUsesLightTheme() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &key) != ERROR_SUCCESS) {
        return true;  // 默认浅色
    }
    DWORD value = 1;
    DWORD size = sizeof(value);
    DWORD type = 0;
    LONG rc = RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, &type,
                               reinterpret_cast<BYTE*>(&value), &size);
    RegCloseKey(key);
    if (rc != ERROR_SUCCESS || type != REG_DWORD) return true;
    return value != 0;
}

// 读取 Windows 强调色（HKCU\...\Windows\DWM\AccentColor）
// 值为 0x00RRGGBB（低 24 位是 RRGGBB，高 8 位 alpha 通常 0x00）
// 返回 true 表示读到有效值，并写入 r/g/b（0~1）
bool ReadSystemAccentColor(float& r, float& g, float& b) {
    // MODERNDESIGN_FORCE_FALLBACK_ACCENT=1 → 跳过系统读取（CI 验证默认墨绿用）
    wchar_t nv[16] = {};
    if (GetEnvironmentVariableW(L"MODERNDESIGN_FORCE_FALLBACK_ACCENT", nv, 16) > 0 &&
        (_wcsicmp(nv, L"1") == 0 || _wcsicmp(nv, L"true") == 0)) {
        return false;
    }
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\DWM",
                      0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    DWORD value = 0;
    DWORD size = sizeof(value);
    DWORD type = 0;
    LONG rc = RegQueryValueExW(key, L"AccentColor", nullptr, &type,
                               reinterpret_cast<BYTE*>(&value), &size);
    RegCloseKey(key);
    if (rc != ERROR_SUCCESS || type != REG_DWORD) return false;

    int rr = static_cast<int>(value & 0xFF);
    int gg = static_cast<int>((value >> 8) & 0xFF);
    int bb = static_cast<int>((value >> 16) & 0xFF);
    if (rr == 0 && gg == 0 && bb == 0) return false;   // 纯黑视为未设置

    r = static_cast<float>(rr) / 255.0f;
    g = static_cast<float>(gg) / 255.0f;
    b = static_cast<float>(bb) / 255.0f;
    return true;
}

constexpr wchar_t kWindowClassName[] = L"ModernDesignAppWindow";

} // namespace

// ============================================================
// 构造 / 析构
// ============================================================
App::App() = default;

App::~App() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    timeEndPeriod(1);   // 恢复系统默认定时器精度（Initialize 里 timeBeginPeriod(1)）
    Shutdown();
}

// ============================================================
// 初始化
// ============================================================
HRESULT App::Initialize(HINSTANCE hInstance, int nCmdShow) {
    hInstance_ = hInstance;

    // 1. COM（D2D / DWrite 需要）
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        LogError(L"CoInitializeEx failed", hr);
        return hr;
    }

    // 2. DPI 感知
    EnableDpiAwareness();

    // 3. 系统主题
    currentLight_ = SystemUsesLightTheme();
    theme_.SetLightMode(currentLight_);
    ApplySystemAccent();   // 优先 Windows 强调色，失败用默认墨绿

    // 4. 注册窗口类
    if (!RegisterWindowClass(hInstance)) {
        return E_FAIL;
    }

    // 5. 创建窗口
    hr = CreateMainWindow(hInstance, nCmdShow);
    if (FAILED(hr)) return hr;

    // 6. 初始化渲染
    hr = Init(hwnd_);
    if (FAILED(hr)) {
        LogError(L"Renderer::Init failed", hr);
        return hr;
    }

    // 7. 尺寸 + 布局
    UpdateClientSize();
    UpdateDpiScale();
    OnLayout();
    OnThemeChanged();
    ApplyTitleBarTheme();   // 标题栏初始跟随系统主题

    lastFrameMs_ = GetTickCount64();
    lastThemePollMs_ = lastFrameMs_;
    // 把系统定时器精度提到 1ms：显著降低 GetTickCount64 的 ~15ms 跳变，
    // 让导航/开关等定时动画的 dt 平滑（否则 paneElapsed += dt 会逐 15ms 跳、卡顿）。
    timeBeginPeriod(1);
    return S_OK;
}

// ============================================================
// 消息循环
// ============================================================
int App::Run() {
    MSG msg;

    while (!quit_) {
        // 处理所有待处理消息
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                quit_ = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (quit_) break;

        // 计算帧间隔
        ULONGLONG now = GetTickCount64();
        float dt = (now - lastFrameMs_) / 1000.0f;
        lastFrameMs_ = now;
        dt = ClampF(dt, 0.0f, 0.1f);   // 防止卡顿后跳变

        // 主题轮询（500ms 节流）——仅在用户未手动切换时自动跟随系统
        if (now - lastThemePollMs_ >= 500) {
            lastThemePollMs_ = now;
            if (!themeManual_) {
                bool lightNow = SystemUsesLightTheme();
                if (lightNow != currentLight_) {
                    currentLight_ = lightNow;
                    theme_.SetLightMode(lightNow);
                    ApplyTitleBarTheme();
                    OnThemeChanged();
                    needsDraw_ = true;
                }
            }
            if (ApplySystemAccent()) {
                needsDraw_ = true;
            }
        }

        // 推进动画（子类返回 true 表示仍在动画）
        if (OnUpdate(dt)) {
            animating_ = true;
        }

        // 需要绘制时渲染一帧
        if (needsDraw_ || animating_) {
            DrawNow();
            needsDraw_ = false;
            animating_ = false;
        }

        // 空闲时挂起等待消息（省 CPU）
        if (!needsDraw_ && !animating_) {
            MsgWaitForMultipleObjectsEx(0, nullptr, 100, QS_ALLINPUT, MWMO_ALERTABLE);
        } else {
            Sleep(1);
        }
    }

    return static_cast<int>(msg.wParam);
}

void App::DrawNow() {
    if (!IsReady()) return;

    if (FAILED(BeginDraw())) {
        MarkDirty();
        return;
    }

    Clear(theme_.WindowBg());
    OnRender();

    HRESULT hr = EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        // 设备丢失：丢弃资源后下一帧重建
        Shutdown();
        Init(hwnd_);
        MarkDirty();
    }
}

void App::Invalidate() const {
    if (hwnd_) {
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
}

void App::ToggleTheme() {
    currentLight_ = !currentLight_;
    theme_.SetLightMode(currentLight_);
    themeManual_ = true;   // 锁定手动值，轮询不再覆盖
    ApplyTitleBarTheme();  // 标题栏跟随
    OnThemeChanged();
    MarkDirty();
}

void App::ApplyTitleBarTheme() {
    if (!hwnd_) return;
    // DWMWA_USE_IMMERSIVE_DARK_MODE (Win10 2004+ / Win11)。
    // 老系统无此属性时 DwmSetWindowAttribute 返回失败，静默忽略。
    BOOL dark = currentLight_ ? FALSE : TRUE;
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE,
                          &dark, sizeof(dark));
}

// ============================================================
// 窗口创建
// ============================================================
bool App::RegisterWindowClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (registered) return true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &App::WndProcThunk;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
    wc.hbrBackground = nullptr;   // 全部自绘
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassExW(&wc)) {
        // ERROR_CLASS_ALREADY_EXISTS 可视为成功
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            LogError(L"RegisterClassExW failed");
            return false;
        }
    }
    registered = true;
    return true;
}

HRESULT App::CreateMainWindow(HINSTANCE hInstance, int nCmdShow) {
    RECT wr = {0, 0, 1200, 760};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    int ww = wr.right - wr.left;
    int wh = wr.bottom - wr.top;

    // WS_EX_NOREDIRECTIONBITMAP：把 D2D backbuffer 直接交给 DWM 合成（Acrylic 用）。
    // 但 GDI 截图（PrintWindow / CopyFromScreen）抓不到这类窗口的客户区内容。
    // 设置环境变量 MODERNDESIGN_NO_NORedirection=1 可关闭它，
    // 让 D2D 走 GDI 重定向路径，便于 CI 截图。CI 是 WARP（无 GPU、无 blur），无损失。
    DWORD ex = 0;
    wchar_t nv[16] = {};
    if (GetEnvironmentVariableW(L"MODERNDESIGN_NO_NOREDIRECT", nv, 16) > 0 &&
        _wcsicmp(nv, L"1") != 0 && _wcsicmp(nv, L"true") != 0) {
        ex = WS_EX_NOREDIRECTIONBITMAP;
    }

    hwnd_ = CreateWindowExW(
        ex,
        kWindowClassName,
        L"Modern Design",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, ww, wh,
        nullptr, nullptr, hInstance, this);

    if (!hwnd_) {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        LogError(L"CreateWindowExW failed", hr);
        return hr;
    }

    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
    return S_OK;
}

void App::UpdateClientSize() {
    if (!hwnd_) return;
    RECT rc{};
    GetClientRect(hwnd_, &rc);
    clientWidth_ = static_cast<float>(rc.right - rc.left);
    clientHeight_ = static_cast<float>(rc.bottom - rc.top);
}

void App::UpdateDpiScale() {
    if (!hwnd_) return;
    UINT dpi = 96;
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        using GetDpiFn = UINT(WINAPI*)(HWND);
        auto fn = reinterpret_cast<GetDpiFn>(GetProcAddress(user32, "GetDpiForWindow"));
        if (fn) dpi = fn(hwnd_);
    }
    if (dpi == 0) dpi = 96;
    dpiScale_ = static_cast<float>(dpi) / 96.0f;
}

bool App::ApplySystemAccent() {
    // 优先 Windows 强调色；读不到则保持 Theme 默认（墨绿）
    float r = theme_.accentR, g = theme_.accentG, b = theme_.accentB;
    ReadSystemAccentColor(r, g, b);

    if (r == lastAccentR_ && g == lastAccentG_ && b == lastAccentB_) {
        return false;   // 无变化
    }
    lastAccentR_ = r;
    lastAccentG_ = g;
    lastAccentB_ = b;
    theme_.SetAccent(r, g, b);
    return true;
}

void App::RequestQuit(HWND hwnd) {
    PostQuitMessage(0);
    (void)hwnd;
}

// ============================================================
// 消息处理
// ============================================================
LRESULT CALLBACK App::WndProcThunk(HWND h, UINT m, WPARAM w, LPARAM l) {
    App* self = nullptr;

    if (m == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(l);
        self = static_cast<App*>(cs->lpCreateParams);
        SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<App*>(GetWindowLongPtrW(h, GWLP_USERDATA));
    }

    if (self) return self->HandleMessage(h, m, w, l);
    return DefWindowProcW(h, m, w, l);
}

LRESULT App::HandleMessage(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_NCCREATE:
        return DefWindowProcW(h, m, w, l);

    case WM_CREATE:
        return 0;

    case WM_SIZE: {
        UINT nw = LOWORD(l);
        UINT nh = HIWORD(l);
        if (nw > 0 && nh > 0) {
            // 像素 → DIP
        UpdateDpiScale();
        UpdateClientSize();
        Resize(nw, nh);      // 渲染目标用像素
        OnLayout();
        MarkDirty();
        }
        return 0;
    }

    case WM_DPICHANGED: {
        // 建议的新窗口矩形
        const RECT* suggested = reinterpret_cast<const RECT*>(l);
        if (suggested && hwnd_) {
            SetWindowPos(hwnd_, nullptr,
                         suggested->left, suggested->top,
                         suggested->right - suggested->left,
                         suggested->bottom - suggested->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        UpdateDpiScale();
        UpdateClientSize();
        OnLayout();
        MarkDirty();
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd_, &ps);
        EndPaint(hwnd_, &ps);
        return 0;   // 全部由 D2D 渲染
    }

    case WM_ERASEBKGND:
        return 1;   // 防止闪烁

    case WM_MOUSEMOVE: {
        float x = static_cast<float>(GET_X_LPARAM(l)) / dpiScale_;
        float y = static_cast<float>(GET_Y_LPARAM(l)) / dpiScale_;
        OnMouseMove(x, y);
        MarkDirty();
        // 追踪鼠标离开
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd_;
        TrackMouseEvent(&tme);
        return 0;
    }

    case WM_MOUSELEAVE:
        OnMouseLeave();
        MarkDirty();
        return 0;

    case WM_LBUTTONDOWN:
        SetCapture(hwnd_);
        SetFocus(hwnd_);
        OnMouseDown(static_cast<float>(GET_X_LPARAM(l)) / dpiScale_,
                    static_cast<float>(GET_Y_LPARAM(l)) / dpiScale_);
        MarkDirty();
        return 0;

    case WM_LBUTTONUP: {
        if (GetCapture() == hwnd_) ReleaseCapture();
        OnMouseUp(static_cast<float>(GET_X_LPARAM(l)) / dpiScale_,
                  static_cast<float>(GET_Y_LPARAM(l)) / dpiScale_);
        MarkDirty();
        return 0;
    }

    case WM_KEYDOWN:
        OnKeyDown(static_cast<int>(w));
        MarkDirty();
        return 0;

    case WM_DESTROY:
        quit_ = true;
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(h, m, w, l);
    }
}

} // namespace ModernDesign