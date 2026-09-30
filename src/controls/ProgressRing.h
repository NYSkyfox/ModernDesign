#pragma once

#include "Geometry.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// ProgressRing — 环形进度
// ============================================================

class ProgressRing {
public:
    ProgressRing() = default;

    void SetProgress(float p) { progress_ = Clamp01(p); }
    float GetProgress() const { return progress_; }

    void SetIndeterminate(bool ind) { indeterminate_ = ind; }
    bool IsIndeterminate() const { return indeterminate_; }

    void SetSize(float size) { size_ = size; }
    float GetSize() const { return size_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    float progress_ = 0.0f;
    bool indeterminate_ = false;
    float size_ = 32.0f;
    RectF bounds_;
    float animT_ = 0.0f;
};

} // namespace ModernDesign