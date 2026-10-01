#pragma once

#include "Geometry.h"
#include <functional>
#include <string>

namespace ModernDesign {

class Renderer;
class Theme;

// ============================================================
// RadioButton — 单选按钮
// ============================================================

class RadioButton {
public:
    RadioButton() = default;

    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }

    void SetSelected(bool selected) { selected_ = selected; }
    bool IsSelected() const { return selected_; }

    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    void SetSelectedCallback(std::function<void()> cb) { onSelected_ = cb; }

    // 输入
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    std::wstring text_ = L"Option";
    bool selected_ = false;
    bool enabled_ = true;
    RectF bounds_;
    std::function<void()> onSelected_;

    bool hot_ = false;
    bool pressed_ = false;
    float hoverT_ = 0.0f;
    float pressedT_ = 0.0f;
    float checkT_ = 0.0f;
};

} // namespace ModernDesign