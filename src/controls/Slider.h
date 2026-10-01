#pragma once

#include "Geometry.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// Slider — 滑块
// ============================================================

class Slider {
public:
    Slider() = default;

    void SetValue(float v) { value_ = Clamp01(v); }
    float GetValue() const { return value_; }

    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    void SetValueCallback(std::function<void(float)> cb) { onValue_ = cb; }

    // 输入
    void OnMouseMove(float x, float y);
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);
    void OnMouseLeave();

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    float value_ = 0.5f;
    bool enabled_ = true;
    RectF bounds_;
    std::function<void(float)> onValue_;

    bool dragging_ = false;
    bool hot_ = false;
    float hoverT_ = 0.0f;
    float pressT_ = 0.0f;
};

} // namespace ModernDesign