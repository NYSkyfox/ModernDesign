#include "pch.h"
#include "core/Renderer.h"

namespace ModernDesign {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Shutdown();
}

// ============================================================
// 初始化 / 释放
// ============================================================
HRESULT Renderer::Init(HWND hwnd) {
    hwnd_ = hwnd;
    if (!hwnd_) return E_INVALIDARG;

    HRESULT hr = CreateFactories();
    if (FAILED(hr)) return hr;

    // 从窗口客户区取物理像素尺寸
    RECT rc{};
    if (GetClientRect(hwnd_, &rc)) {
        pixelWidth_ = static_cast<UINT>(rc.right - rc.left);
        pixelHeight_ = static_cast<UINT>(rc.bottom - rc.top);
    }

    return CreateRenderTarget();
}

void Renderer::Shutdown() {
    solidBrushes_.clear();
    textFormats_.clear();
    ReleaseRenderTarget();
    dwriteFactory_.Reset();
    d2dFactory_.Reset();
    ready_ = false;
}

HRESULT Renderer::CreateFactories() {
    if (!d2dFactory_) {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                       __uuidof(ID2D1Factory1),
                                       reinterpret_cast<void**>(d2dFactory_.GetAddressOf()));
        if (FAILED(hr)) {
            LogError(L"D2D1CreateFactory failed", hr);
            return hr;
        }
    }

    if (!dwriteFactory_) {
        HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                         __uuidof(IDWriteFactory),
                                         reinterpret_cast<IUnknown**>(dwriteFactory_.GetAddressOf()));
        if (FAILED(hr)) {
            LogError(L"DWriteCreateFactory failed", hr);
            return hr;
        }
    }

    return S_OK;
}

HRESULT Renderer::CreateRenderTarget() {
    if (!hwnd_ || !d2dFactory_) return E_FAIL;

    if (pixelWidth_ == 0 || pixelHeight_ == 0) {
        RECT rc{};
        GetClientRect(hwnd_, &rc);
        pixelWidth_ = static_cast<UINT>(rc.right - rc.left);
        pixelHeight_ = static_cast<UINT>(rc.bottom - rc.top);
    }
    if (pixelWidth_ == 0 || pixelHeight_ == 0) {
        pixelWidth_ = 1;
        pixelHeight_ = 1;
    }

    D2D1_RENDER_TARGET_PROPERTIES props = {};
    props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;   // 软件兼容（WARP 也可）
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                          D2D1_ALPHA_MODE_PREMULTIPLIED);
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;
    props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
    props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = {};
    hwndProps.hwnd = hwnd_;
    hwndProps.pixelSize = D2D1::SizeU(pixelWidth_, pixelHeight_);
    hwndProps.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

    HRESULT hr = d2dFactory_->CreateHwndRenderTarget(props, hwndProps,
                                                     rt_.GetAddressOf());
    if (FAILED(hr)) {
        LogError(L"CreateHwndRenderTarget failed", hr);
        ready_ = false;
        return hr;
    }

    // 缓存随渲染目标失效
    solidBrushes_.clear();
    textFormats_.clear();
    ready_ = true;
    return S_OK;
}

void Renderer::ReleaseRenderTarget() {
    if (rt_) rt_.Reset();
}

HRESULT Renderer::Resize(UINT pixelWidth, UINT pixelHeight) {
    pixelWidth_ = pixelWidth;
    pixelHeight_ = pixelHeight;
    if (!rt_) return CreateRenderTarget();

    if (pixelWidth == 0 || pixelHeight == 0) return S_OK;

    HRESULT hr = rt_->Resize(D2D1::SizeU(pixelWidth, pixelHeight));
    if (hr == D2DERR_RECREATE_TARGET) {
        // 渲染目标失效 → 重建
        ReleaseRenderTarget();
        solidBrushes_.clear();
        textFormats_.clear();
        return CreateRenderTarget();
    }
    return hr;
}

