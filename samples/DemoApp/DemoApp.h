#pragma once
// ============================================================
// DemoWindow — 演示应用外壳（解耦架构）
//
// 职责：窗口生命周期 + 导航 + 跨页全局浮层（弹窗/浮出层）+ 页面宿主。
// 各页面（见 pages/）是独立类，自治布局/绘制/输入/动画。
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
#include "pages/ButtonsPage.h"
#include "pages/SelectionPage.h"
#include "pages/SlidersPage.h"
#include "pages/ExpanderPage.h"
#include "pages/CardPage.h"
#include "pages/PopupsPage.h"
#include "pages/DialogPage.h"
#include "pages/SettingsPage.h"
#include "pages/ShowcasePage.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace {
constexpr float kMargin = 28.0f;
}

class DemoWindow : public App {
public:
    // ---- 页面路由 ----
    Demo::DemoPage* Page(int i) { return page_[i]; }
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

    // 页面标题
    void DrawPageHeader(const std::wstring& title, float s) {
        DrawText(title, ContX(), HeadY(), ContW(), 44.0f * s,
                 L"Segoe UI", 28.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, GetTheme().TextPrimary(),
                 DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }

    // 页面作为友元：可直接访问下方共享资源（flyBtn_/flyout_/tip_/nav_ 等）
    friend class ModernDesign::Demo::HomePage;
    friend class ModernDesign::Demo::ButtonsPage;
    friend class ModernDesign::Demo::SelectionPage;
    friend class ModernDesign::Demo::SlidersPage;
    friend class ModernDesign::Demo::ExpanderPage;
    friend class ModernDesign::Demo::CardPage;
    friend class ModernDesign::Demo::PopupsPage;
    friend class ModernDesign::Demo::DialogPage;
    friend class ModernDesign::Demo::SettingsPage;
    friend class ModernDesign::Demo::ShowcasePage;

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
        for (int i = 0; i < kPageCount; ++i) anim |= page_[i]->Update(dt);
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
        settings_.OnThemeChanged();
    }
    void OnKeyDown(int vk) override {
        if (dialog_.OnKeyDown(vk)) return;
        if (flyout_.OnKeyDown(vk)) return;
        if (menu_.OnKeyDown(vk)) return;
        if (vk == VK_SPACE) ToggleTheme();
    }

private:
    // ---- 页面数组（page_ index -> 页对象）----
    // 0=Home 1=Buttons 2=Selection 3=Sliders 4=Expander 5=Card 6=Popups 7=Dialog 8=Settings
    Demo::DemoPage** pages() { return page_; }
    static constexpr int kPageCount = 10;

