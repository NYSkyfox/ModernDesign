#pragma once
// ============================================================
// DemoWindow — 演示应用外壳（解耦架构）
//
// 职责：窗口生命周期 + 导航 + 跨页全局浮层（弹窗/浮出层）+ 页面宿主。
// 各页面（Home / Expander / Settings）是独立类，见 pages/。
// 页面通过 owner_(this) 访问本类的共享资源与几何。
// ============================================================
#include "pch.h"
#include "app/App.h"
#include "controls/NavigationView.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ContentDialog.h"
#include "controls/Flyout.h"
#include "controls/MenuFlyout.h"
#include "controls/ToolTip.h"
#include "utils/FluentIcons.h"
#include "DemoPage.h"
#include "pages/HomePage.h"
#include "pages/ExpanderPage.h"
#include "pages/SettingsPage.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace {
constexpr float kMargin = 28.0f;
}

class DemoWindow : public App {
public:
    // ---- 页面路由（当前页 index：0=Home 1=Expander 2=Settings）----
    Demo::DemoPage* Page(int i) { return &page_[i]; }
    int CurrentPage() const { return current_; }
    const std::vector<int>& PageMap() const { return pageMap_; }

    // ---- 内容区几何（供页面访问）----
    RectF Cont() const { return nav_.ContentRect(); }
    float ContPadL() const {
        bool minimal = nav_.GetDisplayMode() == NavigationView::DisplayMode::LeftMinimal;
        return (minimal ? 44.0f : kMargin) * DpiScale();
    }
    float ContX() const { return Cont().x + ContPadL(); }
    float ContW() const { return FzMx(0.0f, Cont().w - ContPadL() - kMargin * DpiScale()); }
    float HeadY() const { return Cont().y + 24.0f * DpiScale(); }
    float RuleY() const { return Cont().y + 66.0f * DpiScale(); }
    float PageTop() const { return Cont().y + 70.0f * DpiScale(); }

    // 主题切换转发（供页面调用；ToggleTheme 是 App 的 protected 成员）
    void ToggleThemePublic() { ToggleTheme(); }