// ============================================================
// 帧
// ============================================================
HRESULT Renderer::BeginDraw() {
    if (!ready_ || !rt_) return E_FAIL;
    solidBrushes_.clear();
    textFormats_.clear();
    rt_->BeginDraw();
    return S_OK;
}

HRESULT Renderer::EndDraw() {
    if (!ready_ || !rt_) return E_FAIL;
    HRESULT hr = rt_->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        LogWarn(L"EndDraw: D2DERR_RECREATE_TARGET, recreating render target");
        ReleaseRenderTarget();
        solidBrushes_.clear();
        textFormats_.clear();
        hr = CreateRenderTarget();
    }
    return hr;
}

void Renderer::Clear(const Color& c) {
    if (!ready_ || !rt_) return;
    rt_->Clear(c.ToD2D());
}

// ============================================================
// 形状
// ============================================================
void Renderer::FillRect(const RectF& r, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (b) rt_->FillRectangle(r.ToD2D(), b);
}

void Renderer::FillRoundedRect(const RectF& r, float radius, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;
    // 半径不得超过短边一半
    float maxR = FzMn(r.w, r.h) * 0.5f;
    float rad = FzMn(radius, maxR);
    D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(r.ToD2D(), rad, rad);
    rt_->FillRoundedRectangle(rr, b);
}

void Renderer::StrokeRect(const RectF& r, float strokeWidth, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (b) rt_->DrawRectangle(r.ToD2D(), b, strokeWidth);
}

void Renderer::StrokeRoundedRect(const RectF& r, float radius,
                                  float strokeWidth, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;
    float maxR = FzMn(r.w, r.h) * 0.5f;
    float rad = FzMn(radius, maxR);
    D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(r.ToD2D(), rad, rad);
    rt_->DrawRoundedRectangle(rr, b, strokeWidth);
}

void Renderer::FillEllipse(const RectF& r, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;
    D2D1_ELLIPSE e = D2D1::Ellipse(
        D2D1::Point2F(r.x + r.w * 0.5f, r.y + r.h * 0.5f),
        r.w * 0.5f, r.h * 0.5f);
    rt_->FillEllipse(e, b);
}

void Renderer::StrokeEllipse(const RectF& r, float strokeWidth, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;
    D2D1_ELLIPSE e = D2D1::Ellipse(
        D2D1::Point2F(r.x + r.w * 0.5f, r.y + r.h * 0.5f),
        r.w * 0.5f, r.h * 0.5f);
    rt_->DrawEllipse(e, b, strokeWidth);
}

void Renderer::DrawLine(float x1, float y1, float x2, float y2,
                        float strokeWidth, const Color& c) {
    if (!rt_) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;
    rt_->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), b, strokeWidth);
}

// ============================================================
// 文本
// ============================================================
void Renderer::DrawText(const std::wstring& text,
                        float x, float y, float maxW, float maxH,
                        const wchar_t* face, float fontSize,
                        DWRITE_FONT_WEIGHT weight, const Color& c,
                        DWRITE_TEXT_ALIGNMENT align,
                        DWRITE_PARAGRAPH_ALIGNMENT vAlign) {
    if (!rt_ || text.empty()) return;

    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;

    IDWriteTextFormat* fmt = GetTextFormat(face, fontSize, weight, align, vAlign);
    if (!fmt) return;

    D2D1_RECT_F layout = D2D1::RectF(x, y, x + maxW, y + maxH);

    rt_->DrawText(text.c_str(),
                  static_cast<UINT32>(text.size()),
                  fmt,
                  layout,
                  b,
                   D2D1_DRAW_TEXT_OPTIONS_CLIP,
                   0);   // measureMode（DEFAULT），用字面量避免依赖枚举常量名
}

