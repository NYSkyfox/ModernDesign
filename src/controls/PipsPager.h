#pragma once
#include "Geometry.h"
#include "utils/Color.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// PipsPager — 分页圆点指示器
//
// 规格来源：WinUI 3 PipsPager
//   - 一排圆点/药丸，选中项拉伸为长药丸（NavigationButtonWidth=24）
//   - 未选中为小药丸（ButtonWidth=20, ButtonHeight=12）
//   - 选中色 = accent，未选中 = 中性灰
// ============================================================

class PipsPager {
public:
    PipsPager() = default;

    void SetPageIndexCount(int n) { pageCount_ = (n < 1) ? 1 : n; }
    int  PageIndexCount() const { return pageCount_; }
    void SetSelectedIndex(int i) { if (i < 0) i = 0; if (i >= pageCount_) i = pageCount_ - 1; selected_ = i; }
    int  SelectedIndex() const { return selected_; }

    void SetBackground(const Color& c) { bg_ = c; hasBg_ = true; }
    void SetForeground(const Color& c) { fg_ = c; }
    void SetSelectionCallback(std::function<void(int)> cb) { cb_ = cb; }

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    bool Update(float dt) { (void)dt; return false; }
    void Draw(Renderer& r, const Theme& t, float s);

    void OnMouseUp(float x, float y);

private:
    int pageCount_ = 3;
    int selected_ = 0;
    Color bg_;
    bool hasBg_ = false;
    Color fg_;
    bool hasFg_ = false;
    std::function<void(int)> cb_;
    RectF bounds_;
};

} // namespace ModernDesign