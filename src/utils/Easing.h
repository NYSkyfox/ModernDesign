#pragma once

#include <cmath>

namespace ModernDesign {

// ============================================================
// 缓动函数（Easing）
//
// WinUI / WinUIonWeb 的面板与控件动画都写成 CSS 三次贝塞尔，
// 形如 cubic-bezier(x1, y1, x2, y2)。这里给出与浏览器一致的求值：
// 先在 x 方向二分求参数 t（BezierX(t) = x），再代入 y 方向。
// ============================================================

// 三次贝塞尔 Y 值：输入线性进度 x∈[0,1]，输出缓动后的进度 y
inline float CubicBezierY(float x1, float y1, float x2, float y2, float x) {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;

    auto bezierX = [&](float t) {
        float u = 1.0f - t;
        return 3.0f * u * u * t * x1 + 3.0f * u * t * t * x2 + t * t * t;
    };

    // 二分求 t（24 次足够精确到 1e-7）
    float lo = 0.0f, hi = 1.0f, t = x;
    for (int i = 0; i < 24; ++i) {
        t = (lo + hi) * 0.5f;
        if (bezierX(t) < x) lo = t; else hi = t;
    }
    float u = 1.0f - t;
    return 3.0f * u * u * t * y1 + 3.0f * u * t * t * y2 + t * t * t;
}

// ---- WinUI 契约：左侧内联模式（Left / CompactInline）----
// open 200ms / close 200ms，cubic-bezier(0, 0.35, 0.15, 1)
inline float EaseNavInline(float p) { return CubicBezierY(0.0f, 0.35f, 0.15f, 1.0f, p); }

// ---- WinUI 契约：覆盖模式（LeftCompact / LeftMinimal / Overlay）----
// open 350ms / close 120ms，cubic-bezier(0.1, 0.9, 0.2, 1)
inline float EaseNavOverlay(float p) { return CubicBezierY(0.1f, 0.9f, 0.2f, 1.0f, p); }

// ---- 通用 ----
// WinUI 标准缓出：cubic-bezier(0, 0, 0, 1)
inline float EaseStandardOut(float p) { return CubicBezierY(0.0f, 0.0f, 0.0f, 1.0f, p); }

// ---- Flyout / MenuFlyout ----
// 打开：cubic-bezier(0.1, 0.9, 0.2, 1)
inline float EaseFlyoutOpen(float p) { return CubicBezierY(0.1f, 0.9f, 0.2f, 1.0f, p); }
// 关闭（Flit）：cubic-bezier(0.7, 0, 1, 0.5)
inline float EaseFlyoutClose(float p) { return CubicBezierY(0.7f, 0.0f, 1.0f, 0.5f, p); }

} // namespace ModernDesign
