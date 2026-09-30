#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>

namespace ModernDesign {

// ============================================================
// CheckBox — 复选框
// ============================================================

class CheckBox {
public:
    CheckBox() = default;

    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }

    void SetChecked(bool checked) { checked_ = checked; }
    bool IsChecked() const { return checked_; }
    void SetTristate(bool tri) { triState_ = tri; }
    bool IsTristate() const { return triState_; }

    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }

    void SetBounds(const RectF& bounds) { bounds_ = bounds; }
    const RectF& GetBounds() const { return bounds_; }

    void SetCheckCallback(std::function<void(bool)> cb) { checkCallback_ = cb; }

    // 输入
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    // 动画
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    std::wstring text_ = L"CheckBox";
    bool checked_ = false;
    bool triState_ = false;
    bool enabled_ = true;
    RectF bounds_;
    std::function<void(bool)> checkCallback_;

    bool hot_ = false;
    float hoverT_ = 0.0f;
    bool animating_ = false;
};

} // namespace ModernDesign