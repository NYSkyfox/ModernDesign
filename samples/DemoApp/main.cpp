// ============================================================
// ModernDesign DemoApp — 演示应用（导航窗格 + 多页面）
//
//   左侧：NavigationView 侧边栏（hamburger 折叠 + 菜单项 + accent 指示条）
//   右侧：根据选中项切换页面
//       Home     — 基础控件合集
//       Expander — 可折叠容器（HeaderIcon / Description / HeaderControls / Up 方向）
//       Settings — 设置项示例
//
//   鼠标：悬停 / 点击 / 拖拽    键盘：Space 切换深浅主题
// ============================================================

#include "pch.h"
#include <shellapi.h>
#include "app/App.h"
#include "controls/NavigationView.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ToggleSwitch.h"
#include "controls/RadioButton.h"
#include "controls/ProgressBar.h"
#include "controls/Slider.h"
#include "controls/Expander.h"
#include "controls/ContentDialog.h"
#include "controls/Flyout.h"
#include "controls/MenuFlyout.h"
#include "controls/ToolTip.h"
#include "controls/HyperlinkButton.h"
#include "controls/SettingsCard.h"
#include "utils/FluentIcons.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace {
constexpr float kMargin = 28.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kRowGap = 6.0f;
constexpr float kGroupGap = 26.0f;
} // namespace

// ============================================================
// DemoWindow
// ============================================================
class DemoWindow : public App {
protected:
    // 内容区几何：完全跟随 NavigationView（Left 折叠时内容区会平滑跟随位移）
    RectF Cont() const { return nav_.ContentRect(); }
    // LeftMinimal：无占位、汉堡浮在左上角 → 页面按惯例让出 44px（NavigationViewMinimalHeaderMargin）
    float ContPadL() const {
        bool minimal = nav_.GetDisplayMode() == NavigationView::DisplayMode::LeftMinimal;
        return (minimal ? 44.0f : kMargin) * DpiScale();
    }
    float ContX() const { return Cont().x + ContPadL(); }
    float ContW() const { return FzMx(0.0f, Cont().w - ContPadL() - kMargin * DpiScale()); }
    // 页面标题 / 分隔线 / 内容起始 y
    float HeadY() const { return Cont().y + 24.0f * DpiScale(); }
    float RuleY() const { return Cont().y + 66.0f * DpiScale(); }
    float PageTop() const { return Cont().y + 70.0f * DpiScale(); }

    void OnLayout() override {
        BindNav();
        LayoutPages();
    }

    // 导航项只绑定一次（避免每次布局重复 AddItem）
    void BindNav() {
        static bool bound = false;
        if (bound) return;
        bound = true;

        nav_.SetPaneTitle(L"Modern Design");
        nav_.AddHeader(L"Navigation");
        nav_.AddItem({ L"Home", FluentIcon::Home });            // idx 1 → Home 页
        nav_.AddItem({ L"Expander", FluentIcon::ChevronUpDown }); // idx 2 → Expander 页
        nav_.AddHeader(L"Controls");
        // 可折叠分组（父项只负责展开/收起，子项共享 Home 页）
        nav_.AddGroup(L"Basics", FluentIcon::Grid,
                      { { L"CheckBox" }, { L"ToggleSwitch" },
                        { L"Slider" }, { L"RadioButton" } },
                      true);
        nav_.AddSeparator();
        nav_.SetSettings(L"Settings");       // 最后一项 → Settings 页

        pageMap_ = { -1, 0, 1, -1, -1, 0, 0, 0, 0, -1, 2 };
        nav_.SetSelectedIndex(2);            // 默认停在 Expander 页
        current_ = 1;
        nav_.SetSelectionCallback([this](int i) {
            int p = (i >= 0 && i < static_cast<int>(pageMap_.size())) ? pageMap_[i] : -1;
            if (p >= 0) { current_ = p; Invalidate(); }
        });

        BindExpanderPage();
        BindDialog();
        BindPopups();

        // 测试钩子（CI 逐模式出图）：MODERNDESIGN_NAV_MODE = compact | minimal | top
        wchar_t mbuf[64] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_NAV_MODE", mbuf, 64) > 0) {
            std::wstring m = mbuf;
            if (m == L"compact")      nav_.SetDisplayMode(NavigationView::DisplayMode::LeftCompact);
            else if (m == L"minimal") nav_.SetDisplayMode(NavigationView::DisplayMode::LeftMinimal);
            else if (m == L"top")     nav_.SetDisplayMode(NavigationView::DisplayMode::Top);
        }

