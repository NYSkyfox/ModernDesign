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

            BindExpanderPage();
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
            float pad = 16.0f * s;
            float rowW = FzMn(w - 2.0f * pad, 240.0f * s);
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
        anim |= expanderA_.Update(dt); anim |= expanderB_.Update(dt); anim |= expanderC_.Update(dt);
        anim |= expAToggle_.Update(dt); anim |= expBToggle_.Update(dt); anim |= expCToggle_.Update(dt);
        anim |= expHeaderBtn_.Update(dt);
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
            expanderA_.Draw(*this, theme, s);
            expanderB_.Draw(*this, theme, s);
            expanderC_.Draw(*this, theme, s);
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
            expanderA_.OnMouseMove(x, y); expanderB_.OnMouseMove(x, y); expanderC_.OnMouseMove(x, y);
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
            expanderA_.OnMouseLeave(); expanderB_.OnMouseLeave(); expanderC_.OnMouseLeave();
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
            expanderA_.OnMouseDown(x, y); expanderB_.OnMouseDown(x, y); expanderC_.OnMouseDown(x, y);
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
            expanderA_.OnMouseUp(x, y); expanderB_.OnMouseUp(x, y); expanderC_.OnMouseUp(x, y);
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

    // Expander（三种示例：图标+描述 / HeaderControls / Up 方向）
    Expander expanderA_, expanderB_, expanderC_;
    ToggleSwitch expAToggle_, expBToggle_, expCToggle_;
    Button expHeaderBtn_;

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