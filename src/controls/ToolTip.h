#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// ToolTip — 工具提示（规格来源：WinUIonWeb ToolTip.vue）
//
// 外观：min-w 随内容、max-w 320、padding 6 9 8 9、圆角 4（ControlCornerRadius）
//       描边 1px flyout-border
//       阴影 0 8 16 rgba(0,0,0,.14) + 0 0 2 rgba(0,0,0,.18)
//       字号 12 / 行高 16、底色 = 亚克力 × 78%（Theme::TipBg）
// 定位：gap 20（比 Flyout 的 6 大）、与窗口边距 8
//       Placement 默认 Mouse：优先在指针上方，放不下则下方，再不行夹回窗口；
//       其余方向按 top/bottom/left/right 的首选顺序挑第一个能放下的，最后夹回。
// 动画：167ms 线性淡入淡出（无位移、无 clip 揭示）
// 时序：InitialShowDelay 800ms；距上次关闭 ≤200ms 则立即显示（BetweenShowDelay）
// 交互：指针进入目标后延迟显示；离开目标（且未悬停在提示本体上）即关闭
// ============================================================

enum class ToolTipPlacement { Top, Bottom, Left, Right, Mouse };

class ToolTip {
public:
    ToolTip() = default;

    void SetBounds(const RectF& windowBounds) { bounds_ = windowBounds; }
    void SetContent(const std::wstring& text) { content_ = text; }
    void SetAnchor(const RectF& target) { anchor_ = target; }
    void SetPlacement(ToolTipPlacement p) { placement_ = p; }
    void SetPointer(float x, float y) { pointer_ = RectF(x, y, 0.0f, 0.0f); }
    void SetEnabled(bool e) { enabled_ = e; }
    void SetInitialShowDelay(float ms) { initialDelay_ = ms / 1000.0f; }
    void SetBetweenShowDelay(float ms) { betweenDelay_ = ms / 1000.0f; }

    // ---- 指针交互 ----
    void OnPointerEnter(const RectF& target, float x, float y);
    void OnPointerMove(float x, float y);
    void OnPointerLeave();
    void OnPointerEnterTip();     // 指针移到提示本体上（保持显示）
    void OnPointerLeaveTip();

    // ---- 手动控制（调试 / 测试钩子用）----
    void Show(bool immediate = false);
    void Hide(bool force = false);
    bool IsOpen() const { return visible_; }
    bool IsAnimating() const { return anim_; }

    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    const RectF& PanelRect() const { return panel_; }
    float Openness() const { return animT_; }

private:
    void RequestShow(float delay);
    void BeginOpen();
    void BeginClose();
    void Layout(Renderer& renderer, float scale);
    void WrapBody(Renderer& renderer, float maxW, float scale);

    RectF bounds_;         // 整窗客户区
    RectF anchor_;         // 目标矩形（非鼠标模式定位用）
    RectF panel_;          // 最终面板矩形
    RectF pointer_;        // 指针位置（w/h = 0）

    ToolTipPlacement placement_ = ToolTipPlacement::Mouse;

    std::wstring content_;
    std::vector<std::wstring> lines_;

    bool  hoverTarget_ = false;
    bool  hoverTip_ = false;
    bool  enabled_ = true;

    bool  waiting_ = false;
    float waitLeft_ = 0.0f;      // 剩余等待（秒）
    float timeAccum_ = 0.0f;     // 累计时间（秒，用于 BetweenShowDelay）
    float lastCloseTime_ = -1.0f;

    bool  visible_ = false;
    bool  closing_ = false;
    bool  anim_ = false;
    float animT_ = 0.0f;         // 0=透明 1=不透明
    float animFrom_ = 0.0f;
    float elapsed_ = 0.0f;

    float initialDelay_ = 0.800f;
    float betweenDelay_ = 0.200f;

    // ---- 规格常量（DIP）----
    static constexpr float kMaxWidth   = 320.0f;
    static constexpr float kPadL       = 9.0f;
    static constexpr float kPadR       = 9.0f;
    static constexpr float kPadT       = 6.0f;
    static constexpr float kPadB       = 8.0f;
    static constexpr float kRadius     = 4.0f;    // ControlCornerRadius
    static constexpr float kGap        = 20.0f;
    static constexpr float kMargin     = 8.0f;
    static constexpr float kFontSize   = 12.0f;
    static constexpr float kLineHeight = 16.0f;
    static constexpr float kFadeDur    = 0.167f;
    static constexpr float kShadowY    = 8.0f;
    static constexpr float kShadowBlur = 16.0f;
    static constexpr float kShadowA    = 0.14f;
    static constexpr float kEdgeBlur   = 2.0f;
    static constexpr float kEdgeA      = 0.18f;
};

} // namespace ModernDesign
