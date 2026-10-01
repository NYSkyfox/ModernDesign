#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/Button.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// ContentDialog — 内容对话框（规格来源：WinUIonWeb ContentDialog.vue）
//
// 结构：
//   遮罩 SmokeLayer   整窗，rgba(0,0,0,0.302)，83ms 线性淡入淡出
//   卡片（居中）
//     ├ 内容区   padding 24，bg #FFFFFF / rgba(43,43,43,0)，底边 1px 分隔
//     │   ├ Title  20/28 w600，下间距 12
//     │   └ Body   14/20
//     └ CommandSpace  padding 24，bg #F3F3F3 / #202020
//         └ 按钮等宽（1/2 或 1/3 列），间距 8，高 32
//   卡片：宽 clamp(320, 548)，高 clamp(184, min(756, 可用高))，
//         圆角 8，描边 1px rgba(117,117,117,.4)，
//         阴影 0 32px 64px rgba(0,0,0,0.28)
//   动画：卡片 scale 1.05 → 1.0（250ms cubic-bezier(0,0,0,1)）
//                1.0 → 1.05（167ms 同曲线）
//
// 用法（模态）：
//   dialog.SetBounds(整窗客户区);
//   if (!dialog.OnMouseDown(x, y)) { ...其余控件... }      // 返回值=true 表示已被弹窗吃掉
//   dialog.Draw(renderer, theme, scale);                   // 必须最后画
//
// 注意：本控件不吃 D2D 之外的资源；内容可以是纯文本，也可以是任意控件
//       （通过 SetContentDrawFn/InputFn 注入）。
// ============================================================
class ContentDialog {
public:
    enum class Result { None, Primary, Secondary };
    enum class DefaultButton { None, Primary, Secondary, Close };

    using ContentDrawFn   = std::function<void(Renderer&, const Theme&, float, const RectF&)>;
    using ContentUpdateFn = std::function<bool(float)>;
    using ContentInputFn  = std::function<void(float, float)>;

    ContentDialog() = default;

    // ---- 几何 ----
    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    // ---- 内容 ----
    void SetTitle(const std::wstring& title) { title_ = title; }
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
    // 内容区固定高度（DIP，含 padding）；-1 = 按正文自动
    void SetContentHeight(float h) { contentHeight_ = h; }
    // 自定义内容所需高度（DIP，不含 padding）；会在父类正文下方再留出这么多
    void SetContentDrawHeight(float h) { contentDrawH_ = h; }

    // ---- 按钮 ----
    void SetButtons(const std::wstring& primary,
                    const std::wstring& secondary = std::wstring(),
                    const std::wstring& close = std::wstring());
    void SetButtonEnabled(bool primary, bool secondary) {
        primaryEnabled_ = primary;
        secondaryEnabled_ = secondary;
    }
    // DefaultButton 决定「回车」触发哪个按钮，同时该按钮用 Accent 变体
    // （规格：ContentDialog.vue 的 PrimaryButtonStyle = DefaultButton=='Primary'
    //    ? AccentButtonStyle : DefaultButtonStyle）
    void SetDefaultButton(DefaultButton b) { defaultButton_ = b; }
    void SetLightDismissEnabled(bool e) { lightDismiss_ = e; }
    void SetFullSizeDesired(bool e) { fullSize_ = e; }

    // ---- 开关 ----
    void Show();
    void Hide();
    bool IsOpen() const { return visible_; }
    bool IsAnimating() const { return anim_; }

    void SetResultCallback(std::function<void(Result)> cb) { onResult_ = std::move(cb); }
    void SetOpenedCallback(std::function<void()> cb) { onOpened_ = std::move(cb); }

    // ---- 帧 ----
    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

    // ---- 输入：返回 true = 已消费（调用方不要再传给下层控件）----
    bool OnMouseMove(float x, float y);
    bool OnMouseDown(float x, float y);
    bool OnMouseUp(float x, float y);
    void OnMouseLeave();
    bool OnKeyDown(int vk);

