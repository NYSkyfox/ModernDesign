#include "pch.h"
#include "controls/ProgressBar.h"

namespace ModernDesign {

static constexpr float kBarHeight = 4.0f;
static constexpr float kBarRadius = 2.0f;

bool ProgressBar::Update(float dt) {
    if (indeterminate_) {
        animT_ += dt;
        return true;  // 不确定模式持续动画
    }
    return false;
}

void ProgressBar::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    Color accent = theme.Accent();
    Color trackBg = theme.lightMode ? Color(0,0,0,0.12f) : Color(1,1,1,0.24f);

    float barY = bounds_.y + bounds_.h * 0.5f - kBarHeight * s * 0.5f;
    RectF barRect(bounds_.x, barY, bounds_.w, kBarHeight * s);

    // 轨道
    renderer.FillRoundedRect(barRect, kBarRadius * s, trackBg);

    // 进度
    if (indeterminate_) {
        // 不确定模式：循环动画
        float cycle = std::fmod(animT_ * 0.5f, 1.0f);
        float barW = bounds_.w * 0.3f;
        float x = bounds_.x + (bounds_.w - barW) * cycle;
        RectF indRect(x, barY, barW, kBarHeight * s);
        renderer.FillRoundedRect(indRect, kBarRadius * s, accent);
    } else {
        // 确定模式
        float fillW = bounds_.w * progress_;
        if (fillW > 0.0f) {
            RectF fillRect(bounds_.x, barY, fillW, kBarHeight * s);
            renderer.FillRoundedRect(fillRect, kBarRadius * s, accent);
        }
    }
}

} // namespace ModernDesign