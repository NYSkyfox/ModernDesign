#pragma once
#include "Geometry.h"
#include "utils/Color.h"
#include "utils/FluentIcons.h"
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// InfoBadge — 信息徽标
//
// 规格来源：WinUI 3 InfoBadge
//   - 圆形/胶囊底，accent 或自定义色
//   - 内容：数字(0-99) / 图标 / 文字
//   - 最小 16x16，字号 11，圆角 = h/2
// ============================================================

class InfoBadge {
public:
    enum class Content { Value, Icon, Text };

    InfoBadge() = default;

    void SetValue(int v) { value_ = v; content_ = Content::Value; }
    void SetIcon(FluentIcon icon) { icon_ = icon; content_ = Content::Icon; }
    void SetText(const std::wstring& t) { text_ = t; content_ = Content::Text; }
    void SetBackground(const Color& c) { bg_ = c; hasBg_ = true; }
    void SetForeground(const Color& c) { fg_ = c; }
    void SetBadgeMaxCount(int m) { maxCount_ = m; }
    void SetVisibility(bool v) { visible_ = v; }
    bool IsVisible() const { return visible_; }

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    bool Update(float dt) { (void)dt; return false; }
    void Draw(Renderer& r, const Theme& t, float s);

private:
    int value_ = 0;
    FluentIcon icon_ = FluentIcon::Info;
    std::wstring text_;
    Content content_ = Content::Value;
    Color bg_;
    bool hasBg_ = false;
    Color fg_{1.0f, 1.0f, 1.0f, 1.0f};
    int maxCount_ = 99;
    bool visible_ = true;
    RectF bounds_;
};

} // namespace ModernDesign
