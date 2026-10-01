#pragma once

#include "Geometry.h"
#include "utils/Color.h"

namespace ModernDesign {

// ============================================================
// 浮出层定位（Flyout / MenuFlyout / ToolTip 共用）
//
// 规格来源：WinUIonWeb Flyout.vue 的 updatePosition()
//   margin = 8   —— 与窗口四边的最小距离
//   gap    = 6   —— 锚点与浮出层之间的间距（ToolTip 例外，用 20）
//   下方空间不足且上方更宽裕 → 翻转到上方；随后把结果夹回窗口内。
// ============================================================
enum class PopupPlacement {
    Top,
    Bottom,
    Left,
    Right,
    Full,
    TopEdgeAlignedLeft,
    TopEdgeAlignedRight,
    BottomEdgeAlignedLeft,
    BottomEdgeAlignedRight,
};

struct PopupLayout {
    RectF          rect;                    // 最终矩形（已夹到窗口内）
    PopupPlacement actual = PopupPlacement::Bottom;  // 实际方向（可能已翻转）
    bool           above = false;           // 是否在锚点上方
};

inline PopupLayout PlacePopup(PopupPlacement mode, const RectF& anchor,
                              float w, float h, const RectF& bounds,
                              float gap, float margin) {
    PopupLayout out;
    out.actual = mode;

    if (mode == PopupPlacement::Full) {
        out.rect = bounds;
        return out;
    }

    const bool prefTop = (mode == PopupPlacement::Top ||
                          mode == PopupPlacement::TopEdgeAlignedLeft ||
                          mode == PopupPlacement::TopEdgeAlignedRight);
    const bool prefBottom = (mode == PopupPlacement::Bottom ||
                             mode == PopupPlacement::BottomEdgeAlignedLeft ||
                             mode == PopupPlacement::BottomEdgeAlignedRight);

    float x = anchor.x;
    float y = anchor.y;

    if (prefTop || prefBottom) {
        const float spaceBelow = bounds.Bottom() - anchor.Bottom() - gap - margin;
        const float spaceAbove = anchor.y - bounds.y - gap - margin;

        bool up = prefTop;
        if (prefBottom && h > spaceBelow && spaceAbove > spaceBelow) up = true;
        else if (prefTop && h > spaceAbove && spaceBelow > spaceAbove) up = false;

        y = up ? (anchor.y - gap - h) : (anchor.Bottom() + gap);
        out.above = up;

        if (mode == PopupPlacement::TopEdgeAlignedRight ||
            mode == PopupPlacement::BottomEdgeAlignedRight)
            x = anchor.Right() - w;
        else if (mode == PopupPlacement::TopEdgeAlignedLeft ||
                 mode == PopupPlacement::BottomEdgeAlignedLeft)
            x = anchor.x;
        else
            x = anchor.x + (anchor.w - w) * 0.5f;   // 居中

        out.actual = up ? PopupPlacement::Top : PopupPlacement::Bottom;
    } else {
        // Left / Right（默认垂直居中）
        const float spaceRight = bounds.Right() - anchor.Right() - gap - margin;
        const float spaceLeft = anchor.x - bounds.x - gap - margin;

        bool toLeft = (mode == PopupPlacement::Left);
        if (mode == PopupPlacement::Right && w > spaceRight && spaceLeft > spaceRight)
            toLeft = true;

        x = toLeft ? (anchor.x - gap - w) : (anchor.Right() + gap);
        y = anchor.y + (anchor.h - h) * 0.5f;
        out.actual = toLeft ? PopupPlacement::Left : PopupPlacement::Right;
    }

    // 夹回窗口内（Flyout.vue:132-133）
    x = ClampF(x, bounds.x + margin,
               FzMx(bounds.x + margin, bounds.Right() - margin - w));
    y = ClampF(y, bounds.y + margin,
               FzMx(bounds.y + margin, bounds.Bottom() - margin - h));

    out.rect = RectF(x, y, w, h);
    return out;
}

} // namespace ModernDesign