    // 页面标题 + 分隔线
    void DrawPageHeader(const std::wstring& title, float s) {
        DrawText(title, ContX(), HeadY(), ContW(), 44.0f * s,
                 L"Segoe UI", 28.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, GetTheme().TextPrimary(),
                 DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        DrawLine(ContX(), RuleY(), ContX() + ContW(), RuleY(),
                 1.0f * s, GetTheme().CardBorder());
    }

    // 页面作为友元：可直接访问下方共享资源（flyBtn_/flyout_/tip_/nav_ 等）
    friend class ModernDesign::Demo::HomePage;
    friend class ModernDesign::Demo::ExpanderPage;
    friend class ModernDesign::Demo::SettingsPage;

protected:
    void OnLayout() override {
        BindNav();
        LayoutPages();
    }

    void OnRender() override {
        float s = DpiScale();
        const Theme& theme = GetTheme();
        LayoutPages();
        nav_.Draw(*this, theme, s);
        DrawPageHeader(page_[current_]->Title(), s);
        page_[current_]->Draw();
        // 模态弹窗：最顶层（页面之上）
        dialog_.Draw(*this, theme, s);
        // 浮出层：最顶层
        flyout_.Draw(*this, theme, s);
        menu_.Draw(*this, theme, s);
        tip_.Draw(*this, theme, s);
    }

    bool OnUpdate(float dt) override {
        bool anim = nav_.Update(dt);
        anim |= home_.Update(dt);
        anim |= expander_.Update(dt);
        anim |= settings_.Update(dt);
        anim |= dialog_.Update(dt);
        anim |= flyout_.Update(dt);
        anim |= menu_.Update(dt);
        anim |= tip_.Update(dt);
        return anim;
    }

    void OnMouseMove(float x, float y) override {
        if (dialog_.OnMouseMove(x, y)) return;
        if (flyout_.OnMouseMove(x, y)) return;
        if (menu_.OnMouseMove(x, y)) return;
        UpdateToolTipHover(x, y);
        nav_.OnMouseMove(x, y);
        page_[current_]->OnMouseMove(x, y);
    }
    void OnMouseLeave() override {
        if (dialog_.IsOpen()) { dialog_.OnMouseLeave(); return; }
        if (flyout_.IsOpen()) flyout_.OnMouseLeave();
        if (menu_.IsOpen()) menu_.OnMouseLeave();
        if (tipHover_) { tipHover_ = false; tip_.OnPointerLeave(); }
        tip_.OnPointerLeaveTip();
        nav_.OnMouseLeave();
        page_[current_]->OnMouseLeave();
    }
    void OnMouseDown(float x, float y) override {
        if (dialog_.OnMouseDown(x, y)) return;
        if (flyout_.OnMouseDown(x, y)) return;
        if (menu_.OnMouseDown(x, y)) return;
        nav_.OnMouseDown(x, y);
        page_[current_]->OnMouseDown(x, y);
    }
    void OnMouseUp(float x, float y) override {
        if (dialog_.OnMouseUp(x, y)) return;
        if (flyout_.OnMouseUp(x, y)) return;
        if (menu_.OnMouseUp(x, y)) return;
        nav_.OnMouseUp(x, y);
        page_[current_]->OnMouseUp(x, y);
    }

    void OnThemeChanged() override {
        home_.OnThemeChanged();
        expander_.OnThemeChanged();
        settings_.OnThemeChanged();
    }
    void OnKeyDown(int vk) override {
        if (dialog_.OnKeyDown(vk)) return;
        if (flyout_.OnKeyDown(vk)) return;
        if (menu_.OnKeyDown(vk)) return;
        if (vk == VK_SPACE) ToggleTheme();
    }

private:
    // ---- 导航绑定（一次）----
    void BindNav() {
        static bool bound = false;
        if (bound) return;
        bound = true;

        nav_.SetPaneTitle(L"Modern Design");
        nav_.AddHeader(L"Navigation");
        nav_.AddItem({ L"Home", FluentIcon::Home });
        nav_.AddItem({ L"Expander", FluentIcon::ChevronUpDown });
        nav_.AddHeader(L"Controls");
        nav_.AddGroup(L"Basics", FluentIcon::Grid,
                      { { L"CheckBox" }, { L"ToggleSwitch" },
                        { L"Slider" }, { L"RadioButton" } },
                      true);
        nav_.AddSeparator();
        nav_.SetSettings(L"Settings");

        pageMap_ = { -1, 0, 1, -1, -1, 0, 0, 0, 0, -1, 2 };
        nav_.SetSelectedIndex(2);            // 默认停在 Expander 页
        current_ = 1;
        nav_.SetSelectionCallback([this](int i) {
            int p = (i >= 0 && i < static_cast<int>(pageMap_.size())) ? pageMap_[i] : -1;
            if (p >= 0) { current_ = p; Invalidate(); }
        });

        // 各页面自绑定
        home_.Bind();
        expander_.Bind();
        settings_.Bind();

        // 全局浮层/弹窗绑定
        BindDialog();
        BindPopups();

        // 测试钩子：MODERNDESIGN_NAV_MODE = compact | minimal | top
        wchar_t mbuf[64] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_NAV_MODE", mbuf, 64) > 0) {
            std::wstring m = mbuf;
            if (m == L"compact")      nav_.SetDisplayMode(NavigationView::DisplayMode::LeftCompact);
            else if (m == L"minimal") nav_.SetDisplayMode(NavigationView::DisplayMode::LeftMinimal);
            else if (m == L"top")     nav_.SetDisplayMode(NavigationView::DisplayMode::Top);
        }
        // 测试钩子：MODERNDESIGN_PAGE = home | expander | settings
        {
            wchar_t pbuf[32] = {};
            if (GetEnvironmentVariableW(L"MODERNDESIGN_PAGE", pbuf, 32) > 0) {
                std::wstring pg = pbuf;
                int navIdx = -1; int page = -1;
                if (pg == L"home")      { navIdx = 1; page = 0; }
                else if (pg == L"expander")   { navIdx = 2; page = 1; }
                else if (pg == L"settings")   { navIdx = 9; page = 2; }
                if (page >= 0) {
                    nav_.SetSelectedIndex(navIdx);
                    current_ = page;
                    Invalidate();
                }
            }
        }
    }

