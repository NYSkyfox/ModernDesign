#include "pch.h"
#include "app/App.h"

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

    // 4. 注册窗口类
    if (!RegisterWindowClass(hInstance)) {
        return E_FAIL;
    }

    // 5. 创建窗口
    hr = CreateWindowEx(hInstance, nCmdShow);
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

    lastFrameMs_ = GetTickCount64();
    lastThemePollMs_ = lastFrameMs_;
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

        // 主题轮询（500ms 节流）
        if (now - lastThemePollMs_ >= 500) {
            lastThemePollMs_ = now;
            bool lightNow = SystemUsesLightTheme();
            if (lightNow != currentLight_) {
                currentLight_ = lightNow;
                theme_.SetLightMode(lightNow);
                OnThemeChanged();
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
            Sleep(8);
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
    OnThemeChanged();
    MarkDirty();
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
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
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

HRESULT App::CreateWindowEx(HINSTANCE hInstance, int nCmdShow) {
    RECT wr = {0, 0, 1200, 760};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    int ww = wr.right - wr.left;
    int wh = wr.bottom - wr.top;

    hwnd_ = CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP,
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