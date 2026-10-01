// ============================================================
// ModernDesign DemoApp — 演示应用（导航窗格 + 多页面）
//
//   左侧：NavigationView 侧边栏（hamburger 折叠 + 菜单项 + accent 指示条）
//   右侧：根据选中项切换页面
//       Home     — 基础控件合集
//       Expander — 可折叠容器示例
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
    // 内容区（面板右侧）几何
    float PaneW() const { return nav_.PaneWidthDip() * DpiScale(); }
    float ContX() const { return PaneW() + kMargin * DpiScale(); }
    float ContW() const { return FzMx(0.0f, ClientWidth() - PaneW() - kMargin * DpiScale() * 2); }

    void OnLayout() override {
        float s = DpiScale();
        nav_.SetScale(s);
        nav_.SetBounds(RectF(0, 0, ClientWidth(), ClientHeight()));

        static bool bound = false;
        if (!bound) {
            bound = true;
            // 菜单项/标题只绑定一次（避免每次 OnLayout 重复 AddItem）
            nav_.SetPaneTitle(L"Modern Design");
            nav_.AddItem({ L"Home", 0, false });
            nav_.AddItem({ L"Expander", 1, false });
            nav_.SetSettings(L"Settings");
            nav_.SetSelectedIndex(0);   // 默认 Home 页
            nav_.SetSelectionCallback([this](int i) { current_ = i; Invalidate(); });

            // Expander 页面：内容区放一个 ToggleSwitch
            expander_.SetHeader(L"Feature");
            expander_.SetDescription(L"Collapsible container demo");
            expander_.SetIsExpanded(true);
            expander_.SetContentHeight(64.0f); // DIP 自然高（含 padding）
            expander_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF& inner) {
                expanderToggle_.Draw(r, t, sc);
            });
            expander_.SetContentUpdateFn([this](float dt) { return expanderToggle_.Update(dt); });
            expander_.SetContentInputFn(
                [this](float x, float y) { expanderToggle_.OnMouseDown(x, y); },
                [this](float x, float y) { expanderToggle_.OnMouseUp(x, y); },
                [this](float x, float y) { expanderToggle_.OnMouseMove(x, y); },
                [this](float x, float y) { expanderToggle_.OnMouseLeave(); });
        }

        // ---- Home 页布局 ----
        {
            float x = ContX(), y = kMargin * s + 70.0f * s; // 标题区下方
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
            float x = ContX(), y = kMargin * s + 70.0f * s;
            float w = FzMn(ContW(), 460.0f * s);
            expander_.SetBounds(RectF(x, y, w, (expander_.HeaderHeight() + expander_.ContentHeight()) * s));
            float pad = 16.0f * s;
            expanderToggle_.SetText(L"Enable feature");
            expanderToggle_.SetIsOn(true);
            expanderToggle_.SetBounds(RectF(x + pad, y + 48.0f * s + pad, FzMn(ContW(), 220.0f) * s, kRowHeight * s));
        }

        // ---- Settings 页布局 ----
        {
            float x = ContX(), y = kMargin * s + 70.0f * s;
            setTogA_.SetText(L"Dark mode");
            setTogA_.SetIsOn(!GetTheme().lightMode);
            setTogA_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            y += kRowHeight * s + kRowGap * s;
            setTogB_.SetText(L"Notifications");
            setTogB_.SetIsOn(true);
            setTogB_.SetBounds(RectF(x, y, ContW(), kRowHeight * s));
            if (!setBound_) {
                setBound_ = true;
                setTogA_.SetChangedCallback([this](bool on) { if (on != !GetTheme().lightMode) ToggleTheme(); });
            }
        }
    }

    void DrawPageHeader(const std::wstring& title, float s) {
        DrawText(title, ContX(), kMargin * s + 24.0f * s, ContW(), 44.0f * s,
                 L"Segoe UI", 28.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, GetTheme().TextPrimary(),
                 DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        DrawLine(ContX(), kMargin * s + 66.0f * s, ContX() + ContW(), kMargin * s + 66.0f * s,
                 1.0f * s, GetTheme().CardBorder());
    }

    bool OnUpdate(float dt) override {
        bool anim = nav_.Update(dt);
        anim |= homeBtnStd_.Update(dt); anim |= homeBtnAcc_.Update(dt);
        anim |= homeChk_.Update(dt); anim |= homeTog_.Update(dt);
        anim |= homeRadioA_.Update(dt); anim |= homeRadioB_.Update(dt);
        anim |= homeSlider_.Update(dt);
        anim |= expander_.Update(dt);
        anim |= expanderToggle_.Update(dt);
        anim |= setTogA_.Update(dt); anim |= setTogB_.Update(dt);
        return anim;
    }

    void OnRender() override {
        float s = DpiScale();
        const Theme& theme = GetTheme();

        nav_.Draw(*this, theme, s); // 画 panel + content 背景
        if (current_ <= 0) {
            DrawPageHeader(L"Home", s);
            homeBtnStd_.Draw(*this, theme, s); homeBtnAcc_.Draw(*this, theme, s);
            homeChk_.Draw(*this, theme, s); homeTog_.Draw(*this, theme, s);
            homeRadioA_.Draw(*this, theme, s); homeRadioB_.Draw(*this, theme, s);
            homeSlider_.Draw(*this, theme, s); homeProg_.Draw(*this, theme, s);
        } else if (current_ == 1) {
            DrawPageHeader(L"Expander", s);
            expander_.Draw(*this, theme, s);
        } else {
            DrawPageHeader(L"Settings", s);
            setTogA_.Draw(*this, theme, s); setTogB_.Draw(*this, theme, s);
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
            expander_.OnMouseMove(x, y);
        } else {
            setTogA_.OnMouseMove(x, y); setTogB_.OnMouseMove(x, y);
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
            expander_.OnMouseLeave();
        } else {
            setTogA_.OnMouseLeave(); setTogB_.OnMouseLeave();
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
            expander_.OnMouseDown(x, y);
        } else {
            setTogA_.OnMouseDown(x, y); setTogB_.OnMouseDown(x, y);
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
            expander_.OnMouseUp(x, y);
        } else {
            setTogA_.OnMouseUp(x, y); setTogB_.OnMouseUp(x, y);
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

    // Home
    Button homeBtnStd_, homeBtnAcc_;
    CheckBox homeChk_;
    ToggleSwitch homeTog_;
    RadioButton homeRadioA_, homeRadioB_;
    Slider homeSlider_;
    ProgressBar homeProg_;

    // Expander
    Expander expander_;
    ToggleSwitch expanderToggle_;

    // Settings
    ToggleSwitch setTogA_, setTogB_;
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