#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ProgressRing.h"
#include <cmath>

namespace ModernDesign {

// 绘制圆弧（用线段近似，避免 D2D 路径复杂化）
static void DrawArc(Renderer& r, const RectF& bounds, float startAngle,
                    float sweepAngle, float strokeWidth, const Color& c) {
    if (std::fabs(sweepAngle) < 0.01f || std::fabs(sweepAngle) >= 359.9f) return;

    const float cx = bounds.x + bounds.w * 0.5f;
    const float cy = bounds.y + bounds.h * 0.5f;
    const float radius = (FzMn(bounds.w, bounds.h) - strokeWidth) * 0.5f;
    if (radius <= 0.0f) return;

    // 线段数按角度估算
    int segments = static_cast<int>(std::fabs(sweepAngle) / 6.0f) + 2;
    segments = ClampF(segments, 4, 180);

    const float startRad = startAngle * kPi / 180.0f;
    const float sweepRad = sweepAngle * kPi / 180.0f;

    float prevX = cx + std::cos(startRad) * radius;
    float prevY = cy + std::sin(startRad) * radius;

    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        float angle = startRad + sweepRad * t;
        float x = cx + std::cos(angle) * radius;
        float y = cy + std::sin(angle) * radius;
        r.DrawLine(prevX, prevY, x, y, strokeWidth, c);
        prevX = x;
        prevY = y;
    }
}

bool ProgressRing::Update(float dt) {
    if (indeterminate_) {
        animT_ += dt;
        return true;
    }
    return false;
}

void ProgressRing::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    float diameter = size_ * s;
    float strokeWidth = FzMx(2.0f * s, diameter * 0.08f);

    // 居中放置
    RectF ringRect(
        bounds_.x + (bounds_.w - diameter) * 0.5f,
        bounds_.y + (bounds_.h - diameter) * 0.5f,
        diameter, diameter);

    Color accent = theme.Accent();
    Color trackBg = theme.lightMode ? Color(0, 0, 0, 0.12f)
                                    : Color(1, 1, 1, 0.24f);

    // 轨道圆（完整圆用 ellipse 描边近似）
    renderer.StrokeEllipse(ringRect, strokeWidth, trackBg);

    if (indeterminate_) {
        // 不确定：旋转弧
        float rotation = std::fmod(animT_ * 270.0f, 360.0f);
        DrawArc(renderer, ringRect, rotation, 90.0f, strokeWidth, accent);
    } else if (progress_ > 0.001f) {
        // 确定：固定起点顺时针
        DrawArc(renderer, ringRect, -90.0f, progress_ * 360.0f, strokeWidth, accent);
    }
}

} // namespace ModernDesign