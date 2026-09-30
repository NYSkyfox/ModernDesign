#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Card.h"

namespace ModernDesign {

// WinUI 3 规格
constexpr float kCardRadius = 4.0f;
constexpr float kCardTitleFont = 17.0f;   // Subhead
constexpr float kCardBodyFont = 13.0f;
constexpr float kCardPadTop = 12.0f;
constexpr float kCardPadX = 16.0f;
constexpr float kCardPadBottom = 12.0f;
constexpr float kCardTitleGap = 6.0f;
constexpr float kCardLineHeight = 18.0f;

// 统计内容里的换行数
static int CountLines(const std::wstring& s) {
    if (s.empty()) return 0;
    int lines = 1;
    for (wchar_t c : s) {
        if (c == L'\n') ++lines;
    }
    return lines;
}

float Card::MeasureHeight(const Theme& theme, float scale) const {
    (void)theme;   // 目前仅依赖字号常量
    float s = scale;
    float h = kCardPadTop * s + kCardPadBottom * s;

    if (!title_.empty()) {
        h += kCardTitleFont * s * 1.4f + kCardTitleGap * s;
    }
    if (!content_.empty()) {
        h += CountLines(content_) * kCardLineHeight * s;
    }
    return h;
}

void Card::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    float radius = kCardRadius * s;

    // 底色
    renderer.FillRoundedRect(bounds_, radius, theme.CardBg());
    // 1px 描边
    renderer.StrokeRoundedRect(bounds_, radius, 1.0f * s, theme.CardBorder());

    float contentX = bounds_.x + kCardPadX * s;
    float contentW = bounds_.w - kCardPadX * s * 2.0f;
    float y = bounds_.y + kCardPadTop * s;

    // 标题
    if (!title_.empty()) {
        float titleH = kCardTitleFont * s * 1.4f;
        renderer.DrawText(title_,
                          contentX, y, contentW, titleH,
                          L"Segoe UI", kCardTitleFont * s,
                          DWRITE_FONT_WEIGHT_SEMI_BOLD, theme.TextPrimary(),
                          DWRITE_TEXT_ALIGNMENT_LEADING,
                          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        y += titleH + kCardTitleGap * s;
    }

    // 内容（按 \n 拆行）
    if (!content_.empty()) {
        size_t pos = 0;
        int lineIndex = 0;
        while (pos <= content_.size()) {
            size_t nl = content_.find(L'\n', pos);
            std::wstring line = (nl == std::wstring::npos)
                ? content_.substr(pos)
                : content_.substr(pos, nl - pos);

            renderer.DrawText(line,
                              contentX, y + lineIndex * kCardLineHeight * s,
                              contentW, kCardLineHeight * s,
                              L"Segoe UI", kCardBodyFont * s,
                              DWRITE_FONT_WEIGHT_NORMAL, theme.TextSecondary(),
                              DWRITE_TEXT_ALIGNMENT_LEADING,
                              DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

            ++lineIndex;
            if (nl == std::wstring::npos) break;
            pos = nl + 1;
        }
    }
}

} // namespace ModernDesign