void Renderer::DrawTextCentered(const std::wstring& text, const RectF& box,
                                const wchar_t* face, float fontSize,
                                DWRITE_FONT_WEIGHT weight, const Color& c) {
    // 水平 + 垂直都交给 DirectWrite 对齐属性
    // 这是 FluentZero 里踩坑后确定的做法：手算偏移永远对不齐
    DrawText(text,
             box.x, box.y, box.w, box.h,
             face, fontSize, weight, c,
             DWRITE_TEXT_ALIGNMENT_CENTER,
             DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

bool Renderer::MeasureText(const std::wstring& text, float maxW,
                           const wchar_t* face, float fontSize,
                           DWRITE_FONT_WEIGHT weight,
                           float* outWidth, float* outHeight,
                           float* outLineHeight) {
    if (!dwriteFactory_ || text.empty()) {
        if (outWidth) *outWidth = 0.0f;
        if (outHeight) *outHeight = 0.0f;
        if (outLineHeight) *outLineHeight = 0.0f;
        return false;
    }

    // 测量用途的格式（不需要对齐设置）
    ComPtr<IDWriteTextFormat> fmt;
    HRESULT hr = dwriteFactory_->CreateTextFormat(
        face ? face : L"Segoe UI",
        nullptr,
        weight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"zh-CN",
        fmt.GetAddressOf());
    if (FAILED(hr)) return false;

    fmt->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    ComPtr<IDWriteTextLayout> layout;
    hr = dwriteFactory_->CreateTextLayout(text.c_str(),
                                          static_cast<UINT32>(text.size()),
                                          fmt.Get(),
                                          FzMx(maxW, 1.0f),
                                          4096.0f,
                                          layout.GetAddressOf());
    if (FAILED(hr)) return false;

    DWRITE_TEXT_METRICS metrics = {};
    hr = layout->GetMetrics(&metrics);
    if (FAILED(hr)) return false;

    if (outWidth) *outWidth = static_cast<float>(metrics.width);
    if (outHeight) *outHeight = static_cast<float>(metrics.height);
    if (outLineHeight) *outLineHeight = static_cast<float>(metrics.height);
    return true;
}

// ============================================================
// 缓存
// ============================================================
ID2D1SolidColorBrush* Renderer::GetBrush(const Color& c) {
    if (!rt_) return nullptr;

    for (auto& entry : solidBrushes_) {
        if (entry.r == c.r && entry.g == c.g && entry.b == c.b && entry.a == c.a) {
            return entry.brush.Get();
        }
    }

    ComPtr<ID2D1SolidColorBrush> brush;
    HRESULT hr = rt_->CreateSolidColorBrush(c.ToD2D(), brush.GetAddressOf());
    if (FAILED(hr)) {
        LogError(L"CreateSolidColorBrush failed", hr);
        return nullptr;
    }

    solidBrushes_.push_back({c.r, c.g, c.b, c.a, brush});
    return brush.Get();
}

IDWriteTextFormat* Renderer::GetTextFormat(const wchar_t* face, float fontSize,
                                           DWRITE_FONT_WEIGHT weight,
                                           DWRITE_TEXT_ALIGNMENT align,
                                           DWRITE_PARAGRAPH_ALIGNMENT vAlign) {
    if (!dwriteFactory_) return nullptr;

    std::wstring faceName = face ? face : L"Segoe UI";

    for (auto& entry : textFormats_) {
        if (entry.face == faceName &&
            entry.size == fontSize &&
            entry.weight == weight &&
            entry.align == align &&
            entry.vAlign == vAlign) {
            return entry.format.Get();
        }
    }

    ComPtr<IDWriteTextFormat> fmt;
    HRESULT hr = dwriteFactory_->CreateTextFormat(
        faceName.c_str(),
        nullptr,
        weight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"zh-CN",
        fmt.GetAddressOf());
    if (FAILED(hr)) {
        LogError(L"CreateTextFormat failed", hr);
        return nullptr;
    }

    // 对齐属性：文本水平对齐 + 段落（垂直）对齐
    fmt->SetTextAlignment(align);
    fmt->SetParagraphAlignment(vAlign);
    // 单行不换行（控件内文字通常手动换行）
    fmt->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    textFormats_.push_back({faceName, fontSize, weight, align, vAlign, fmt});
    return fmt.Get();
}

} // namespace ModernDesign