    // ---- 只读的布局结果（测试/调试用）----
    const RectF& CardRect() const { return card_; }
    float BodyHeight() const { return bodyH_; }

private:
    void  Layout(Renderer& renderer, float scale);
    void  Close(Result result);
    int   VisibleButtonCount() const;
    bool  HasPrimary() const { return !primaryText_.empty(); }
    bool  HasSecondary() const { return !secondaryText_.empty(); }
    bool  HasClose() const { return !closeText_.empty(); }
    // 换行：按空格贪心断行（返回总高）
    float WrapBody(Renderer& renderer, float maxW, float scale);

    // 文本
    std::wstring title_;
    std::wstring content_;
    std::wstring primaryText_, secondaryText_, closeText_;
    bool primaryEnabled_ = true;
    bool secondaryEnabled_ = true;
    DefaultButton defaultButton_ = DefaultButton::None;
    bool lightDismiss_ = false;
    bool fullSize_ = false;
    float contentHeight_ = -1.0f;   // DIP（含 padding）
    float contentDrawH_ = 0.0f;     // DIP（自定义内容高度，不含 padding）

    // 状态
    bool  visible_ = false;   // 逻辑打开（含关闭动画期间）
    bool  closing_ = false;
    bool  anim_ = false;

    RectF bounds_;            // 整窗客户区
    RectF card_;              // 当前布局的卡片
    RectF contentBox_;        // 卡片内「内容区」（含 padding）
    RectF cmdBox_;            // 卡片内「命令区」
    float bodyH_ = 0.0f;      // 正文换行后高度（DIP）
    std::vector<std::wstring> bodyLines_;

    // buttons 在 ModernDesign::Controls 命名空间下（历史遗留，见 controls/Button.h）
    Controls::Button primaryBtn_, secondaryBtn_, closeBtn_;
    RectF  primaryRect_, secondaryRect_, closeRect_;   // 三个按钮的最终位置（空=不可见）

    // 动画
    float cardT_ = 0.0f;      // 0=关闭 1=完全展开
    float overlayT_ = 0.0f;
    float elapsed_ = 0.0f;

    // 交互
    int pressBtn_ = -1;       // 0=primary 1=secondary 2=close

    ContentDrawFn   contentDraw_;
    ContentUpdateFn contentUpdate_;
    ContentInputFn  contentMove_, contentDown_, contentUp_, contentLeave_;
    std::function<void(Result)> onResult_;
    std::function<void()> onOpened_;

    // ---- 规格常量（DIP，来自 ContentDialog.vue）----
    static constexpr float kMinWidth    = 320.0f;
    static constexpr float kMaxWidth    = 548.0f;
    static constexpr float kMinHeight   = 184.0f;
    static constexpr float kMaxHeight   = 756.0f;
    static constexpr float kPad         = 24.0f;   // ContentDialogPadding
    static constexpr float kOuterPad    = 24.0f;   // 遮罩内边距
    static constexpr float kCorner      = 8.0f;    // OverlayCornerRadius
    static constexpr float kBorderW     = 1.0f;
    static constexpr float kShadowOffY  = 32.0f;
    static constexpr float kShadowBlur  = 64.0f;
    static constexpr float kShadowAlpha = 0.28f;
    static constexpr float kSmokeAlpha  = 0.302f;
    static constexpr float kTitleSize   = 20.0f;
    static constexpr float kTitleLine   = 28.0f;
    static constexpr float kTitleGap    = 12.0f;
    static constexpr float kBodySize    = 14.0f;
    static constexpr float kBodyLine    = 20.0f;
    static constexpr float kBtnH        = 32.0f;
    static constexpr float kBtnGap      = 8.0f;
    static constexpr float kOverlayFade = 0.083f;  // 83ms
    static constexpr float kOpenDur     = 0.250f;
    static constexpr float kCloseDur    = 0.167f;
    static constexpr float kScaleFrom   = 1.05f;
};

} // namespace ModernDesign