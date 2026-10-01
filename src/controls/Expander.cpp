#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Expander.h"

namespace ModernDesign {

static constexpr float kFontSize = 14.0f;
static constexpr float kDescFontSize = 12.0f;

RectF Expander::HeaderRect(float s) const {
    return RectF(bounds_.x, bounds_.y, bounds_.w, kHeaderH * s);
}

RectF Expander::ContentArea(float s) const {
    // 含 padding 的内容区（header 下方到边框底部）
    float top = bounds_.y + kHeaderH * s;
    return RectF(bounds_.x, top, bounds_.w, FzMx(0.0f, bounds_.h - kHeaderH * s));
}

float Expander::FullHeight(float s) const {
    return kHeaderH * s + contentH_ * s;
}

void Expander::OnMouseMove(float x, float y) {
    hot_ = bounds_.Contains(x, y);
    if (isExpanded_ && contentMove_) contentMove_(x, y);
}

void Expander::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
    if (contentLeave_) contentLeave_(0, 0);
}

void Expander::OnMouseDown(float x, float y) {
    if (!bounds_.Contains(x, y)) return;
    pressed_ = true;
    if (isExpanded_ && contentDown_) contentDown_(x, y);
}

void Expander::OnMouseUp(float x, float y) {
    bool wasPressed = pressed_;
    pressed_ = false;
    if (isExpanded_ && contentUp_) contentUp_(x, y);
    // header 区域点击 → 折叠/展开
    if (wasPressed && bounds_.Contains(x, y)) {
        RectF hr = HeaderRect(1.0f);
        if (hr.Contains(x, y)) {
            isExpanded_ = !isExpanded_;
            if (onExpanded_) onExpanded_(isExpanded_);
        }
    }
}

bool Expander::Update(float dt) {
    float expTarget = isExpanded_ ? 1.0f : 0.0f;
    expandT_ = Approach(expandT_, expTarget, dt, 20.0f);
    float hotTarget = hot_ ? 1.0f : 0.0f;
    hotT_ = Approach(hotT_, hotTarget, dt, 20.0f);
    float pressTarget = pressed_ ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);

    bool anim = std::abs(expandT_ - expTarget) > 0.005f
             || std::abs(hotT_ - hotTarget) > 0.005f
             || std::abs(pressT_ - pressTarget) > 0.005f;
    if (isExpanded_ && contentUpdate_) anim = anim || contentUpdate_(dt);
    return anim;
}

