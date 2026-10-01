#pragma once

#include "Geometry.h"
#include "controls/PopupPlacement.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// Flyout — 浮出控件（规格来源：WinUIonWeb Flyout.vue）
//
// 面板：min-w 96 / max-w min(456, 窗口宽-16)
//       min-h 40 / max-h min(758, 窗口高-16)
//       padding 15 16 17 16（DefaultFlyoutPresenterStyle.FlyoutContentPadding）
//       圆角 8、描边 1px flyout-border、阴影 0 8 24 rgba(0,0,0,.18)
// 定位：gap 6、与窗口边距 8；min-width 不小于锚点宽度；空间不足自动翻转
// 动画：打开 250ms cubic-bezier(.1,.9,.2,1)（从锚点侧 1px 展开 + 位移 ±16）
//       + 83ms 线性淡入；关闭 167ms cubic-bezier(.7,0,1,.5)（淡出 + 上移 4）
// 交互：IsLightDismissEnabled 默认 true（点面板外关闭）；Esc 关闭
// ============================================================
class Flyout {
public:
    using ContentDrawFn   = std::function<void(Renderer&, const Theme&, float, const RectF&)>;
    using ContentUpdateFn = std::function<bool(float)>;
    using ContentInputFn  = std::function<void(float, float)>;

    Flyout() = default;

    void SetBounds(const RectF& windowBounds) { bounds_ = windowBounds; }
    void SetAnchor(const RectF& anchor) { anchor_ = anchor; }
    const RectF& GetAnchor() const { return anchor_; }

    void SetPlacement(PopupPlacement p) { placement_ = p; }
    void SetContent(const std::wstring& text) { content_ = text; }
    void SetContentDrawFn(ContentDrawFn fn) { contentDraw_ = std::move(fn); }
    void SetContentUpdateFn(ContentUpdateFn fn) { contentUpdate_ = std::move(fn); }
    void SetContentInputFn(ContentInputFn move, ContentInputFn down,
                           ContentInputFn up, ContentInputFn leave) {
        contentMove_ = std::move(move);
        contentDown_ = std::move(down);
        contentUp_ = std::move(up);
        contentLeave_ = std::move(leave);
    }
    // 自定义内容高度（DIP，不含 padding）
    void SetContentDrawHeight(float h) { contentDrawH_ = h; }

    void SetLightDismissEnabled(bool e) { lightDismiss_ = e; }
    void Show();
    void Hide();
    bool IsOpen() const { return visible_; }
    bool IsAnimating() const { return anim_; }

    void SetClosedCallback(std::function<void()> cb) { onClosed_ = std::move(cb); }

    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    // 返回 true = 已消费（浮出层打开时，点外部=浅色消失）
    bool OnMouseMove(float x, float y);
    bool OnMouseDown(float x, float y);
    bool OnMouseUp(float x, float y);
    void OnMouseLeave();
    bool OnKeyDown(int vk);

    const RectF& PanelRect() const { return panel_; }

private:
    void Layout(Renderer& renderer, float scale);
    void Close();
    float WrapBody(Renderer& renderer, float maxW, float scale);

    RectF bounds_;       // 整窗客户区
    RectF anchor_;       // 锚点
    RectF panel_;        // 最终面板矩形
    PopupPlacement placement_ = PopupPlacement::Bottom;

    std::wstring content_;
    std::vector<std::wstring> lines_;
    float contentH_ = 0.0f;
    float contentDrawH_ = 0.0f;

    bool  visible_ = false;
    bool  closing_ = false;
    bool  anim_ = false;
    bool  lightDismiss_ = true;
    float animT_ = 0.0f;     // 0=收起 1=完全展开
    float elapsed_ = 0.0f;
    bool  above_ = false;    // 实际方向（决定展开/位移方向）

    ContentDrawFn   contentDraw_;
    ContentUpdateFn contentUpdate_;
    ContentInputFn  contentMove_, contentDown_, contentUp_, contentLeave_;
    std::function<void()> onClosed_;

    // ---- 规格常量（DIP）----
    static constexpr float kMinWidth   = 96.0f;
    static constexpr float kMaxWidth   = 456.0f;
    static constexpr float kMinHeight  = 40.0f;
    static constexpr float kMaxHeight  = 758.0f;
    static constexpr float kPadL       = 16.0f;
    static constexpr float kPadR       = 16.0f;
    static constexpr float kPadT       = 15.0f;
    static constexpr float kPadB       = 17.0f;
    static constexpr float kRadius     = 8.0f;    // OverlayCornerRadius
    static constexpr float kGap        = 6.0f;
    static constexpr float kMargin     = 8.0f;
    static constexpr float kShadowY    = 8.0f;
    static constexpr float kShadowBlur = 24.0f;
    static constexpr float kShadowA    = 0.18f;
    static constexpr float kOpenDur    = 0.250f;
    static constexpr float kCloseDur   = 0.167f;
    static constexpr float kFadeDur    = 0.083f;
    static constexpr float kSlide      = 16.0f;   // 打开时位移
    static constexpr float kCloseShift = 4.0f;    // 关闭时上移
    static constexpr float kShadowBleed= 32.0f;
    static constexpr float kFontSize   = 14.0f;
    static constexpr float kLineHeight = 20.0f;
};

} // namespace ModernDesign