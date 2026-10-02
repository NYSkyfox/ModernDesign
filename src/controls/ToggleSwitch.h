#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>

namespace ModernDesign {

// ============================================================
// ToggleSwitch — 开关
// ============================================================

class ToggleSwitch {
public:
    ToggleSwitch() = default;

    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }

    void SetIsOn(bool on) { isOn_ = on; }
    bool IsOn() const { return isOn_; }

    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    void SetChangedCallback(std::function<void(bool)> cb) { onChanged_ = cb; }

    // 输入
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    std::wstring text_ = L"Toggle";
    bool isOn_ = false;
    bool enabled_ = true;
    RectF bounds_;
    std::function<void(bool)> onChanged_;

    bool hot_ = false;
    bool pressed_ = false;
    float toggleT_ = 0.0f;
    float hoverT_ = 0.0f;
    float pressedT_ = 0.0f;
    // 滑动动画（固定时长、定时驱动，避免指数趋近的生硬感）
    float toggleFrom_ = 0.0f;
    float toggleTo_ = 0.0f;
    float toggleElapsed_ = 0.0f;
    bool  toggleAnim_ = false;
    static constexpr float kToggleDur = 0.15f;
};

} // namespace ModernDesign