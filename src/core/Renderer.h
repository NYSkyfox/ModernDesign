#pragma once

#include "Geometry.h"
#include "core/Theme.h"
#include <d2d1_1.h>
#include <dwrite.h>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// Renderer — Direct2D 绘制封装
//
// 设计要点（借鉴 FluentZero 已验证的经验）：
//   1. 文本对齐用 DirectWrite 对齐属性（SetTextAlignment /
//      SetParagraphAlignment），不依赖 Measure 做布局。
//   2. 垂直居中 = 布局框高设为容器高 + paragraph align = CENTER。
//   3. 画刷 / 文本格式按参数缓存，避免每帧重复创建 COM 对象。
//   4. EndDraw 返回 D2DERR_RECREATE_TARGET 时重建渲染目标。
// ============================================================

class Renderer {
public:
    Renderer();
    virtual ~Renderer();

    // 初始化（创建工厂 + 渲染目标）
    HRESULT Init(HWND hwnd);
    // 释放全部设备资源
    void Shutdown();

    // 客户区尺寸变化（单位：物理像素）
    HRESULT Resize(UINT pixelWidth, UINT pixelHeight);

    bool IsReady() const { return ready_; }

    // ---- 帧 ----
    HRESULT BeginDraw();
    HRESULT EndDraw();
    void Clear(const Color& color);

    // ---- 形状（坐标单位：DIP，与 dpiScale 无关的逻辑像素）----
    void FillRect(const RectF& r, const Color& c);
    void FillRoundedRect(const RectF& r, float radius, const Color& c);
    void StrokeRect(const RectF& r, float strokeWidth, const Color& c);
    void StrokeRoundedRect(const RectF& r, float radius, float strokeWidth, const Color& c);
    void FillEllipse(const RectF& r, const Color& c);
    void StrokeEllipse(const RectF& r, float strokeWidth, const Color& c);
    void DrawLine(float x1, float y1, float x2, float y2,
                  float strokeWidth, const Color& c);

    // ---- 矢量图标路径（SVG path 子集）----
    // 支持 M/L/H/V/C/Z（大小写、绝对/相对），在 (x,y) 为左上角、边长 size 的
    // 方盒内填充；路径源坐标系为 viewBox×viewBox，自动等比缩放。
    // rotRad：绕图标盒中心的旋转（弧度），用于箭头展开动画。
    // 相同 d 指针只会解析/建几何一次（内部缓存，d 必须是静态字面量）。
    void FillSvgPath(const char* d, float viewBox, float x, float y, float size,
                     const Color& c, float rotRad = 0.0f);

    // ---- 裁剪（Push/Pop 成对使用，作用于当前图层栈）----
    void PushClip(const RectF& clip);
    void PopClip();

    // ---- 文本 ----
    // 在 (x, y, maxW, maxH) 布局框内绘制文本
    //   align  : 水平对齐（默认 LEADING = 左对齐）
    //   vAlign : 垂直对齐（默认 NEAR = 顶对齐）
    void DrawText(const std::wstring& text,
                  float x, float y, float maxW, float maxH,
                  const wchar_t* face, float fontSize,
                  DWRITE_FONT_WEIGHT weight, const Color& c,
                  DWRITE_TEXT_ALIGNMENT align = DWRITE_TEXT_ALIGNMENT_LEADING,
                  DWRITE_PARAGRAPH_ALIGNMENT vAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    // 便捷重载：在 box 内水平+垂直居中
    void DrawTextCentered(const std::wstring& text, const RectF& box,
                          const wchar_t* face, float fontSize,
                          DWRITE_FONT_WEIGHT weight, const Color& c);

    // 文本尺寸测量（DIP；outLineHeight 为单行高度）
    bool MeasureText(const std::wstring& text, float maxW,
                     const wchar_t* face, float fontSize,
                     DWRITE_FONT_WEIGHT weight,
                     float* outWidth, float* outHeight,
                     float* outLineHeight = nullptr);

protected:
    HRESULT CreateFactories();
    HRESULT CreateRenderTarget();
    void ReleaseRenderTarget();

    ID2D1SolidColorBrush* GetBrush(const Color& c);
    // 取（或首次解析/缓存）SVG 路径几何
    ID2D1PathGeometry* GetSvgGeometry(const char* d);
    IDWriteTextFormat* GetTextFormat(const wchar_t* face, float fontSize,
                                     DWRITE_FONT_WEIGHT weight,
                                     DWRITE_TEXT_ALIGNMENT align,
                                     DWRITE_PARAGRAPH_ALIGNMENT vAlign);

    HWND hwnd_ = nullptr;
    ComPtr<ID2D1Factory1> d2dFactory_;
    ComPtr<IDWriteFactory> dwriteFactory_;
    ComPtr<ID2D1HwndRenderTarget> rt_;

    // 画刷缓存
    struct SolidBrush {
        float r, g, b, a;
        ComPtr<ID2D1SolidColorBrush> brush;
    };
    std::vector<SolidBrush> solidBrushes_;

    // 文本格式缓存
    struct TextFormatEntry {
        std::wstring face;
        float size;
        DWRITE_FONT_WEIGHT weight;
        DWRITE_TEXT_ALIGNMENT align;
        DWRITE_PARAGRAPH_ALIGNMENT vAlign;
        ComPtr<IDWriteTextFormat> format;
    };
    std::vector<TextFormatEntry> textFormats_;

    // SVG 路径几何缓存（key = d 字符串指针，要求是静态字面量）
    struct SvgPathEntry {
        const char* d = nullptr;
        ComPtr<ID2D1PathGeometry> geo;
    };
    std::vector<SvgPathEntry> svgPaths_;

    UINT pixelWidth_ = 0;
    UINT pixelHeight_ = 0;
    bool ready_ = false;
};

} // namespace ModernDesign