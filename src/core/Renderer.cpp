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

void Renderer::PushClip(const RectF& clip) {
    if (!rt_) return;
    // D2D1 矩形裁剪：PushAxisAlignedClip / PopAxisAlignedClip（DIP 坐标；本框架不设 D2D 变换，故与绘制坐标一致）
    rt_->PushAxisAlignedClip(clip.ToD2D(), D2D1_ANTIALIAS_MODE_ALIASED);
}

void Renderer::PopClip() {
    if (!rt_) return;
    rt_->PopAxisAlignedClip();
}

// ============================================================
// 矢量图标路径（SVG path 子集：M/L/H/V/C/Z）
//
// 只服务于「官方 Fluent 图标」这类由工具生成的静态路径，
// 因此刻意保持极小：不支持弧线 A / 二次曲线 Q（上游数据也用不到）。
// ============================================================
namespace {

// 读一个 SVG 数字（允许逗号/空白分隔、前后符号、小数点、指数）
bool SvgNumber(const char* s, size_t& i, float& out) {
    while (s[i] == ' ' || s[i] == ',' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') ++i;
    size_t start = i;
    bool anyDigit = false;
    if (s[i] == '+' || s[i] == '-') ++i;
    bool dot = false;
    while (s[i]) {
        const char ch = s[i];
        if (ch >= '0' && ch <= '9') { anyDigit = true; ++i; continue; }
        if (ch == '.' && !dot) { dot = true; ++i; continue; }
        if ((ch == 'e' || ch == 'E') && anyDigit) {
            ++i;
            if (s[i] == '+' || s[i] == '-') ++i;
            continue;
        }
        break;
    }
    if (!anyDigit) { i = start; return false; }

    char buf[40];
    size_t n = i - start;
    if (n > sizeof(buf) - 1) n = sizeof(buf) - 1;
    memcpy(buf, s + start, n);
    buf[n] = '\0';
    try {
        out = std::stof(buf);
    } catch (...) {
        return false;
    }
    return true;
}

// 把 d 属性写进 sink
bool BuildSvgSink(ID2D1GeometrySink* sink, const char* s) {
    float cx = 0.0f, cy = 0.0f;   // 当前点
    float sx = 0.0f, sy = 0.0f;   // 子路径起点
    char  cmd = 0;
    bool  open = false;
    size_t i = 0;

    auto number = [&](float& v) { return SvgNumber(s, i, v); };
    auto ensureOpen = [&]() {
        if (!open) {
            sink->BeginFigure(D2D1::Point2F(cx, cy), D2D1_FIGURE_BEGIN_FILLED);
            open = true;
            sx = cx;
            sy = cy;
        }
    };

    while (true) {
        while (s[i] == ' ' || s[i] == ',' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') ++i;
        if (!s[i]) break;

        const char ch = s[i];
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            cmd = ch;
            ++i;
        } else if (!cmd) {
            return false;   // 路径必须以命令开头
        }

        float a[6] = {};
        switch (cmd) {
        case 'M': case 'm': {
            if (!number(a[0]) || !number(a[1])) return false;
            float x = a[0], y = a[1];
            if (cmd == 'm') { x += cx; y += cy; }
            if (open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
            sink->BeginFigure(D2D1::Point2F(x, y), D2D1_FIGURE_BEGIN_FILLED);
            open = true;
            cx = sx = x;
            cy = sy = y;
            cmd = (cmd == 'M') ? 'L' : 'l';   // 后续参数对按隐式 L 处理
            break;
        }
        case 'L': case 'l': {
            if (!number(a[0]) || !number(a[1])) return false;
            float x = a[0], y = a[1];
            if (cmd == 'l') { x += cx; y += cy; }
            ensureOpen();
            sink->AddLine(D2D1::Point2F(x, y));
            cx = x; cy = y;
            break;
        }
        case 'H': case 'h': {
            if (!number(a[0])) return false;
            float x = (cmd == 'h') ? cx + a[0] : a[0];
            ensureOpen();
            sink->AddLine(D2D1::Point2F(x, cy));
            cx = x;
            break;
        }
        case 'V': case 'v': {
            if (!number(a[0])) return false;
            float y = (cmd == 'v') ? cy + a[0] : a[0];
            ensureOpen();
            sink->AddLine(D2D1::Point2F(cx, y));
            cy = y;
            break;
        }
        case 'C': case 'c': {
            for (int k = 0; k < 6; ++k)
                if (!number(a[k])) return false;
            float x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x = a[4], y = a[5];
            if (cmd == 'c') {
                x1 += cx; y1 += cy;
                x2 += cx; y2 += cy;
                x += cx;  y += cy;
            }
            ensureOpen();
            sink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(x1, y1),
                                                D2D1::Point2F(x2, y2),
                                                D2D1::Point2F(x, y)));
            cx = x; cy = y;
            break;
        }
        case 'Z': case 'z': {
            if (open) {
                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                open = false;
            }
            cx = sx;
            cy = sy;
            break;
        }
        default:
            return false;   // 未支持的命令（A/Q/S/T）
        }
    }
    if (open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
    return true;
}

} // namespace

ID2D1PathGeometry* Renderer::GetSvgGeometry(const char* d) {
    if (!d) return nullptr;
    for (auto& e : svgPaths_)
        if (e.d == d) return e.geo.Get();
    if (!d2dFactory_) return nullptr;

    ComPtr<ID2D1PathGeometry> geo;
    if (FAILED(d2dFactory_->CreatePathGeometry(geo.GetAddressOf()))) return nullptr;
    ComPtr<ID2D1GeometrySink> sink;
    if (FAILED(geo->Open(sink.GetAddressOf()))) return nullptr;
    sink->SetFillMode(D2D1_FILL_MODE_WINDING);   // 与 SVG 默认 fill-rule（nonzero）一致
    if (!BuildSvgSink(sink.Get(), d)) {
        sink->Close();
        return nullptr;
    }
    if (FAILED(sink->Close())) return nullptr;

    SvgPathEntry entry;
    entry.d = d;
    entry.geo = geo;
    svgPaths_.push_back(entry);
    return geo.Get();
}

void Renderer::FillSvgPath(const char* d, float viewBox, float x, float y, float size,
                           const Color& c, float rotRad) {
    if (!rt_ || viewBox <= 0.0f || size <= 0.0f) return;
    ID2D1PathGeometry* geo = GetSvgGeometry(d);
    if (!geo) return;
    ID2D1SolidColorBrush* b = GetBrush(c);
    if (!b) return;

    const float k = size / viewBox;
    D2D1_MATRIX_3X2_F old{};
    rt_->GetTransform(&old);

    D2D1_MATRIX_3X2_F m = D2D1::Matrix3x2F::Scale(k, k) *
                          D2D1::Matrix3x2F::Translation(x, y);
    if (rotRad != 0.0f) {
        const float deg = rotRad * 180.0f / kPi;
        m = m * D2D1::Matrix3x2F::Rotation(
                    deg, D2D1::Point2F(x + size * 0.5f, y + size * 0.5f));
    }

    rt_->SetTransform(m);
    rt_->FillGeometry(geo, b);
    rt_->SetTransform(old);
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
                   DWRITE_MEASURING_MODE_NATURAL);
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