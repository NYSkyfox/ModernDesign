#pragma once

#include <d2d1_1.h>
#include <algorithm>
#include <cmath>

namespace ModernDesign {

// ============================================================
// 几何类型
// ============================================================

// 浮点矩形（DIP 坐标）
struct RectF {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;

    RectF() = default;
    RectF(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

    float Left() const { return x; }
    float Top() const { return y; }
    float Right() const { return x + w; }
    float Bottom() const { return y + h; }
    float CenterX() const { return x + w * 0.5f; }
    float CenterY() const { return y + h * 0.5f; }
    bool IsEmpty() const { return w <= 0.0f || h <= 0.0f; }

    // 点是否在矩形内
    bool Contains(float px, float py) const {
        return px >= x && px < Right() && py >= y && py < Bottom();
    }

    // 外扩/内缩
    RectF Inflate(float d) const { return RectF(x - d, y - d, w + d * 2.0f, h + d * 2.0f); }
    RectF Deflate(float d) const { return Inflate(-d); }

    // 转换为 D2D 矩形
    D2D1_RECT_F ToD2D() const { return D2D1::RectF(x, y, x + w, y + h); }
};

// 浮点点
struct PointF {
    float x = 0.0f, y = 0.0f;
    PointF() = default;
    PointF(float x_, float y_) : x(x_), y(y_) {}
};

// 圆角矩形便捷构造
inline D2D1_ROUNDED_RECT MakeRoundedRect(const RectF& r, float rx, float ry) {
    return D2D1::RoundedRect(r.ToD2D(), rx, ry);
}

// 命中测试（矩形）
inline bool HitRect(const RectF& r, float px, float py) { return r.Contains(px, py); }

} // namespace ModernDesign