void Expander::Draw(Renderer& renderer, const Theme& theme, float scale) {
    if (bounds_.IsEmpty()) return;

    float s = scale;
    bool light = theme.lightMode;

    // ===== WinUIonWeb 精确规格 (ExpanderBase.vue + theme.css) =====
    // 浅色: card-bg(255,255,255,0.70) card-stroke(0,0,0,0.06)
    //       card-bg-secondary(246,246,246,0.50) stroke-divider(0,0,0,0.06)
    //       subtle-secondary(0,0,0,0.0373) subtle-tertiary(0,0,0,0.0241)
    // 深色: card-bg(255,255,255,0.0512) card-stroke(0,0,0,0.10)
    //       card-bg-secondary(255,255,255,0.0326) stroke-divider(255,255,255,0.08)
    //       subtle-secondary(255,255,255,0.0605) subtle-tertiary(255,255,255,0.0419)
    Color cardBg   = light ? Color(1, 1, 1, 0.70f)   : Color(1, 1, 1, 0.0512f);
    Color cardStroke = light ? Color(0, 0, 0, 0.06f) : Color(0, 0, 0, 0.10f);
    Color cardBg2  = light ? Color(0.965f, 0.965f, 0.965f, 0.50f) : Color(1, 1, 1, 0.0326f);
    Color divider  = light ? Color(0, 0, 0, 0.06f) : Color(1, 1, 1, 0.08f);
    Color subtleHov  = light ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f);
    Color subtlePress = light ? Color(0, 0, 0, 0.0241f) : Color(1, 1, 1, 0.0419f);
    Color textPrimary = theme.TextPrimary();
    Color textSecondary = theme.TextSecondary();

    float eT = EaseOut(expandT_);
    float radius = 4.0f * s;

    float fullBlockH = FzMx(kHeaderH, contentH_) * s;   // 内容块自然高(至少 header 高)
    float visibleBlockH = fullBlockH * eT;              // 当前可见内容高
    float drawnH = kHeaderH * s + visibleBlockH;        // 实际绘制高度
    // 外框用实际绘制高度（收起时只占 header）
    RectF outer(bounds_.x, bounds_.y, bounds_.w, drawnH);

    // ---- 内容区背景（先画，被 header 覆盖上沿）----
    float contentTop = bounds_.y + kHeaderH * s;
    if (visibleBlockH > 1.0f) {
        RectF contentRect(bounds_.x, contentTop, bounds_.w, visibleBlockH);
        renderer.FillRoundedRect(contentRect, radius, cardBg2);
    }

    // ---- header 背景 ----
    RectF header = RectF(bounds_.x, bounds_.y, bounds_.w, kHeaderH * s);
    renderer.FillRoundedRect(header, radius, cardBg);

    // ---- 分隔线（展开时）----
    if (eT > 0.5f) {
        float dy = contentTop;
        renderer.DrawLine(bounds_.x + 1.0f * s, dy, bounds_.x + bounds_.w - 1.0f * s, dy, 1.0f * s, divider);
    }

    // ---- 外框 ----
    renderer.StrokeRoundedRect(outer, radius, 1.0f * s, cardStroke);

    // ---- header 文字（左，padding 16）----
    float pad = kPad * s;
    float textLeft = bounds_.x + pad;
    float chevronW = 32.0f * s;
    float textMaxW = bounds_.w - pad - chevronW - pad;
    float headerCY = bounds_.y + kHeaderH * s * 0.5f;

    if (!description_.empty()) {
        // 两行：标题(上) + 描述(下)
        float lh = 20.0f * s;
        renderer.DrawText(header_, textLeft, headerCY - lh, textMaxW, lh,
                          L"Segoe UI", kFontSize * s, DWRITE_FONT_WEIGHT_NORMAL, textPrimary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        renderer.DrawText(description_, textLeft, headerCY, textMaxW, lh,
                          L"Segoe UI", kDescFontSize * s, DWRITE_FONT_WEIGHT_NORMAL, textSecondary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    } else {
        renderer.DrawText(header_, textLeft, bounds_.y, textMaxW, kHeaderH * s,
                          L"Segoe UI", kFontSize * s, DWRITE_FONT_WEIGHT_NORMAL, textPrimary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    // ---- 箭头（右侧 32x32 区域，hover 时圆角背景）----
    float chevCX = bounds_.x + bounds_.w - chevronW * 0.5f;
    float chevCY = bounds_.y + kHeaderH * s * 0.5f;
    if (hotT_ > 0.001f) {
        Color cb = pressed_ ? subtlePress : subtleHov;
        cb = cb.WithAlpha(hotT_);
        float cs = 32.0f * s;
        renderer.FillRoundedRect(RectF(chevCX - cs * 0.5f, chevCY - cs * 0.5f, cs, cs), 4.0f * s, cb);
    }
    // chevron：收起指向下(▼)，展开旋转 180° 指向上(▲)
    float ang = kPi * eT;
    float aw = 3.5f * s, ah = 2.5f * s;
    // 收起时的下箭头三点
    float p1x = chevCX - aw, p1y = chevCY - ah;
    float p2x = chevCX,     p2y = chevCY + ah;
    float p3x = chevCX + aw, p3y = chevCY - ah;
    float cosa = std::cos(ang), sina = std::sin(ang);
    auto rot = [&](float px, float py, float& ox, float& oy) {
        float dx = px - chevCX, dy = py - chevCY;
        ox = chevCX + dx * cosa - dy * sina;
        oy = chevCY + dx * sina + dy * cosa;
    };
    float rx1, ry1, rx2, ry2, rx3, ry3;
    rot(p1x, p1y, rx1, ry1); rot(p2x, p2y, rx2, ry2); rot(p3x, p3y, rx3, ry3);
    Color chevCol = textSecondary;
    renderer.DrawLine(rx1, ry1, rx2, ry2, 1.5f * s, chevCol);
    renderer.DrawLine(rx2, ry2, rx3, ry3, 1.5f * s, chevCol);

    // ---- 内容（裁剪到可见区）----
    if (visibleBlockH > 1.0f && contentDraw_) {
        RectF clipRect(bounds_.x, contentTop, bounds_.w, visibleBlockH);
        renderer.PushClip(clipRect);
        float naturalInnerH = FzMx(0.0f, contentH_ - 2.0f * kPad) * s;
        RectF inner(bounds_.x + pad, contentTop + pad, bounds_.w - 2.0f * pad, naturalInnerH);
        contentDraw_(renderer, theme, scale, inner);
        renderer.PopClip();
    }
}

} // namespace ModernDesign