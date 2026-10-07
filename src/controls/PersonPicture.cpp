#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/PersonPicture.h"

namespace ModernDesign {

// WinUI 3 状态色
Color PersonPicture::BadgeColor(Badge b, bool light) {
    switch (b) {
        case Badge::Available:    return Color::Hex(0x00CC00);
        case Badge::Busy:         return Color::Hex(0xFF0000);
        case Badge::Away:         return Color::Hex(0xFFCC00);
        case Badge::DoNotDisturb: return Color::Hex(0xFF0000);
        case Badge::Offline:      return light ? Color::Hex(0x7A7A7A) : Color::Hex(0xA0A0A0);
        default:                  return Color::Hex(0x000000);
    }
}

void PersonPicture::Draw(Renderer& r, const Theme& t, float s) {
    if (bounds_.IsEmpty()) return;

    float d = FzMn(bounds_.w, bounds_.h);   // 正方形边
    RectF box(bounds_.x + (bounds_.w - d) * 0.5f, bounds_.y + (bounds_.h - d) * 0.5f, d, d);

    // 圆底
    Color bg = hasBg_ ? bg_ : (t.lightMode ? Color(0, 0, 0, 0.08f) : Color(1, 1, 1, 0.10f));
    r.FillEllipse(box, bg);

    // 占位人物图标（内缩 ~22%）
    float iconSize = d * 0.66f;
    RectF icon(box.x + (d - iconSize) * 0.5f, box.y + (d - iconSize) * 0.5f, iconSize, iconSize);
    Color fg = fg_.a > 0.0f ? fg_ : t.TextSecondary();
    DrawFluentIconCentered(r, FluentIcon::PersonCircle, icon, iconSize, fg);

    // 状态徽章：右下角圆点，描边 2px（EllipseBadgeStrokeThickness）
    if (badge_ != Badge::None) {
        float bd = d * 0.28f;
        float bx = box.x + d - bd * 0.5f;
        float by = box.y + d - bd * 0.5f;
        RectF badge(bx - bd * 0.5f, by - bd * 0.5f, bd, bd);
        Color ring = hasBg_ ? bg_ : t.WindowBg();
        r.FillEllipse(badge.Inflate(2.0f * s), ring);
        r.FillEllipse(badge, BadgeColor(badge_, t.lightMode));
    }
}

} // namespace ModernDesign