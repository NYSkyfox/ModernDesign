#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/TextBlock.h"

namespace ModernDesign {

void TextBlock::Draw(Renderer& renderer, const Theme& theme, float scale) const {
    if (bounds_.IsEmpty() || text_.empty()) return;

    float s = scale;
    float fs = fontSize_ * s;
    Color fg = color_.a > 0.0f ? color_ : theme.TextPrimary();
    DWRITE_FONT_WEIGHT w = bold_ ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;

    DWRITE_TEXT_ALIGNMENT hAlign;
    switch (align_) {
    case Align::Left:   hAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
    case Align::Center: hAlign = DWRITE_TEXT_ALIGNMENT_CENTER; break;
    case Align::Right:  hAlign = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
    default:            hAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
    }

    // 垂直：单行默认居中；多行顶部
    DWRITE_PARAGRAPH_ALIGNMENT vAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

    renderer.DrawText(text_,
                      bounds_.x, bounds_.y, bounds_.w, bounds_.h,
                      L"Segoe UI", fs, w, fg, hAlign, vAlign);
}

} // namespace ModernDesign