#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Expander.h"
#include "utils/FluentIcons.h"
#include "utils/Easing.h"

namespace ModernDesign {

// ============================================================
// 规格常量（WinUIonWeb ExpanderBase.vue）
// ============================================================
namespace {
constexpr float kHeaderTextSize  = 14.0f;  // HeaderText FontSize
constexpr float kHeaderLineH     = 20.0f;  // HeaderText LineHeight
constexpr float kDescTextSize    = 12.0f;  // Description FontSize (SettingsCardDescriptionFontSize)
constexpr float kDescLineH       = 16.0f;  // Description LineHeight
constexpr float kHeaderPadX      = 16.0f;  // header padding 0 16
constexpr float kHeaderGap       = 16.0f;  // header gap 16
constexpr float kIconSize        = 20.0f;  // .win-expander-header-icon 20×20
constexpr float kIconMarginLeft  = 2.0f;   // margin: 0 20px 0 2px
constexpr float kIconMarginRight = 20.0f;
constexpr float kChevronSize     = 32.0f;  // .win-expander-chevron 32×32
constexpr float kContentRadius   = 3.0f;   // .win-expander-content radius 0 0 3 3
constexpr float kOuterRadius     = 4.0f;   // .win-expander radius 4
constexpr float kSquareOff       = 3.0f;   // 直角补边宽度（= 圆角半径）

// 规格：transition-timing-function = cubic-bezier(0,0,0,1)
// 该曲线 x(t) = t³，y(t) = 3t² - 2t³；由 x = p 反解得 y = 3·p^(2/3) - 2·p
float EaseStandard(float p) {
    p = Clamp01(p);
    float t = std::cbrt(p);
    return 3.0f * t * t - 2.0f * t * t * t;
}
} // namespace

// ============================================================
// 几何
// ============================================================

float Expander::EasedProgress() const {
    // WinUI 3 ExpandCollapse：FastOutSlowIn = cubic-bezier(0.1, 0.9, 0.2, 1.0)
    return EaseFlyoutOpen(expandT_);
}

// Header 随方向锚定：Down 贴 bounds 顶，Up 贴 bounds 底（与动画无关）
RectF Expander::HeaderRect(float scale) const {
    float hdrH = headerH_ * scale;
    float y = (dir_ == Direction::Up) ? (bounds_.y + bounds_.h - hdrH) : bounds_.y;
    return RectF(bounds_.x, y, bounds_.w, hdrH);
}

// HeaderControls 区域（Header 右侧，Chevron 左边，间隔 gap）
RectF Expander::ControlsRect(float scale) const {
    if (controlsW_ <= 0.0f) return RectF();
    RectF hdr = HeaderRect(scale);
    float padX = kHeaderPadX * scale;
    float chevLeft = bounds_.x + bounds_.w - padX - kChevronSize * scale;
    float mainRight = chevLeft - kHeaderGap * scale;
    float cw = controlsW_ * scale;
    float x = mainRight - cw;
    return RectF(x, hdr.y, cw, hdr.h);
}

// ============================================================
// 输入
// ============================================================

void Expander::OnMouseMove(float x, float y) {
    float s = lastScale_;
    RectF hdr = HeaderRect(s);
    hot_ = hdr.Contains(x, y);
    if (controlsW_ > 0.0f && ControlsRect(s).Contains(x, y)) {
        if (controlsMove_) controlsMove_(x, y);
        return;
    }
    if (isExpanded_ && contentMove_) contentMove_(x, y);
}

void Expander::OnMouseLeave() {
    hot_ = false;
    pressed_ = false;
    if (controlsLeave_) controlsLeave_(0, 0);
    if (contentLeave_) contentLeave_(0, 0);
}

void Expander::OnMouseDown(float x, float y) {
    if (!bounds_.Contains(x, y)) return;
    float s = lastScale_;
    if (!HeaderRect(s).Contains(x, y)) {
        if (isExpanded_ && contentDown_) contentDown_(x, y);
        return;
    }
    // HeaderControls 内的点击交给子控件，不触发折叠
    if (controlsW_ > 0.0f && ControlsRect(s).Contains(x, y)) {
        if (controlsDown_) controlsDown_(x, y);
        return;
    }
    pressed_ = true;
}

void Expander::OnMouseUp(float x, float y) {
    float s = lastScale_;
    bool wasPressed = pressed_;
    pressed_ = false;

    if (controlsW_ > 0.0f && ControlsRect(s).Contains(x, y)) {
        if (controlsUp_) controlsUp_(x, y);
        return;
    }
    // Header 空白区按下并抬起 → 切换展开
    if (wasPressed && HeaderRect(s).Contains(x, y)) {
        isExpanded_ = !isExpanded_;
        if (onExpanded_) onExpanded_(isExpanded_);
        return;
    }
    if (isExpanded_ && contentUp_) contentUp_(x, y);
}

// ============================================================
// 动画
// ============================================================

bool Expander::Update(float dt) {
    // 规格：0.2s cubic-bezier(0,0,0,1) → 线性时间进度，绘制时再套曲线
    float target = isExpanded_ ? 1.0f : 0.0f;
    float speed = 1.0f / kAnimDuration;
    if (expandT_ < target)      expandT_ = FzMn(target, expandT_ + dt * speed);
    else if (expandT_ > target) expandT_ = FzMx(target, expandT_ - dt * speed);

    float hotTarget = hot_ ? 1.0f : 0.0f;
    hotT_ = Approach(hotT_, hotTarget, dt, 20.0f);
    float pressTarget = pressed_ ? 1.0f : 0.0f;
    pressT_ = Approach(pressT_, pressTarget, dt, 30.0f);

    bool anim = std::abs(expandT_ - target) > 0.0005f
             || std::abs(hotT_ - hotTarget) > 0.005f
             || std::abs(pressT_ - pressTarget) > 0.005f;
    if (isExpanded_ && contentUpdate_) anim = anim || contentUpdate_(dt);
    return anim;
}

// ============================================================
// 绘制
// ============================================================

void Expander::Draw(Renderer& renderer, const Theme& theme, float scale) {
    lastScale_ = scale;
    if (bounds_.IsEmpty()) return;

    const float s = scale;
    const bool light = theme.lightMode;

    // ===== WinUIonWeb/styles/theme.css 精确色值 =====
    // 浅色  card-bg(255,255,255,0.70)  card-stroke(0,0,0,0.06)
    //       card-bg-secondary(246,246,246,0.50)  stroke-divider(0,0,0,0.06)
    //       subtle-secondary(0,0,0,0.0373)  subtle-tertiary(0,0,0,0.0241)
    // 深色  card-bg(255,255,255,0.0512) card-stroke(0,0,0,0.10)
    //       card-bg-secondary(255,255,255,0.0326) stroke-divider(255,255,255,0.08)
    //       subtle-secondary(255,255,255,0.0605)  subtle-tertiary(255,255,255,0.0419)
    const Color cardBg     = light ? Color(1, 1, 1, 0.70f)          : Color(1, 1, 1, 0.0512f);
    const Color cardStroke = light ? Color(0, 0, 0, 0.06f)          : Color(0, 0, 0, 0.10f);
    const Color cardBg2    = light ? Color(0.965f, 0.965f, 0.965f, 0.50f)
                                   : Color(1, 1, 1, 0.0326f);
    const Color divider    = light ? Color(0, 0, 0, 0.06f)          : Color(1, 1, 1, 0.08f);
    const Color subtleHov  = light ? Color(0, 0, 0, 0.0373f)        : Color(1, 1, 1, 0.0605f);
    const Color subtlePrs  = light ? Color(0, 0, 0, 0.0241f)        : Color(1, 1, 1, 0.0419f);
    const Color textPrimary   = theme.TextPrimary();
    const Color textSecondary = theme.TextSecondary();

    const bool up = (dir_ == Direction::Up);
    const float radius = kOuterRadius * s;

    const float hdrH = headerH_ * s;
    const float contentNatH = EffectiveContentHeight() * s;
    const float e = EasedProgress();
    const float visibleH = contentNatH * e;
    const float drawnH = hdrH + visibleH;

    // 外框锚定：Down 顶部固定；Up 底部固定（向上生长）
    const float outerY = up ? (bounds_.y + bounds_.h - drawnH) : bounds_.y;
    const RectF outer(bounds_.x, outerY, bounds_.w, drawnH);
    const RectF hdr = HeaderRect(s);
    const RectF cont(bounds_.x, up ? outer.y : (outer.y + hdrH), bounds_.w, visibleH);

    const bool contentVisible = visibleH > 0.5f;

    const float cornerSq = kSquareOff * s;

    // ---- 1. Header 背景：外侧两角圆角(4)、靠内容侧两角直角 ----
    // 做法：圆角矩形「向内容侧延伸」cornerSq，再用 clip 裁回 header 内。
    // 这样内侧圆角被裁掉变直角，且只有一次半透明填充（避免补边叠加导致颜色变深）。
    {
        renderer.PushClip(hdr);
        RectF fill = hdr;
        if (contentVisible) {
            if (up) { fill.y -= cornerSq; fill.h += cornerSq; }  // 内侧在上 → 向上延伸
            else    { fill.h += cornerSq; }                      // 内侧在下 → 向下延伸
        }
        renderer.FillRoundedRect(fill, radius, cardBg);
        renderer.PopClip();
    }

    // ---- 2. Content 背景：radius 3，靠 Header 一侧直角 ----
    if (contentVisible) {
        renderer.PushClip(cont);
        RectF fill = cont;
        if (up) { fill.h += cornerSq; }                          // 内侧在下 → 向下延伸
        else    { fill.y -= cornerSq; fill.h += cornerSq; }      // 内侧在上 → 向上延伸
        renderer.FillRoundedRect(fill, kContentRadius * s, cardBg2);
        renderer.PopClip();
    }

    // ---- 3. 分隔线（展开时 Header 与 Content 之间 1px）----
    if (contentVisible) {
        float dy = up ? cont.Bottom() : cont.y;
        renderer.DrawLine(bounds_.x + 1.0f * s, dy,
                          bounds_.x + bounds_.w - 1.0f * s, dy,
                          1.0f * s, divider);
    }

    // ---- 4. 外框 ----
    renderer.StrokeRoundedRect(outer, radius, 1.0f * s, cardStroke);

    // ---- 5. Header 布局（flex: space-between + gap 16）----
    const float padX = kHeaderPadX * s;
    const float gap = kHeaderGap * s;
    const float chevSize = kChevronSize * s;
    const float chevLeft = bounds_.x + bounds_.w - padX - chevSize;
    float mainRight = chevLeft - gap;

    RectF ctlBox;
    if (controlsW_ > 0.0f) {
        float cw = controlsW_ * s;
        ctlBox = RectF(mainRight - cw, hdr.y, cw, hdr.h);
        mainRight = ctlBox.x - gap;
    }

    float mainLeft = bounds_.x + padX;
    RectF iconBox;
    if (iconDraw_) {
        float d = kIconSize * s;
        iconBox = RectF(mainLeft + kIconMarginLeft * s,
                        hdr.y + (hdr.h - d) * 0.5f, d, d);
        mainLeft = iconBox.x + d + kIconMarginRight * s;
    }

    const float textW = FzMx(0.0f, mainRight - mainLeft);
    const float lineH1 = kHeaderLineH * s;
    const float lineH2 = kDescLineH * s;

    if (!description_.empty()) {
        // 两行：HeaderText(14/20) + Description(12/16)，整体垂直居中
        float blockH = lineH1 + lineH2;
        float top = hdr.y + (hdr.h - blockH) * 0.5f;
        renderer.DrawText(header_, mainLeft, top, textW, lineH1,
                          L"Segoe UI", kHeaderTextSize * s, DWRITE_FONT_WEIGHT_NORMAL, textPrimary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        renderer.DrawText(description_, mainLeft, top + lineH1, textW, lineH2,
                          L"Segoe UI", kDescTextSize * s, DWRITE_FONT_WEIGHT_NORMAL, textSecondary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    } else if (!header_.empty()) {
        float top = hdr.y + (hdr.h - lineH1) * 0.5f;
        renderer.DrawText(header_, mainLeft, top, textW, lineH1,
                          L"Segoe UI", kHeaderTextSize * s, DWRITE_FONT_WEIGHT_NORMAL, textPrimary,
                          DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    if (iconDraw_) iconDraw_(renderer, theme, s, iconBox);
    if (controlsDraw_) controlsDraw_(renderer, theme, s, ctlBox);

    // ---- 6. Chevron ----
    {
        // 箭头朝向：默认(0°)指下；(IsExpanded XOR Up) 时旋转 180° 指上
        const bool flip = (isExpanded_ != up);
        float ang = kPi * (up ? (1.0f - e) : e);
        // 按下时箭头位移 1px（指向下 → 上移；指向上 → 下移）
        float nudge = EaseOut(pressT_) * (flip ? 1.0f : -1.0f) * s;

        float cx = chevLeft + chevSize * 0.5f;
        float cy = hdr.y + hdr.h * 0.5f + nudge;

        float hot = EaseOut(hotT_);
        if (hot > 0.001f) {
            float a = LerpF(subtleHov.a, subtlePrs.a, EaseOut(pressT_)) * hot;
            renderer.FillRoundedRect(RectF(cx - chevSize * 0.5f, cy - chevSize * 0.5f,
                                           chevSize, chevSize),
                                     kOuterRadius * s, subtleHov.WithAlpha(a));
        }

        // 官方 Fluent chevron（12px 字形），绕字形中心旋转
        const float gs = 12.0f * s;
        DrawFluentIcon(renderer, FluentIcon::ChevronDown,
                       cx - gs * 0.5f, cy - gs * 0.5f, gs,
                       textSecondary, ang);
    }

    // ---- 7. Content ----
    if (contentVisible && contentDraw_) {
        float pad = padding_ * s;
        float innerW = FzMx(0.0f, bounds_.w - 2.0f * pad);
        float innerH = FzMx(0.0f, contentNatH - 2.0f * pad);
        // Down：内容顶对齐（随展开从上往下揭示）；Up：内容底对齐（向上生长）
        float innerY = up ? (cont.Bottom() - pad - innerH) : (cont.y + pad);
        RectF inner(bounds_.x + pad, innerY, innerW, innerH);

        // HorizontalContentAlignment / VerticalContentAlignment
        float boxW = (contentNatW_ > 0.0f && alignH_ != HAlign::Stretch)
                     ? FzMn(contentNatW_ * s, inner.w) : inner.w;
        float boxH = (contentNatH_ > 0.0f && alignV_ != VAlign::Stretch)
                     ? FzMn(contentNatH_ * s, inner.h) : inner.h;
        float ox = 0.0f, oy = 0.0f;
        if (alignH_ == HAlign::Center)      ox = (inner.w - boxW) * 0.5f;
        else if (alignH_ == HAlign::Right)  ox = inner.w - boxW;
        if (alignV_ == VAlign::Center)      oy = (inner.h - boxH) * 0.5f;
        else if (alignV_ == VAlign::Bottom) oy = inner.h - boxH;

        RectF box(inner.x + ox, inner.y + oy, boxW, boxH);

        renderer.PushClip(RectF(cont.x, cont.y, cont.w, visibleH));
        contentDraw_(renderer, theme, s, box);
        renderer.PopClip();
    }
}

} // namespace ModernDesign