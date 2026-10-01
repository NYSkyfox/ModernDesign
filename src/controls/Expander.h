#pragma once

#include "Geometry.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include <functional>
#include <string>

namespace ModernDesign {

// ============================================================
// Expander — 可折叠容器（WinUIonWeb 规格）
// 结构：外框(1px 边框 圆角4) + header(48px 标题/描述/右侧控件/箭头) + content(16px 内边距 可折叠动画)
// 内容通过回调注册：caller 负责在内容区内绘制/更新/处理输入
// ============================================================
class Expander {
public:
    // 内容绘制回调：在 contentInner 矩形内绘制内容（caller 用 PushClip 前已设好）
    using ContentDrawFn = std::function<void(Renderer&, const Theme&, float scale, const RectF& inner)>;
    // 内容动画回调：返回 true 表示内容区仍在动画
    using ContentUpdateFn = std::function<bool(float dt)>;
    // 内容输入回调
    using ContentInputFn = std::function<void(float x, float y)>;

    Expander() = default;

    void SetHeader(const std::wstring& t) { header_ = t; }
    void SetDescription(const std::wstring& d) { description_ = d; }
    void SetIsExpanded(bool e) { isExpanded_ = e; }
    bool IsExpanded() const { return isExpanded_; }
    void SetExpandedCallback(std::function<void(bool)> cb) { onExpanded_ = cb; }

    void SetBounds(const RectF& b) { bounds_ = b; }
    const RectF& GetBounds() const { return bounds_; }

    // 内容区自然高度（DIP，未缩放；用于 caller 布局）
    void SetContentHeight(float h) { contentH_ = h; }
    float ContentHeight() const { return contentH_; }
    float HeaderHeight() const { return kHeaderH; }

    // 注册内容（Draw / Update / 输入）
    void SetContentDrawFn(ContentDrawFn fn) { contentDraw_ = std::move(fn); }
    void SetContentUpdateFn(ContentUpdateFn fn) { contentUpdate_ = std::move(fn); }
    void SetContentInputFn(ContentInputFn down, ContentInputFn up, ContentInputFn move, ContentInputFn leave) {
        contentDown_ = std::move(down);
        contentUp_ = std::move(up);
        contentMove_ = std::move(move);
        contentLeave_ = std::move(leave);
    }

    // 输入（header 点击折叠；content 区输入转发给注册回调）
    void OnMouseMove(float x, float y);
    void OnMouseLeave();
    void OnMouseDown(float x, float y);
    void OnMouseUp(float x, float y);

    bool Update(float dt);
    void Draw(Renderer& renderer, const Theme& theme, float scale);

private:
    RectF HeaderRect(float s) const;
    RectF ContentArea(float s) const;   // 含 padding 的内容区
    float FullHeight(float s) const;    // 完全展开总高

    std::wstring header_ = L"Expand";
    std::wstring description_;
    bool isExpanded_ = false;
    RectF bounds_;
    std::function<void(bool)> onExpanded_;

    float expandT_ = 0.0f;   // 0=收起 1=展开
    bool hot_ = false;
    bool pressed_ = false;
    float hotT_ = 0.0f;
    float pressT_ = 0.0f;
    float contentH_ = 0.0f;  // 内容自然高度（DIP）

    ContentDrawFn contentDraw_;
    ContentUpdateFn contentUpdate_;
    ContentInputFn contentDown_, contentUp_, contentMove_, contentLeave_;

    static constexpr float kHeaderH = 48.0f;
    static constexpr float kPad = 16.0f;
};

} // namespace ModernDesign
