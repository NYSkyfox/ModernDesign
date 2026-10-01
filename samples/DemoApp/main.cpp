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
#include "app/App.h"
#include "controls/NavigationView.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ToggleSwitch.h"
#include "controls/RadioButton.h"
#include "controls/ProgressBar.h"
#include "controls/Slider.h"
#include "controls/Expander.h"

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
    float ContX() const { return Cont().x + kMargin * DpiScale(); }
    float ContW() const { return FzMx(0.0f, Cont().w - kMargin * DpiScale() * 2); }
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
        nav_.AddItem({ L"Home", 0 });        // idx 1 → Home 页
        nav_.AddItem({ L"Expander", 1 });    // idx 2 → Expander 页
        nav_.AddHeader(L"Controls");
        // 可折叠分组（父项只负责展开/收起，子项共享 Home 页）
        nav_.AddGroup(L"Basics", 1,
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

        // 测试钩子（CI 逐模式出图）：MODERNDESIGN_NAV_MODE = compact | minimal | top
        wchar_t mbuf[64] = {};
        if (GetEnvironmentVariableW(L"MODERNDESIGN_NAV_MODE", mbuf, 64) > 0) {
            std::wstring m = mbuf;
            if (m == L"compact")      nav_.SetDisplayMode(NavigationView::DisplayMode::LeftCompact);
            else if (m == L"minimal") nav_.SetDisplayMode(NavigationView::DisplayMode::LeftMinimal);
            else if (m == L"top")     nav_.SetDisplayMode(NavigationView::DisplayMode::Top);
        }
    }

    // 每帧调用：面板折叠/展开时内容区宽度平滑变化
    void LayoutPages() {
        float s = DpiScale();
        nav_.SetScale(s);
        nav_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));

        // ---- Home 页布局 ----
        {
            float x = ContX(), y = PageTop(); // 标题区下方
            homeBtnStd_.SetText(L"Standard");
            homeBtnStd_.SetVariant(ButtonVariant::Standard);
            homeBtnStd_.SetBounds(RectF(x, y, 96.0f * s, kRowHeight * s));
            homeBtnAcc_.SetText(L"Accent");
            homeBtnAcc_.SetVariant(ButtonVariant::Accent);
            homeBtnAcc_.SetBounds(RectF(x + 108.0f * s, y, 96.0f * s, kRowHeight * s));
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

        // ---- Settings 页布局 ----
        {
            float x = ContX(), y = PageTop();
            setTogA_.SetText(L"Dark mode");
            setTogA_.SetIsOn(!GetTheme().lightMode);
            setTogA_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kRowGap * s;

            setTogB_.SetText(L"Compact pane (48px rail)");
            setTogB_.SetIsOn(nav_.IsCompact());
            setTogB_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kRowGap * s;

            setTogC_.SetText(L"Top navigation");
            setTogC_.SetIsOn(nav_.GetDisplayMode() == NavigationView::DisplayMode::Top);
            setTogC_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));

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
    }

    // Expander 页面：三个示例，覆盖全部能力
    void BindExpanderPage() {
        // ---------- A：HeaderIcon + Description + Content ----------
        expanderA_.SetHeader(L"Feature");
        expanderA_.SetDescription(L"Collapsible container demo");
        expanderA_.SetIsExpanded(true);
        expanderA_.SetContentHeight(64.0f);   // DIP（含 padding）
        expanderA_.SetHeaderIcon([](Renderer& r, const Theme& th, float sc, const RectF& box) {
            // 20×20 信息图标（圆 + i），矢量绘制（无 Fluent Icon 字体依赖）
            float cx = box.CenterX(), cy = box.CenterY();
            float d = box.w;
            Color col = th.TextSecondary();
            r.StrokeEllipse(RectF(cx - d * 0.5f, cy - d * 0.5f, d, d), 1.2f * sc, col);
            float dot = 1.8f * sc;
            r.FillEllipse(RectF(cx - dot * 0.5f, cy - 4.2f * sc, dot, dot), col);
            r.DrawLine(cx, cy - 1.4f * sc, cx, cy + 4.0f * sc, 1.4f * sc, col);
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
        anim |= expanderA_.Update(dt); anim |= expanderB_.Update(dt); anim |= expanderC_.Update(dt);
        anim |= expAToggle_.Update(dt); anim |= expBToggle_.Update(dt); anim |= expCToggle_.Update(dt);
        anim |= expHeaderBtn_.Update(dt);
        anim |= setTogA_.Update(dt); anim |= setTogB_.Update(dt); anim |= setTogC_.Update(dt);
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
            homeChk_.Draw(*this, theme, s); homeTog_.Draw(*this, theme, s);
            homeRadioA_.Draw(*this, theme, s); homeRadioB_.Draw(*this, theme, s);
            homeSlider_.Draw(*this, theme, s); homeProg_.Draw(*this, theme, s);
        } else if (current_ == 1) {
            DrawPageHeader(L"Expander", s);
            expanderA_.Draw(*this, theme, s);
            expanderB_.Draw(*this, theme, s);
            expanderC_.Draw(*this, theme, s);
        } else {
            DrawPageHeader(L"Settings", s);
            setTogA_.Draw(*this, theme, s); setTogB_.Draw(*this, theme, s);
            setTogC_.Draw(*this, theme, s);
        }
    }

    void OnMouseMove(float x, float y) override {
        nav_.OnMouseMove(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseMove(x, y); homeBtnAcc_.OnMouseMove(x, y);
            homeChk_.OnMouseMove(x, y); homeTog_.OnMouseMove(x, y);
            homeRadioA_.OnMouseMove(x, y); homeRadioB_.OnMouseMove(x, y);
            homeSlider_.OnMouseMove(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseMove(x, y); expanderB_.OnMouseMove(x, y); expanderC_.OnMouseMove(x, y);
        } else {
            setTogA_.OnMouseMove(x, y); setTogB_.OnMouseMove(x, y); setTogC_.OnMouseMove(x, y);
        }
    }
    void OnMouseLeave() override {
        nav_.OnMouseLeave();
        if (current_ <= 0) {
            homeBtnStd_.OnMouseLeave(); homeBtnAcc_.OnMouseLeave();
            homeChk_.OnMouseLeave(); homeTog_.OnMouseLeave();
            homeRadioA_.OnMouseLeave(); homeRadioB_.OnMouseLeave();
            homeSlider_.OnMouseLeave();
        } else if (current_ == 1) {
            expanderA_.OnMouseLeave(); expanderB_.OnMouseLeave(); expanderC_.OnMouseLeave();
        } else {
            setTogA_.OnMouseLeave(); setTogB_.OnMouseLeave(); setTogC_.OnMouseLeave();
        }
    }
    void OnMouseDown(float x, float y) override {
        nav_.OnMouseDown(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseDown(x, y); homeBtnAcc_.OnMouseDown(x, y);
            homeChk_.OnMouseDown(x, y); homeTog_.OnMouseDown(x, y);
            homeRadioA_.OnMouseDown(x, y); homeRadioB_.OnMouseDown(x, y);
            homeSlider_.OnMouseDown(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseDown(x, y); expanderB_.OnMouseDown(x, y); expanderC_.OnMouseDown(x, y);
        } else {
            setTogA_.OnMouseDown(x, y); setTogB_.OnMouseDown(x, y); setTogC_.OnMouseDown(x, y);
        }
    }
    void OnMouseUp(float x, float y) override {
        nav_.OnMouseUp(x, y);
        if (current_ <= 0) {
            homeBtnStd_.OnMouseUp(x, y); homeBtnAcc_.OnMouseUp(x, y);
            homeChk_.OnMouseUp(x, y); homeTog_.OnMouseUp(x, y);
            homeRadioA_.OnMouseUp(x, y); homeRadioB_.OnMouseUp(x, y);
            homeSlider_.OnMouseUp(x, y);
        } else if (current_ == 1) {
            expanderA_.OnMouseUp(x, y); expanderB_.OnMouseUp(x, y); expanderC_.OnMouseUp(x, y);
        } else {
            setTogA_.OnMouseUp(x, y); setTogB_.OnMouseUp(x, y); setTogC_.OnMouseUp(x, y);
        }
    }

    void OnThemeChanged() override {
        setTogA_.SetIsOn(!GetTheme().lightMode);
    }
    void OnKeyDown(int vk) override { if (vk == VK_SPACE) ToggleTheme(); }

private:
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

    // Expander（三种示例：图标+描述 / HeaderControls / Up 方向）
    Expander expanderA_, expanderB_, expanderC_;
    ToggleSwitch expAToggle_, expBToggle_, expCToggle_;
    Button expHeaderBtn_;

    // Settings
    ToggleSwitch setTogA_, setTogB_, setTogC_;
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