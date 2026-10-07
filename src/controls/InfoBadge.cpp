#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/InfoBadge.h"

namespace ModernDesign {

// WinUI 3 InfoBadge 规格
static constexpr float kValueFontSize = 11.0f;   // InfoBadgeValueFontSize
static constexpr float kIconSize = 12.0f;        // InfoBadgeIconWidth/Height
static constexpr float kPadH = 4.0f;             // 左右内边距

static const wchar_t* kFace = L"Segoe UI";

void InfoBadge::Draw(Renderer& r, const Theme& t, float s) {
    if (!visible_ || bounds_.IsEmpty()) return;
    if (content_ == Content::Text && text_.empty()) return;

    float h = bounds_.h;
    Color bg = hasBg_ ? bg_ : t.Accent();
    Color fg = fg_;

    // 圆角 = h/2（胶囊/圆形）
    r.FillRoundedRect(bounds_, h * 0.5f, bg);

    if (content_ == Content::Value) {
        std::wstring txt;
        if (value_ > maxCount_) {
            wchar_t buf[16] = {};
            swprintf_s(buf, L"%d+", maxCount_);
            txt = buf;
        } else {
            wchar_t buf[16] = {};
            swprintf_s(buf, L"%d", value_);
            txt = buf;
        }
        r.DrawTextCentered(txt, bounds_, kFace, kValueFontSize * s,
                           DWRITE_FONT_WEIGHT_SEMI_BOLD, fg);
    } else if (content_ == Content::Icon) {
        DrawFluentIconCentered(r, icon_, bounds_, kIconSize * s, fg);
    } else { // Text
        r.DrawTextCentered(text_, bounds_, kFace, kValueFontSize * s,
                           DWRITE_FONT_WEIGHT_SEMI_BOLD, fg);
    }
}

} // namespace ModernDesign