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
    float toggleT_ = 0.0f;
};

} // namespace ModernDesign