    // ---- 每帧布局：导航 + 当前页 + 全局浮层 ----
    void LayoutPages() {
        float s = DpiScale();
        nav_.SetScale(s);
        nav_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));
        dialog_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));

        page_[current_]->Layout();

        // 浮出层：全客户区 + 锚定触发按钮（触发按钮在 Home 页）
        {
            const RectF full(0.0f, 0.0f, ClientWidth(), ClientHeight());
            flyout_.SetBounds(full);
            menu_.SetBounds(full);
            tip_.SetBounds(full);
            flyout_.SetAnchor(flyBtn_.GetBounds());
            menu_.SetAnchor(menuBtn_.GetBounds());
            tip_.SetAnchor(tipHost_.GetBounds());

            if (pendingPopup_ != 0) {
                const int k = pendingPopup_;
                pendingPopup_ = 0;
                if (k == 1) {
                    flyout_.Show();
                } else if (k == 2) {
                    menu_.Show();
                    menu_.ExpandSubitem(4);
                } else {
                    tip_.SetPointer(tipHost_.GetBounds().CenterX(),
                                    tipHost_.GetBounds().CenterY());
                    tip_.SetPlacement(ToolTipPlacement::Mouse);
                    tip_.Show(true);
                    tipDemo_ = true;
                }
            }
        }
    }

    // ToolTip 悬停跟踪
    void UpdateToolTipHover(float x, float y) {
        if (tipDemo_) return;
        const RectF host = tipHost_.GetBounds();
        const bool inHost = host.Contains(x, y);
        if (inHost && !tipHover_) {
            tipHover_ = true;
            tip_.OnPointerEnter(host, x, y);
        } else if (inHost) {
            tip_.OnPointerMove(x, y);
        } else if (tipHover_) {
            tipHover_ = false;
            tip_.OnPointerLeave();
        }
        const bool inPanel = tip_.IsOpen() && tip_.PanelRect().Contains(x, y);
        if (inPanel) tip_.OnPointerEnterTip();
        else         tip_.OnPointerLeaveTip();
    }

    // ---- ContentDialog（模态弹窗）----
    void BindDialog() {
        dialog_.SetTitle(L"Delete this file?");
        dialog_.SetContent(L"This action can't be undone. The file will be removed from this device.");
        dialog_.SetButtons(L"Delete", L"", L"Cancel");
        dialog_.SetDefaultButton(ContentDialog::DefaultButton::Primary);
        dialog_.SetContentDrawHeight(40.0f);
        dlgChk_.SetText(L"Don't ask me again");
        dlgChk_.SetChecked(false);

        dialog_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF& box) {
            dlgChk_.SetBounds(RectF(box.x, box.y + 4.0f * sc, box.w, 32.0f * sc));
            dlgChk_.Draw(r, t, sc);
        });
        dialog_.SetContentUpdateFn([this](float dt) { return dlgChk_.Update(dt); });
        dialog_.SetContentInputFn(
            [this](float px, float py) { dlgChk_.OnMouseMove(px, py); },
            [this](float px, float py) { dlgChk_.OnMouseDown(px, py); },
            [this](float px, float py) { dlgChk_.OnMouseUp(px, py); },
            [this](float, float)       { dlgChk_.OnMouseLeave(); });
        dialog_.SetResultCallback([this](ContentDialog::Result r) {
            dlgResult_ = (r == ContentDialog::Result::Primary) ? 1
                       : (r == ContentDialog::Result::Secondary) ? 2 : 0;
            Invalidate();
        });

        dlgBtn_.SetText(L"Show dialog");
        dlgBtn_.SetVariant(ButtonVariant::Standard);
        dlgBtn_.SetClickCallback([this] { dialog_.Show(); Invalidate(); });

        wchar_t dbuf[8] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_DIALOG", dbuf, 8) > 0 && dbuf[0] == L'1') {
            nav_.SetSelectedIndex(1);
            current_ = 0;
            dialog_.Show();
        }
    }

    // ---- Flyout / MenuFlyout / ToolTip ----
    void BindPopups() {
        flyout_.SetContent(L"This is a flyout. Click anywhere outside to dismiss it.");
        flyout_.SetPlacement(PopupPlacement::Bottom);
        flyBtn_.SetText(L"Show flyout");
        flyBtn_.SetVariant(ButtonVariant::Standard);
        flyBtn_.SetClickCallback([this] { flyout_.Show(); Invalidate(); });

        menu_.AddItemWithIcon(L"New", FluentIcon::Home, L"Ctrl+N");
        menu_.AddItemWithIcon(L"Open", FluentIcon::Grid, L"Ctrl+O");
        menu_.AddItem(L"Save", L"Ctrl+S");
        menu_.AddSeparator();
        {
            const int styleIdx = menu_.AddSubItemWithIcon(L"Style", FluentIcon::Info);
            menu_.AddChildIcon(styleIdx, L"Bold", FluentIcon::Info);
            menu_.AddChildIcon(styleIdx, L"Italic", FluentIcon::Info);
            menu_.AddChild(styleIdx, L"Underline");
            const int splitIdx = menu_.AddSplitItem(L"Save as");
            menu_.AddChild(splitIdx, L"Save copy");
            menu_.AddChild(splitIdx, L"Export");
        }
        menu_.AddSeparator();
        menu_.AddToggle(L"Word wrap", true);
        menu_.AddSeparator();
        menu_.AddRadio(L"Small", true);
        menu_.AddRadio(L"Medium", false);
        menu_.AddRadio(L"Large", false);
        menu_.SetPlacement(PopupPlacement::BottomEdgeAlignedLeft);
        menuBtn_.SetText(L"Show menu");
        menuBtn_.SetVariant(ButtonVariant::Standard);
        menuBtn_.SetClickCallback([this] { menu_.Show(); Invalidate(); });

        tip_.SetContent(L"This is a tooltip. It follows the pointer.");
        tip_.SetPlacement(ToolTipPlacement::Mouse);
        tipHost_.SetText(L"Hover for tooltip");
        tipHost_.SetVariant(ButtonVariant::Standard);

        wchar_t b1[8] = {}, b2[8] = {}, b3[8] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_FLYOUT", b1, 8) > 0 && b1[0] == L'1')
            pendingPopup_ = 1;
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_MENU", b2, 8) > 0 && b2[0] == L'1')
            pendingPopup_ = 2;
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_TOOLTIP", b3, 8) > 0 && b3[0] == L'1')
            pendingPopup_ = 3;
        if (pendingPopup_ != 0) {
            nav_.SetSelectedIndex(1);
            current_ = 0;
        }
    }

    // ---- 共享状态 ----
    int current_ = 0;
    NavigationView nav_;
    std::vector<int> pageMap_;

    // 跨页全局浮层 / 弹窗（最顶层）
    ContentDialog dialog_;
    Button dlgBtn_;
    CheckBox dlgChk_;
    int dlgResult_ = 0;
    Flyout flyout_;
    MenuFlyout menu_;
    ToolTip tip_;
    Button flyBtn_, menuBtn_, tipHost_;
    bool tipHover_ = false;
    bool tipDemo_ = false;
    int  pendingPopup_ = 0;

    // 页面（一个文件一个类，自治布局/绘制/输入/动画）
    Demo::HomePage     home_{this};
    Demo::ExpanderPage expander_{this};
    Demo::SettingsPage settings_{this};
    Demo::DemoPage* page_[3] = { &home_, &expander_, &settings_ };
};