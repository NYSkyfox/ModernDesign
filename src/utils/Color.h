#pragma once

#include <windows.h>
#include <d2d1helper.h>
#include <algorithm>
#include <cmath>

namespace ModernDesign {

// ============================================================
// Color — 颜色类型
// 分量全部为 0.0f ~ 1.0f 浮点（与 D2D1_COLOR_F 一致，免转换）
// ============================================================
struct Color {
    float r, g, b, a;

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float rr, float gg, float bb, float aa = 1.0f)
        : r(rr), g(gg), b(bb), a(aa) {}

    // 从 0xRRGGBB 整数构造（不透明）
    static Color Hex(DWORD rgb) {
        return Color(
            static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
            static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
            static_cast<float>(rgb & 0xFF) / 255.0f,
            1.0f);
    }

    // 从 0xRRGGBB 构造 + 指定 alpha（0~1）
    static Color HexA(DWORD rgb, float alpha) {
        Color c = Hex(rgb);
        c.a = alpha;
        return c;
    }

    // 从 0xAARRGGBB 整数构造
    static Color FromARGB(DWORD argb) {
        return Color(
            static_cast<float>((argb >> 16) & 0xFF) / 255.0f,
            static_cast<float>((argb >> 8) & 0xFF) / 255.0f,
            static_cast<float>(argb & 0xFF) / 255.0f,
            static_cast<float>((argb >> 24) & 0xFF) / 255.0f);
    }

    // 替换 alpha
    Color WithAlpha(float alpha) const { return Color(r, g, b, alpha); }

    // 线性插值
    static Color Lerp(const Color& c1, const Color& c2, float t) {
        t = Clamp01(t);
        return Color(
            c1.r + (c2.r - c1.r) * t,
            c1.g + (c2.g - c1.g) * t,
            c1.b + (c2.b - c1.b) * t,
            c1.a + (c2.a - c1.a) * t);
    }

    // 变亮（朝白色靠拢）
    Color Lighten(float amount) const { return Lerp(*this, Color(1, 1, 1, a), amount); }

    // 变暗（朝黑色靠拢）
    Color Darken(float amount) const { return Lerp(*this, Color(0, 0, 0, a), amount); }

    // 转 D2D 颜色
    D2D1_COLOR_F ToD2D() const { return D2D1::ColorF(r, g, b, a); }

    // 相对亮度（WCAG）
    float Luminance() const {
        auto lin = [](float c) {
            return (c <= 0.03928f) ? (c / 12.92f) : std::pow((c + 0.055f) / 1.055f, 2.4f);
        };
        return 0.2126f * lin(r) + 0.7152f * lin(g) + 0.0722f * lin(b);
    }
};

// ============================================================
// 常量
// ============================================================
constexpr float kPi = 3.14159265358979323846f;

// ============================================================
// 便捷函数
// ============================================================
inline float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
inline float ClampF(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float LerpF(float a, float b, float t) { return a + (b - a) * t; }
inline float FzMx(float a, float b) { return a > b ? a : b; }
inline float FzMn(float a, float b) { return a < b ? a : b; }

// 缓动（缓出三次方，最常用）
inline float EaseOut(float t) {
    t = Clamp01(t);
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

// 帧率无关的指数趋近（用于 hover/press 平滑过渡）
inline float Approach(float current, float target, float dt, float speed) {
    float t = Clamp01(dt * speed);
    return current + (target - current) * t;
}

} // namespace ModernDesign