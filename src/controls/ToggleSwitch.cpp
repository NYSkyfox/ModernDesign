#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/ToggleSwitch.h"

namespace ModernDesign {

static constexpr float kToggleFontSize = 14.0f;
static constexpr float kTogglePad = 12.0f;

void ToggleSwitch::OnMouseMove(float x, float y) {
    if (!enabled_) return;
    hot_ = bounds_.Contains(x, y);
}

void ToggleSwitch::OnMouseLeave() { hot_ = false; }

void ToggleSwitch::OnMouseDown(float x, float y) {
    (void)x; (void)y;
}

void ToggleSwitch::OnMouseUp(float x, float y) {
    if (!enabled_) return;
    if (bounds_.Contains(x, y)) {
        isOn_ = !isOn_;
        if (onChanged_) onChanged_(isOn_);
    }
}

bool ToggleSwitch::Update(float dt) {
    float target = (isOn_ && enabled_) ? 1.0f : 0.0f;
    toggleT_ = Approach(toggleT_, target, dt, 20.0f);

    float hoverTarget = (hot_ && enabled_) ? 1.0f : 0.0f;
    hoverT_ = Approach(hoverT_, hoverTarget, dt, 20.0f);

    return std::abs(toggleT_ - target) > 0.005f
        || std::abs(hoverT_ - hoverTarget) > 0.005f;
}

void ToggleSwitch::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    float hT = EaseOut(hoverT_);
    float tT = EaseOut(toggleT_);

    // ===== WinUI 3（Windows 11 Fluent）ToggleSwitch 规范（照抄 FluentZero）=====
    const float trackW = 40.0f * s, trackH = 20.0f * s;
    // track 在右侧，文字在左侧
    float tx = bounds_.x + bounds_.w - trackW - kTogglePad * s;
    float ty = bounds_.y + (bounds_.h - trackH) * 0.5f;

    const float inset = 3.0f * s;               // 滑块到轨道内壁间隙
    // 滑块：12px（关）→ 14px（开），Win11 开态放大；纯白、无投影、无描边
    float knobD = trackH - 8.0f * s + 2.0f * s * tT;
    float knobR = knobD * 0.5f;
    // 滑块中心：关=左，开=右，按 toggleT 滑动
    float knobX = tx + inset + knobR + tT * (trackW - 2 * inset - knobD);
    float knobY = ty + trackH * 0.5f;

    // 轨道配色：
    //   OFF 填充 = 浅色白底 / 深色 #202020，hover 微调
    //   ON  填充 = accent 实底，hover 提亮 12%
    //   OFF 描边 = 37% 黑（浅）/ 22% 白（深），hover 加深
    //   ON  描边 = 深一度 accent
    Color off, on, offEdge, onEdge, knobOff;
    Color accent = theme.Accent();
    if (!enabled_) {
        off = theme.lightMode ? Color(0.92f, 0.92f, 0.92f, 1) : Color(0.24f, 0.24f, 0.24f, 1);
        on  = accent;
        offEdge = theme.lightMode ? Color(0, 0, 0, 0.15f) : Color(1, 1, 1, 0.10f);
        onEdge  = offEdge;
        knobOff = theme.lightMode ? Color(0.62f, 0.62f, 0.62f, 1) : Color(0.55f, 0.55f, 0.55f, 1);
    } else {
        off = theme.lightMode
            ? Color::Lerp(Color(1, 1, 1, 1), Color(0.96f, 0.96f, 0.96f, 1), hT)
            : Color::Lerp(Color(0.2f, 0.2f, 0.2f, 1), Color(0.24f, 0.24f, 0.24f, 1), hT);
        on = accent;
        on = Color::Lerp(on, accent.Lighten(0.12f), hT);   // ON hover 提亮
        offEdge = theme.lightMode
            ? Color(0, 0, 0, 0.37f).Darken(0.10f * hT)     // 浅色 hover 加深
            : Color(1, 1, 1, 0.22f).Lighten(0.10f * hT);   // 深色 hover 提亮
        onEdge  = on.Darken(0.12f);
        knobOff = theme.lightMode ? Color(0.45f, 0.45f, 0.45f, 1) : Color(0.70f, 0.70f, 0.70f, 1);
    }

    RectF trackRect(tx, ty, trackW, trackH);
    float radius = trackH * 0.5f;

    // 轨道底（off 色）
    renderer.FillRoundedRect(trackRect, radius, off);
    // ON 的 accent 叠层：按 tT 淡入
    if (tT > 0.003f) {
        renderer.FillRoundedRect(trackRect, radius, Color(on.r, on.g, on.b, tT));
    }
    // 描边（off→on 边色插值）
    Color edge = Color::Lerp(offEdge, onEdge, tT);
    renderer.StrokeRoundedRect(trackRect, radius, 1.0f, edge);

    // 滑块：OFF 灰色实心圆 → ON 纯白（禁用态用更浅灰）
    Color knobFill = Color::Lerp(knobOff, Color(1, 1, 1, 1), tT);
    RectF knobRect(knobX - knobR, knobY - knobR, knobD, knobD);
    renderer.FillEllipse(knobRect, knobFill);

    // 文字（左侧）
    Color textCol = theme.TextPrimary();
    if (!enabled_) textCol = theme.lightMode ? Color(0, 0, 0, 0.36f) : Color(1, 1, 1, 0.36f);
    RectF textRect(bounds_.x, bounds_.y,
                   bounds_.w - (trackW + kTogglePad * s),
                   bounds_.h);
    renderer.DrawText(text_,
                      textRect.x, textRect.y, textRect.w, textRect.h,
                      L"Segoe UI", kToggleFontSize * s,
                      DWRITE_FONT_WEIGHT_NORMAL, textCol,
                      DWRITE_TEXT_ALIGNMENT_LEADING,
                      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

} // namespace ModernDesign