        // 测试钩子（CI 出图）：MODERNDESIGN_PAGE = home | expander | settings
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

    // 每帧调用：面板折叠/展开时内容区宽度平滑变化
    void LayoutPages() {
        float s = DpiScale();
        nav_.SetScale(s);
        nav_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));
        // ContentDialog 覆盖整个客户区（模态遮罩铺满）
        dialog_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));

        // ---- Home 页布局 ----
        {
            float x = ContX(), y = PageTop(); // 标题区下方
            homeBtnStd_.SetText(L"Standard");
            homeBtnStd_.SetVariant(ButtonVariant::Standard);
            homeBtnStd_.SetBounds(RectF(x, y, 96.0f * s, kRowHeight * s));
            homeBtnAcc_.SetText(L"Accent");
            homeBtnAcc_.SetVariant(ButtonVariant::Accent);
            homeBtnAcc_.SetBounds(RectF(x + 108.0f * s, y, 96.0f * s, kRowHeight * s));
            // ContentDialog 触发按钮
            dlgBtn_.SetBounds(RectF(x + 216.0f * s, y, 120.0f * s, kRowHeight * s));
            y += kRowHeight * s + kGroupGap * s;

            homeChk_.SetText(L"CheckBox checked");
            homeChk_.SetChecked(true);
            homeChk_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kRowGap * s;

            homeTog_.SetText(L"Toggle on");
            homeTog_.SetIsOn(true);
            homeTog_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kGroupGap * s;

            homeRadioA_.SetText(L"RadioButton unchecked");
            homeRadioA_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kRowGap * s;
            homeRadioB_.SetText(L"RadioButton checked");
            homeRadioB_.SetSelected(true);
            homeRadioB_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kGroupGap * s;

            homeSlider_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kGroupGap * s;

            homeProg_.SetProgress(0.6f);
            homeProg_.SetBounds(RectF(x, y, ContW(), 12.0f * s));
            y += 12.0f * s + kGroupGap * s;

            // HyperlinkButton：链接按钮（accent 文字，宽度贴合内容）
            homeLinkA_.SetText(L"Learn more about Modern Design");
            homeLinkA_.SetNavigateUri(L"https://github.com/NYSkyfox/ModernDesign");
            homeLinkA_.SetClickCallback([] {
                ShellExecuteW(nullptr, L"open", L"https://github.com/NYSkyfox/ModernDesign",
                              nullptr, nullptr, SW_SHOWNORMAL);
            });
            homeLinkB_.SetText(L"Unavailable link");
            homeLinkB_.SetEnabled(false);
            homeLinkA_.SetBounds(RectF(x, y, homeLinkA_.MeasureWidth(*this, s), kRowHeight * s));
            homeLinkB_.SetBounds(RectF(x + 28.0f * s, y, homeLinkB_.MeasureWidth(*this, s), kRowHeight * s));
            y += kRowHeight * s + kGroupGap * s;

            // 浮出层触发按钮（Flyout / MenuFlyout / ToolTip）
            flyBtn_.SetBounds(RectF(x, y, 118.0f * s, kRowHeight * s));
            menuBtn_.SetBounds(RectF(x + 130.0f * s, y, 118.0f * s, kRowHeight * s));
            tipHost_.SetBounds(RectF(x + 260.0f * s, y, 150.0f * s, kRowHeight * s));
        }

        // ---- Expander 页布局 ----
        {
            float x = ContX(), y = PageTop();
            float w = ContW();              // WinUI 默认 HorizontalAlignment=Stretch → 占满整行
            float pad = 16.0f * s;
            float rowW = w - 2.0f * pad;    // 内容子控件同样占满内容区（开关贴右边缘）
            float mb = Expander::kMarginBottom * s;

            // A：HeaderIcon + Description + Content
            {
                float h = (expanderA_.HeaderHeight() + expanderA_.ContentHeight()) * s;
                expanderA_.SetScale(s);
                expanderA_.SetBounds(RectF(x, y, w, h));
                expAToggle_.SetBounds(RectF(x + pad, y + expanderA_.HeaderHeight() * s + pad,
                                            rowW, kRowHeight * s));
                y += h + mb;
            }
            // B：HeaderControls（Header 右侧放一个 Button）
            {
                float h = (expanderB_.HeaderHeight() + expanderB_.ContentHeight()) * s;
                expanderB_.SetScale(s);
                expanderB_.SetBounds(RectF(x, y, w, h));
                RectF cb = expanderB_.HeaderControlsBox(s);
                expHeaderBtn_.SetBounds(RectF(cb.x, cb.y + (cb.h - 32.0f * s) * 0.5f,
                                              cb.w, 32.0f * s));
                expBToggle_.SetBounds(RectF(x + pad, y + expanderB_.HeaderHeight() * s + pad,
                                            rowW, kRowHeight * s));
                y += h + mb;
            }
            // C：ExpandDirection = Up（内容在 Header 上方）
            {
                float h = (expanderC_.HeaderHeight() + expanderC_.ContentHeight()) * s;
                expanderC_.SetScale(s);
                expanderC_.SetBounds(RectF(x, y, w, h));
                expCToggle_.SetBounds(RectF(x + pad, y + pad, rowW, kRowHeight * s));
                y += h + mb;
            }
        }

        // ---- Settings 页布局（SettingsCard 容器）----
        {
            float x = ContX(), y = PageTop();
            const float cardH = 70.0f;
            const float cardGap = 4.0f;   // margin-bottom
            const float togW = 52.0f;     // track 40 + pad 12（开关文字区为 0，不重复标签）

            // Card A — Appearance（开关：深色模式）
            setCardA_.SetHeader(L"Appearance");
            setCardA_.SetDescription(L"Choose the app color mode");
            setCardA_.SetHeaderIcon(static_cast<int>(FluentIcon::Home));
            setCardA_.SetContentWidth(togW);
            setCardA_.SetBounds(RectF(x, y, ContW(), cardH * s));
            setTogA_.SetText(L"");
            setTogA_.SetIsOn(!GetTheme().lightMode);
            setTogA_.SetBounds(setCardA_.ContentRect(s));
            y += cardH * s + cardGap * s;

            // Card B — Navigation（开关：紧凑窗格）
            setCardB_.SetHeader(L"Navigation");
            setCardB_.SetDescription(L"Compact pane (48px rail)");
            setCardB_.SetHeaderIcon(static_cast<int>(FluentIcon::Navigation));
            setCardB_.SetContentWidth(togW);
            setCardB_.SetBounds(RectF(x, y, ContW(), cardH * s));
            setTogB_.SetText(L"");
            setTogB_.SetIsOn(nav_.IsCompact());
            setTogB_.SetBounds(setCardB_.ContentRect(s));
            y += cardH * s + cardGap * s;

            // Card C — Layout（开关：顶部导航）
            setCardC_.SetHeader(L"Layout");
            setCardC_.SetDescription(L"Top navigation bar");
            setCardC_.SetHeaderIcon(static_cast<int>(FluentIcon::Grid));
            setCardC_.SetContentWidth(togW);
            setCardC_.SetBounds(RectF(x, y, ContW(), cardH * s));
            setTogC_.SetText(L"");
            setTogC_.SetIsOn(nav_.GetDisplayMode() == NavigationView::DisplayMode::Top);
            setTogC_.SetBounds(setCardC_.ContentRect(s));
            y += cardH * s + cardGap * s;

            // Card D — About（纯文本 content + ActionIcon chevron；clickable 演示 hover 态）
            setCardD_.SetHeader(L"About");
            setCardD_.SetDescription(L"ModernDesign — Fluent framework");
            setCardD_.SetHeaderIcon(static_cast<int>(FluentIcon::Info));
            setCardD_.SetClickable(true);
            setCardD_.SetActionIconVisible(true);
            setCardD_.SetContent(L"v1.0");
            setCardD_.SetContentWidth(36.0f);
            setCardD_.SetBounds(RectF(x, y, ContW(), cardH * s));

            if (!setBound_) {
                setBound_ = true;
                setTogA_.SetChangedCallback([this](bool on) { if (on != !GetTheme().lightMode) ToggleTheme(); });
                setTogB_.SetChangedCallback([this](bool on) { nav_.SetCompact(on); Invalidate(); });
                setTogC_.SetChangedCallback([this](bool on) {
                    nav_.SetDisplayMode(on ? NavigationView::DisplayMode::Top
                                           : NavigationView::DisplayMode::Left);
                    Invalidate();
                });
            }
        }

        // ---- 浮出层布局（Flyout / MenuFlyout / ToolTip）----
        {
            const RectF full(0.0f, 0.0f, ClientWidth(), ClientHeight());
            flyout_.SetBounds(full);
            menu_.SetBounds(full);
            tip_.SetBounds(full);
            flyout_.SetAnchor(flyBtn_.GetBounds());
            menu_.SetAnchor(menuBtn_.GetBounds());
            tip_.SetAnchor(tipHost_.GetBounds());

            // 测试钩子：布局完成后（锚点已知）再弹出，保证截图稳定
            if (pendingPopup_ != 0) {
                const int k = pendingPopup_;
                pendingPopup_ = 0;
                if (k == 1) {
                    flyout_.Show();
                } else if (k == 2) {
                    menu_.Show();
                    menu_.ExpandSubitem(4);   // 展开 "Style" 子项（CI 出二级菜单）
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

    // Expander 页面：三个示例，覆盖全部能力
    void BindExpanderPage() {
        // ---------- A：HeaderIcon + Description + Content ----------
        expanderA_.SetHeader(L"Feature");
        expanderA_.SetDescription(L"Collapsible container demo");
        expanderA_.SetIsExpanded(true);
        expanderA_.SetContentHeight(64.0f);   // DIP（含 padding）
        expanderA_.SetHeaderIcon([](Renderer& r, const Theme& th, float sc, const RectF& box) {
            // 官方 Fluent info 图标（20×20 图标盒）
            DrawFluentIcon(r, FluentIcon::Info, box.x, box.y, box.w, th.TextSecondary());
        });
        expanderA_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
            expAToggle_.Draw(r, t, sc);
        });
        expanderA_.SetContentUpdateFn([this](float dt) { return expAToggle_.Update(dt); });
        expanderA_.SetContentInputFn(
            [this](float px, float py) { expAToggle_.OnMouseDown(px, py); },
            [this](float px, float py) { expAToggle_.OnMouseUp(px, py); },
            [this](float px, float py) { expAToggle_.OnMouseMove(px, py); },
            [this](float, float)       { expAToggle_.OnMouseLeave(); });
        expAToggle_.SetText(L"Enable feature");
        expAToggle_.SetIsOn(true);

        // ---------- B：HeaderControls ----------
        expanderB_.SetHeader(L"Notifications");
        expanderB_.SetIsExpanded(true);
        expanderB_.SetContentHeight(64.0f);
        expanderB_.SetHeaderControls(88.0f, [this](Renderer& r, const Theme& t, float sc, const RectF&) {
            expHeaderBtn_.Draw(r, t, sc);
        });
        expanderB_.SetHeaderControlsInputFn(
            [this](float px, float py) { expHeaderBtn_.OnMouseDown(px, py); },
            [this](float px, float py) { expHeaderBtn_.OnMouseUp(px, py); },
            [this](float px, float py) { expHeaderBtn_.OnMouseMove(px, py); },
            [this](float, float)       { expHeaderBtn_.OnMouseLeave(); });
        expHeaderBtn_.SetText(L"Reset");
        expHeaderBtn_.SetVariant(ButtonVariant::Standard);
        expanderB_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
            expBToggle_.Draw(r, t, sc);
        });
        expanderB_.SetContentUpdateFn([this](float dt) { return expBToggle_.Update(dt); });
        expanderB_.SetContentInputFn(
            [this](float px, float py) { expBToggle_.OnMouseDown(px, py); },
            [this](float px, float py) { expBToggle_.OnMouseUp(px, py); },
            [this](float px, float py) { expBToggle_.OnMouseMove(px, py); },
            [this](float, float)       { expBToggle_.OnMouseLeave(); });
        expBToggle_.SetText(L"Enable notifications");
        expBToggle_.SetIsOn(true);

        // ---------- C：ExpandDirection = Up ----------
        expanderC_.SetDirection(Expander::Direction::Up);
        expanderC_.SetHeader(L"Advanced");
        expanderC_.SetDescription(L"ExpandDirection = Up");
        expanderC_.SetIsExpanded(true);
        expanderC_.SetContentHeight(64.0f);
        expanderC_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
            expCToggle_.Draw(r, t, sc);
        });
        expanderC_.SetContentUpdateFn([this](float dt) { return expCToggle_.Update(dt); });
        expanderC_.SetContentInputFn(
            [this](float px, float py) { expCToggle_.OnMouseDown(px, py); },
            [this](float px, float py) { expCToggle_.OnMouseUp(px, py); },
            [this](float px, float py) { expCToggle_.OnMouseMove(px, py); },
            [this](float, float)       { expCToggle_.OnMouseLeave(); });
        expCToggle_.SetText(L"Advanced mode");
        expCToggle_.SetIsOn(false);
    }

    // ---------- ContentDialog（模态弹窗）----------
    void BindDialog() {
        dialog_.SetTitle(L"Delete this file?");
        dialog_.SetContent(L"This action can't be undone. The file will be removed from this device.");
        dialog_.SetButtons(L"Delete", L"", L"Cancel");
        dialog_.SetDefaultButton(ContentDialog::DefaultButton::Primary);
        dialog_.SetContentDrawHeight(40.0f);   // 内容里那个 CheckBox 的高度
        dlgChk_.SetText(L"Don't ask me again");
        dlgChk_.SetChecked(false);

        dialog_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF& box) {
            dlgChk_.SetBounds(RectF(box.x, box.y + 4.0f * sc, box.w, kRowHeight * sc));
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

        // 测试钩子（CI 出图）：MODERNDESIGN_SHOW_DIALOG=1 → 启动即弹出对话框
        wchar_t dbuf[8] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_DIALOG", dbuf, 8) > 0 && dbuf[0] == L'1') {
            nav_.SetSelectedIndex(1);   // 切到 Home 页（触发按钮在那儿）
            current_ = 0;
            dialog_.Show();
        }
    }

    // ---------- Flyout / MenuFlyout / ToolTip ----------
    void BindPopups() {
        // Flyout：纯文本内容
        flyout_.SetContent(L"This is a flyout. Click anywhere outside to dismiss it.");
        flyout_.SetPlacement(PopupPlacement::Bottom);
        flyBtn_.SetText(L"Show flyout");
        flyBtn_.SetVariant(ButtonVariant::Standard);
        flyBtn_.SetClickCallback([this] {
            flyout_.Show();
            Invalidate();
        });

        // MenuFlyout：普通项 / 图标项 / 快捷键 / 分隔线 / 复选 / 单选 / 子项 / 分裂项
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
        menuBtn_.SetClickCallback([this] {
            menu_.Show();
            Invalidate();
        });

        // ToolTip：默认 Mouse 定位，800ms 延迟
        tip_.SetContent(L"This is a tooltip. It follows the pointer.");
        tip_.SetPlacement(ToolTipPlacement::Mouse);
        tipHost_.SetText(L"Hover for tooltip");
        tipHost_.SetVariant(ButtonVariant::Standard);

        // 测试钩子（CI 出图）：
        //   MODERNDESIGN_SHOW_FLYOUT=1 / _MENU=1 / _TOOLTIP=1
        wchar_t b1[8] = {}, b2[8] = {}, b3[8] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_FLYOUT", b1, 8) > 0 && b1[0] == L'1')
            pendingPopup_ = 1;
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_MENU", b2, 8) > 0 && b2[0] == L'1')
            pendingPopup_ = 2;
        if (GetEnvironmentVariableW(L"MODERNDESIGN_SHOW_TOOLTIP", b3, 8) > 0 && b3[0] == L'1')
            pendingPopup_ = 3;
        if (pendingPopup_ != 0) {
            nav_.SetSelectedIndex(1);   // Home 页（触发按钮在那儿）
            current_ = 0;
        }
    }

    void DrawPageHeader(const std::wstring& title, float s) {
        DrawText(title, ContX(), HeadY(), ContW(), 44.0f * s,
                 L"Segoe UI", 28.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, GetTheme().TextPrimary(),
                 DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        DrawLine(ContX(), RuleY(), ContX() + ContW(), RuleY(),
                 1.0f * s, GetTheme().CardBorder());
    }

    bool OnUpdate(float dt) override {
        bool anim = nav_.Update(dt);
        anim |= homeBtnStd_.Update(dt); anim |= homeBtnAcc_.Update(dt);
        anim |= homeChk_.Update(dt); anim |= homeTog_.Update(dt);
        anim |= homeRadioA_.Update(dt); anim |= homeRadioB_.Update(dt);
        anim |= homeSlider_.Update(dt);
        anim |= homeLinkA_.Update(dt); anim |= homeLinkB_.Update(dt);
        anim |= expanderA_.Update(dt); anim |= expanderB_.Update(dt); anim |= expanderC_.Update(dt);
        anim |= expAToggle_.Update(dt); anim |= expBToggle_.Update(dt); anim |= expCToggle_.Update(dt);
        anim |= expHeaderBtn_.Update(dt);
        anim |= setTogA_.Update(dt); anim |= setTogB_.Update(dt); anim |= setTogC_.Update(dt);
        anim |= setCardA_.Update(dt); anim |= setCardB_.Update(dt); anim |= setCardC_.Update(dt); anim |= setCardD_.Update(dt);
        anim |= dlgBtn_.Update(dt);
        anim |= dialog_.Update(dt);   // 弹窗动画 + 内容（CheckBox）
        anim |= flyout_.Update(dt);   // 浮出层（含 83ms 淡入 / 250ms 展开）
        anim |= menu_.Update(dt);
        anim |= tip_.Update(dt);      // ToolTip（含 show delay 倒计时）
        return anim;
    }

    void OnRender() override {
        float s = DpiScale();
        const Theme& theme = GetTheme();

        LayoutPages();   // 面板折叠动画期间内容区需要逐帧重排

        nav_.Draw(*this, theme, s); // 画 panel + content 背景
        if (current_ <= 0) {
            DrawPageHeader(L"Home", s);
            homeBtnStd_.Draw(*this, theme, s); homeBtnAcc_.Draw(*this, theme, s);
            dlgBtn_.Draw(*this, theme, s);
            homeChk_.Draw(*this, theme, s); homeTog_.Draw(*this, theme, s);
            homeRadioA_.Draw(*this, theme, s); homeRadioB_.Draw(*this, theme, s);
            homeSlider_.Draw(*this, theme, s); homeProg_.Draw(*this, theme, s);
            homeLinkA_.Draw(*this, theme, s); homeLinkB_.Draw(*this, theme, s);
            flyBtn_.Draw(*this, theme, s); menuBtn_.Draw(*this, theme, s);
            tipHost_.Draw(*this, theme, s);
        } else if (current_ == 1) {
            DrawPageHeader(L"Expander", s);
            expanderA_.Draw(*this, theme, s);
            expanderB_.Draw(*this, theme, s);
            expanderC_.Draw(*this, theme, s);
        } else {
            DrawPageHeader(L"Settings", s);
            setCardA_.Draw(*this, theme, s); setCardB_.Draw(*this, theme, s);
            setCardC_.Draw(*this, theme, s); setCardD_.Draw(*this, theme, s);
            setTogA_.Draw(*this, theme, s); setTogB_.Draw(*this, theme, s);
            setTogC_.Draw(*this, theme, s);
        }

        // ContentDialog：模态浮层，必须最后画（盖住导航栏与页面）
        dialog_.Draw(*this, theme, s);

        // 浮出层（Flyout / MenuFlyout / ToolTip）：最顶层
        flyout_.Draw(*this, theme, s);
        menu_.Draw(*this, theme, s);
        tip_.Draw(*this, theme, s);
    }

    void OnMouseMove(float x, float y) override {
        if (dialog_.OnMouseMove(x, y)) return;   // 模态：弹窗吃掉一切
        if (flyout_.OnMouseMove(x, y)) return;   // 浮出层在页面之上
        if (menu_.OnMouseMove(x, y)) return;
        UpdateToolTipHover(x, y);

        nav_.OnMouseMove(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseMove(x, y); homeBtnAcc_.OnMouseMove(x, y);
            homeChk_.OnMouseMove(x, y); homeTog_.OnMouseMove(x, y);
            homeRadioA_.OnMouseMove(x, y); homeRadioB_.OnMouseMove(x, y);
            homeSlider_.OnMouseMove(x, y);
            homeLinkA_.OnMouseMove(x, y); homeLinkB_.OnMouseMove(x, y);
            flyBtn_.OnMouseMove(x, y); menuBtn_.OnMouseMove(x, y);
            tipHost_.OnMouseMove(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseMove(x, y); expanderB_.OnMouseMove(x, y); expanderC_.OnMouseMove(x, y);
        } else {
            setTogA_.OnMouseMove(x, y); setTogB_.OnMouseMove(x, y); setTogC_.OnMouseMove(x, y);
            setCardA_.OnMouseMove(x, y); setCardB_.OnMouseMove(x, y); setCardC_.OnMouseMove(x, y); setCardD_.OnMouseMove(x, y);
        }
    }
    void OnMouseLeave() override {
        if (dialog_.IsOpen()) { dialog_.OnMouseLeave(); return; }
        if (flyout_.IsOpen()) flyout_.OnMouseLeave();
        if (menu_.IsOpen()) menu_.OnMouseLeave();
        if (tipHover_) { tipHover_ = false; tip_.OnPointerLeave(); }
        tip_.OnPointerLeaveTip();

        nav_.OnMouseLeave();
        if (current_ <= 0) {
            homeBtnStd_.OnMouseLeave(); homeBtnAcc_.OnMouseLeave();
            homeChk_.OnMouseLeave(); homeTog_.OnMouseLeave();
            homeRadioA_.OnMouseLeave(); homeRadioB_.OnMouseLeave();
            homeSlider_.OnMouseLeave();
            homeLinkA_.OnMouseLeave(); homeLinkB_.OnMouseLeave();
            flyBtn_.OnMouseLeave(); menuBtn_.OnMouseLeave(); tipHost_.OnMouseLeave();
        } else if (current_ == 1) {
            expanderA_.OnMouseLeave(); expanderB_.OnMouseLeave(); expanderC_.OnMouseLeave();
        } else {
            setTogA_.OnMouseLeave(); setTogB_.OnMouseLeave(); setTogC_.OnMouseLeave();
            setCardA_.OnMouseLeave(); setCardB_.OnMouseLeave(); setCardC_.OnMouseLeave(); setCardD_.OnMouseLeave();
        }
    }
    void OnMouseDown(float x, float y) override {
        if (dialog_.OnMouseDown(x, y)) return;
        if (flyout_.OnMouseDown(x, y)) return;   // 点外部 = light dismiss
        if (menu_.OnMouseDown(x, y)) return;
        nav_.OnMouseDown(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseDown(x, y); homeBtnAcc_.OnMouseDown(x, y);
            homeChk_.OnMouseDown(x, y); homeTog_.OnMouseDown(x, y);
            homeRadioA_.OnMouseDown(x, y); homeRadioB_.OnMouseDown(x, y);
            homeSlider_.OnMouseDown(x, y);
            homeLinkA_.OnMouseDown(x, y); homeLinkB_.OnMouseDown(x, y);
            flyBtn_.OnMouseDown(x, y); menuBtn_.OnMouseDown(x, y); tipHost_.OnMouseDown(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseDown(x, y); expanderB_.OnMouseDown(x, y); expanderC_.OnMouseDown(x, y);
        } else {
            setTogA_.OnMouseDown(x, y); setTogB_.OnMouseDown(x, y); setTogC_.OnMouseDown(x, y);
            setCardA_.OnMouseDown(x, y); setCardB_.OnMouseDown(x, y); setCardC_.OnMouseDown(x, y); setCardD_.OnMouseDown(x, y);
        }
    }
    void OnMouseUp(float x, float y) override {
        if (dialog_.OnMouseUp(x, y)) return;
        if (flyout_.OnMouseUp(x, y)) return;
        if (menu_.OnMouseUp(x, y)) return;
        nav_.OnMouseUp(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseUp(x, y); homeBtnAcc_.OnMouseUp(x, y);
            homeChk_.OnMouseUp(x, y); homeTog_.OnMouseUp(x, y);
            homeRadioA_.OnMouseUp(x, y); homeRadioB_.OnMouseUp(x, y);
            homeSlider_.OnMouseUp(x, y);
            homeLinkA_.OnMouseUp(x, y); homeLinkB_.OnMouseUp(x, y);
            flyBtn_.OnMouseUp(x, y); menuBtn_.OnMouseUp(x, y); tipHost_.OnMouseUp(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseUp(x, y); expanderB_.OnMouseUp(x, y); expanderC_.OnMouseUp(x, y);
        } else {
            setTogA_.OnMouseUp(x, y); setTogB_.OnMouseUp(x, y); setTogC_.OnMouseUp(x, y);
            setCardA_.OnMouseUp(x, y); setCardB_.OnMouseUp(x, y); setCardC_.OnMouseUp(x, y); setCardD_.OnMouseUp(x, y);
        }
    }

    void OnThemeChanged() override {
        setTogA_.SetIsOn(!GetTheme().lightMode);
    }
    void OnKeyDown(int vk) override {
        if (dialog_.OnKeyDown(vk)) return;
        if (flyout_.OnKeyDown(vk)) return;   // Esc 关闭浮出层
        if (menu_.OnKeyDown(vk)) return;
        if (vk == VK_SPACE) ToggleTheme();
    }

private:
    // ToolTip 悬停跟踪：进入目标后延迟显示；在提示本体上时保持显示
    void UpdateToolTipHover(float x, float y) {
        if (tipDemo_) return;   // CI 固定展示：忽略指针进出，保证截图稳定
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

    int current_ = 0;
    bool setBound_ = false;
    NavigationView nav_;
    std::vector<int> pageMap_;   // 导航项索引 → 页面

    // Home
    Button homeBtnStd_, homeBtnAcc_;
    CheckBox homeChk_;
    ToggleSwitch homeTog_;
    RadioButton homeRadioA_, homeRadioB_;
    Slider homeSlider_;
    ProgressBar homeProg_;
    HyperlinkButton homeLinkA_, homeLinkB_;

    // Expander（三种示例：图标+描述 / HeaderControls / Up 方向）
    Expander expanderA_, expanderB_, expanderC_;
    ToggleSwitch expAToggle_, expBToggle_, expCToggle_;
    Button expHeaderBtn_;

    // Settings
    SettingsCard setCardA_, setCardB_, setCardC_, setCardD_;
    ToggleSwitch setTogA_, setTogB_, setTogC_;

    // ContentDialog（模态弹窗）+ 触发按钮 + 弹窗内容里的 CheckBox
    ContentDialog dialog_;
    Button dlgBtn_;
    CheckBox dlgChk_;
    int dlgResult_ = 0;   // 0=None 1=Primary 2=Secondary

    // 浮出层：Flyout / MenuFlyout / ToolTip + 触发控件
    Flyout flyout_;
    MenuFlyout menu_;
    ToolTip tip_;
    Button flyBtn_, menuBtn_, tipHost_;
    bool tipHover_ = false;
    bool tipDemo_ = false;    // CI 固定展示 ToolTip（忽略指针进出）
    int  pendingPopup_ = 0;   // 1=Flyout 2=MenuFlyout 3=ToolTip（CI 测试钩子）
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    DemoWindow app;
    HRESULT hr = app.Initialize(hInstance, nCmdShow);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"ModernDesign init failed (requires Win10 1809+)",
                    L"ModernDesign", MB_ICONERROR);
        return 1;
    }
    return app.Run();
}