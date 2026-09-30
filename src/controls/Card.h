#pragma once

#include "Geometry.h"
#include <string>

namespace ModernDesign {

// ============================================================
// Card — 卡片
// 圆角底色 + 1px 描边 + 标题 + 内容
// ============================================================

class Card {
public:
    Card() = default;

    void SetTitle(const std::wstring& title) { title_ = title; }
    const std::wstring& GetTitle() const { return title_; }

    void SetContent(const std::wstring& content) { content_ = content; }
    const std::wstring& GetContent() const { return content_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    void SetElevation(float e) { elevation_ = e; }
    float GetElevation() const { return elevation_; }

    // 计算所需高度（用于自适应布局）
    float MeasureHeight(const Theme& theme, float scale) const;

    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    std::wstring title_;
    std::wstring content_;
    RectF bounds_;
    float elevation_ = 1.0f;
};

} // namespace ModernDesign