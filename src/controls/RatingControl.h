#pragma once
#include "Geometry.h"
#include "utils/Color.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// RatingControl — 星形评分
//
// 规格来源：WinUI 3 RatingControl
//   - 一排 Star，0~MaximumValue（默认 5），支持半星
//   - Star 20px，项间距 RatingControlItemSpacing=2
//   - 已评 = accent（StarFilled），未评 = 中性灰（Star 描边）
//   - 交互：hover 高亮预览，点击设定值（点星右半 => 半星）
// ============================================================

class RatingControl {
public:
    RatingControl() = default;

    void SetMaximumValue(int n) { max_ = (n < 1) ? 1 : n; if (value_ > max_) value_ = max_; }
    int  MaximumValue() const { return max_; }
    void SetValue(int v) { if (v < 0) v = 0; if (v > max_) v = max_; value_ = v; }
    int  Value() const { return value_; }
    void SetIsReadOnly(bool ro) { readOnly_ = ro; }

    void SetForeground(const Color& c) { fg_ = c; hasFg_ = true; }
    void SetChangeCallback(std::function<void(int)> cb) { cb_ = cb; }

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    bool Update(float dt) { (void)dt; return false; }
    void Draw(Renderer& r, const Theme& t, float s);

    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseUp(float x, float y);

private:
    int max_ = 5;
    int value_ = 0;
    int hover_ = 0;          // 0 = 无悬停预览
    bool readOnly_ = false;
    Color fg_;
    bool hasFg_ = false;
    std::function<void(int)> cb_;
    RectF bounds_;
};

} // namespace ModernDesign