    // ---- 导航绑定（一次）----
    // 导航项 -> 页 index（-1 = 不切页）
    //  0 Home | 1 Input* 2 Buttons 3 Selection 4 Sliders
    // | 5 Container* 6 Expander 7 Card | 8 Popups 9 Dialog | 10 Settings
    void BindNav() {
        static bool bound = false;
        if (bound) return;
        bound = true;

        nav_.SetPaneTitle(L"Modern Design");
        nav_.AddItem({ L"Home", FluentIcon::Home });                          // 0
        nav_.AddGroup(L"Input", FluentIcon::Checkmark,                          // 1 (组头)
                      { { L"Buttons" }, { L"Selection" }, { L"Sliders" } },     // 2 3 4
                      true);
        nav_.AddGroup(L"Container", FluentIcon::Grid,                            // 5 (组头)
              { { L"Expander" }, { L"Card" }, { L"Showcase" } },          // 6 7 8
              true);
nav_.AddItem({ L"Popups", FluentIcon::Info });                          // 9
nav_.AddItem({ L"Dialog", FluentIcon::ChevronUpDown });                 // 10
nav_.SetSettings(L"Settings");                                          // 11
pageMap_ = { 0, -1, 1, 2, 3, -1, 4, 5, 9, 6, 7, 8 };
        nav_.SetSelectedIndex(0);            // 默认停在 Home 页
        current_ = 0;
        prevSel_ = 0;
        SetBackEnabled(false);
        // 标题栏汉堡按钮（back 右侧）：切换导航面板展开/收起
        SetPaneToggleCallback([this] {
            nav_.TogglePane();
            Invalidate();
        });
        // 后退按钮（标题栏最左）：回退上一次选择的导航项
        SetBackRequestedCallback([this] {
            if (navBackStack_.empty()) return;
            int target = navBackStack_.back();
            navBackStack_.pop_back();
            navProg_ = true;
            nav_.SetSelectedIndex(target);   // 会触发 selection 回调同步 current_
            navProg_ = false;
            prevSel_ = target;
            SetBackEnabled(!navBackStack_.empty());
            Invalidate();
        });
        nav_.SetSelectionCallback([this](int i) {
            int p = (i >= 0 && i < static_cast<int>(pageMap_.size())) ? pageMap_[i] : -1;
            if (p >= 0) { current_ = p; Invalidate(); }
            // 前进导航项才入栈（后退由 back 回调处理，用 navProg_ 抑制重入）
            if (!navProg_ && i != prevSel_ && i >= 0) {
                navBackStack_.push_back(prevSel_);
                prevSel_ = i;
                SetBackEnabled(true);
            }
        });

        // 各页面自绑定
        home_.Bind();
        buttons_.Bind();
        selection_.Bind();
        sliders_.Bind();
        expander_.Bind();
        card_.Bind();
        showcase_.Bind();
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
        // 测试钩子：MODERNDESIGN_PAGE = home|buttons|selection|sliders|expander|card|popups|dialog|settings
        {
            wchar_t pbuf[32] = {};
            if (GetEnvironmentVariableW(L"MODERNDESIGN_PAGE", pbuf, 32) > 0) {
                std::wstring pg = pbuf;
                int navIdx = -1; int page = -1;
                if      (pg == L"home")      { navIdx = 0;  page = 0; }
                else if (pg == L"buttons")   { navIdx = 2;  page = 1; }
                else if (pg == L"selection") { navIdx = 3;  page = 2; }
                else if (pg == L"sliders")   { navIdx = 4;  page = 3; }
                else if (pg == L"expander")  { navIdx = 6;  page = 4; }
                else if (pg == L"card")      { navIdx = 7;  page = 5; }
                else if (pg == L"showcase")  { navIdx = 8;  page = 9; }
                else if (pg == L"popups")    { navIdx = 9;  page = 6; }
                else if (pg == L"dialog")    { navIdx = 10; page = 7; }
                else if (pg == L"settings")  { navIdx = 11; page = 8; }
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
        float tbH = TitleBarHeight();   // 沉浸式标题栏占顶部，内容从 tbH 起
        nav_.SetScale(s);
        nav_.SetBounds(RectF(0, tbH, ClientWidth(), FzMx(0.0f, ClientHeight() - tbH)));
        dialog_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));

        page_[current_]->Layout();

        // 浮出层：全客户区 + 锚定触发按钮（触发按钮在 Popups 页）
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
            nav_.SetSelectedIndex(10);
            current_ = 7;
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
            nav_.SetSelectedIndex(9);
            current_ = 6;
        }
    }

    // ---- 共享状态 ----
    int current_ = 0;
    NavigationView nav_;
    std::vector<int> pageMap_;
    std::vector<int> navBackStack_;   // 标题栏后退按钮：前进导航项栈
    int prevSel_ = -1;
    bool navProg_ = false;            // back 回调驱动 SetSelectedIndex 时的重入抑制

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
    Demo::ButtonsPage  buttons_{this};
    Demo::SelectionPage selection_{this};
    Demo::SlidersPage  sliders_{this};
    Demo::ExpanderPage expander_{this};
    Demo::CardPage     card_{this};
    Demo::ShowcasePage showcase_{this};
    Demo::PopupsPage   popups_{this};
    Demo::DialogPage   dialogPg_{this};
    Demo::SettingsPage settings_{this};
    Demo::DemoPage* page_[kPageCount] = {
        &home_, &buttons_, &selection_, &sliders_, &expander_,
        &card_, &popups_, &dialogPg_, &settings_, &showcase_ };
};