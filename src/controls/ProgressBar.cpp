#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ProgressBar.h"

namespace ModernDesign {

static constexpr float kBarTrackH = 1.0f;   // track 1px
static constexpr float kBarFillH = 3.0f;    // indicator 3px

// 分段线性插值（模拟 CSS keyframes），points 为 (t, x) 升序
static float PiecewiseX(float t, const float* ts, const float* xs, int n) {
    if (t <= ts[0]) return xs[0];
    for (int i = 1; i < n; ++i) {
        if (t <= ts[i]) {
            float span = ts[i] - ts[i - 1];
            float f = (span > 1e-6f) ? (t - ts[i - 1]) / span : 0.0f;
            return LerpF(xs[i - 1], xs[i], f);
        }
    }
    return xs[n - 1];
}

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

    // ===== WinUIonWeb 精确规格 (ProgressBar.vue + theme.css) =====
    // track 1px, bg --ctrl-strong-stroke(45%黑/54%白), radius 0.5
    // determinate indicator 3px, accent-base, radius 1.5, width = progress%
    // indeterminate: 两条 40%/60% 宽, 3px 高, 2s 循环动画, track 隐藏
    Color trackBg = theme.lightMode ? Color(0, 0, 0, 0.45f) : Color(1, 1, 1, 0.54f);
    Color fillColor = accent;

    float w = bounds_.w;
    float cy = bounds_.y + bounds_.h * 0.5f;

    // 轨道（仅确定模式可见；不确定模式 CSS 令 opacity:0）
    if (!indeterminate_) {
        float trackH = kBarTrackH * s;
        RectF trackRect(bounds_.x, cy - trackH * 0.5f, w, trackH);
        renderer.FillRoundedRect(trackRect, trackH * 0.5f, trackBg);
    }

    float fillH = kBarFillH * s;
    float fillY = cy - fillH * 0.5f;

    if (indeterminate_) {
        // 2s 周期循环
        float t = std::fmod(animT_ / 2.0f, 1.0f);

        // indicator1: 宽 40%W。keyframes: 0%→-100%, 75%→300%, 100%→300%
        // translateX 百分比相对自身宽度(0.4W) → 左边缘(占W比例):
        // 0%: -0.40, 75%: -0.40+3*0.40=0.80, 100%: 0.80
        const float t1[] = {0.0f, 0.75f, 1.0f};
        const float x1[] = {-0.40f, 0.80f, 0.80f};
        float p1 = PiecewiseX(t, t1, x1, 3) * w;   // 左边缘像素
        float iw1 = w * 0.40f;

        // indicator2: 宽 60%W。keyframes: 0%→-150%, 37.5%→-150%, 100%→166%
        // 左边缘(占W比例): 0%: -0.90, 37.5%: -0.90, 100%: -0.90+1.66*0.60=0.096
        const float t2[] = {0.0f, 0.375f, 1.0f};
        const float x2[] = {-0.90f, -0.90f, 0.096f};
        float p2 = PiecewiseX(t, t2, x2, 3) * w;
        float iw2 = w * 0.60f;

        // 裁剪到 [bounds_.x, bounds_.x + w]
        auto drawClipped = [&](float px, float iw) {
            float l = FzMx(bounds_.x, px);
            float r = FzMn(bounds_.x + w, px + iw);
            if (r - l > 0.5f) {
                renderer.FillRoundedRect(RectF(l, fillY, r - l, fillH), fillH * 0.5f, fillColor);
            }
        };
        drawClipped(p2, iw2);   // 先画 indicator2（底层）
        drawClipped(p1, iw1);   // 再画 indicator1（上层）
    } else {
        // 确定模式
        float fillW = w * progress_;
        if (fillW > 0.5f) {
            renderer.FillRoundedRect(RectF(bounds_.x, fillY, fillW, fillH), fillH * 0.5f, fillColor);
        }
    }
}

} // namespace ModernDesign