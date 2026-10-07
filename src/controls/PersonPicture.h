#pragma once
#include "Geometry.h"
#include "utils/Color.h"
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// PersonPicture — 人物头像
//
// 规格来源：WinUI 3 PersonPicture
//   - 圆形头像，无图片时显示 PersonCircle 图标占位
//   - 底 = 中性填充，图标 = 次级文本色
//   - 可选状态徽章（右下角圆点，EllipseBadgeStrokeThickness=2）
// ============================================================

class PersonPicture {
public:
    enum class Badge { None, Available, Busy, Away, DoNotDisturb, Offline };

    PersonPicture() = default;

    void SetBadge(Badge b) { badge_ = b; }
    void SetBackground(const Color& c) { bg_ = c; hasBg_ = true; }
    void SetForeground(const Color& c) { fg_ = c; }

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    bool Update(float dt) { (void)dt; return false; }
    void Draw(Renderer& r, const Theme& t, float s);

private:
    static Color BadgeColor(Badge b, bool light);

    Badge badge_ = Badge::None;
    Color bg_;
    bool hasBg_ = false;
    Color fg_{1.0f, 1.0f, 1.0f, 1.0f};
    RectF bounds_;
};

} // namespace ModernDesign