#pragma once

#include "Geometry.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// ProgressBar — 进度条
// ============================================================

class ProgressBar {
public:
    ProgressBar() = default;

    void SetProgress(float p) { progress_ = Clamp01(p); }
    float GetProgress() const { return progress_; }

    void SetIndeterminate(bool ind) { indeterminate_ = ind; }
    bool IsIndeterminate() const { return indeterminate_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    float progress_ = 0.0f;
    bool indeterminate_ = false;
    RectF bounds_;
    float animT_ = 0.0f;
};

} // namespace ModernDesign