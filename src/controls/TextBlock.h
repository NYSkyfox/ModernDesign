#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <string>

namespace ModernDesign {

// ============================================================
// TextBlock — 文本块
// 支持：文字、字号、粗细、对齐、颜色
// ============================================================

class TextBlock {
public:
    TextBlock() = default;

    // 配置
    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }

    void SetFontSize(float size) { fontSize_ = size; }
    float GetFontSize() const { return fontSize_; }

    void SetBold(bool bold) { bold_ = bold; }
    bool IsBold() const { return bold_; }

    void SetColor(const Color& c) { color_ = c; }
    const Color& GetColor() const { return color_; }

    // 对齐
    enum class Align { Left, Center, Right };
    void SetAlign(Align a) { align_ = a; }
    Align GetAlign() const { return align_; }

    // 布局
    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    // 绘制
    void Draw(Renderer& renderer, const Theme& theme, float scale) const;

private:
    std::wstring text_ = L"Text";
    float fontSize_ = 14.0f;
    bool bold_ = false;
    Color color_{};
    Align align_ = Align::Left;
    RectF bounds_;
};

} // namespace ModernDesign