// ============================================================
// ModernDesign DemoApp — 演示应用
//
// 展示框架的 8 种基础控件。
//   鼠标：悬停 / 点击 / 拖拽
//   键盘：Space 切换深浅主题
// ============================================================

#include "pch.h"
#include "app/App.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ToggleSwitch.h"
#include "controls/RadioButton.h"
#include "controls/ProgressBar.h"
#include "controls/ProgressRing.h"
#include "controls/Slider.h"
#include "controls/Card.h"
#include "controls/TextBlock.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace {

// 演示窗口尺寸（客户区逻辑像素）
constexpr float kMargin = 32.0f;
constexpr float kColumnGap = 40.0f;
constexpr float kRowGap = 8.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kGroupGap = 28.0f;

} // namespace

// ============================================================
// DemoWindow
// ============================================================
class DemoWindow : public App {
protected:
    void OnLayout() override {
        float s = DpiScale();
        float colW = FzMn((ClientWidth() - kMargin * 2 - kColumnGap) * 0.5f, 520.0f);

        float x = kMargin * s;
        float y = kMargin * s;

        // ---- 标题区 ----
        title_.SetText(L"Modern Design");
        title_.SetFontSize(26.0f);
        title_.SetBold(true);
        title_.SetBounds(RectF(x, y, colW * 2 + kColumnGap * s, 34.0f * s));
        y += 40.0f * s;

        subtitle_.SetText(L"Fluent Design (WinUI 3) framework — pure C++ / Direct2D");
        subtitle_.SetFontSize(12.0f);
        subtitle_.SetBounds(RectF(x, y, colW * 2 + kColumnGap * s, 18.0f * s));
        y += 28.0f * s;

        dividerY_ = y;
        dividerW_ = colW * 2 + kColumnGap * s;
        y += 24.0f * s;

        // ============================================================
        // 左栏
        // ============================================================
        float ly = y;
        float lx = x;

        // Buttons
        float bw = 88.0f * s;
        float bh = kRowHeight * s;
        float bg = kRowGap * s;

        btnStandard_.SetText(L"Standard");
        btnStandard_.SetVariant(ButtonVariant::Standard);
        btnStandard_.SetBounds(RectF(lx, ly, bw, bh));

        btnAccent_.SetText(L"Accent");
        btnAccent_.SetVariant(ButtonVariant::Accent);
        btnAccent_.SetBounds(RectF(lx + (bw + bg), ly, bw, bh));

        btnSubtle_.SetText(L"Subtle");
        btnSubtle_.SetVariant(ButtonVariant::Subtle);
        btnSubtle_.SetBounds(RectF(lx + (bw + bg) * 2, ly, bw, bh));

        btnDisabled_.SetText(L"Disabled");
        btnDisabled_.SetVariant(ButtonVariant::Standard);
        btnDisabled_.SetEnabled(false);
        btnDisabled_.SetBounds(RectF(lx + (bw + bg) * 3, ly, bw, bh));

        ly += bh + kGroupGap * s;

        // CheckBoxes
        chkA_.SetText(L"CheckBox unchecked");
        chkA_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s;

        chkB_.SetText(L"CheckBox checked");
        chkB_.SetChecked(true);
        chkB_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s + kGroupGap * s;

        // ToggleSwitches
        toggleA_.SetText(L"Toggle off");
        toggleA_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s;

        toggleB_.SetText(L"Toggle on");
        toggleB_.SetIsOn(true);
        toggleB_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s;

        toggleC_.SetText(L"Toggle disabled");
        toggleC_.SetEnabled(false);
        toggleC_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s + kGroupGap * s;

        // RadioButtons（互斥）
        radioA_.SetText(L"RadioButton unchecked");
        radioA_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s;

        radioB_.SetText(L"RadioButton checked");
        radioB_.SetSelected(true);
        radioB_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s + kGroupGap * s;

        // Slider
        slider_.SetBounds(RectF(lx, ly, colW, kRowHeight * s));
        ly += kRowHeight * s + kGroupGap * s;

        // ProgressBars
        progressDeterminate_.SetProgress(0.65f);
        progressDeterminate_.SetBounds(RectF(lx, ly, colW, 12.0f * s));
        ly += 28.0f * s;

        progressIndeterminate_.SetIndeterminate(true);
        progressIndeterminate_.SetBounds(RectF(lx, ly, colW, 12.0f * s));

        // ============================================================
        // 右栏
        // ============================================================
        float rx = x + colW + kColumnGap * s;
        float ry = y;

        // ProgressRing
        ringIndeterminate_.SetSize(44.0f);
        ringIndeterminate_.SetIndeterminate(true);
        ringIndeterminate_.SetBounds(RectF(rx, ry, 60.0f * s, 44.0f * s));
        ry += 60.0f * s;

        ringDeterminate_.SetSize(44.0f);
        ringDeterminate_.SetProgress(0.7f);
        ringDeterminate_.SetBounds(RectF(rx, ry, 60.0f * s, 44.0f * s));
        ry += 72.0f * s;

        // Cards
        cardProfile_.SetTitle(L"Profile");
        cardProfile_.SetContent(
            L"Name     NYSkyfox\n"
            L"Role     Developer\n"
            L"Location  Earth");
        float h1 = cardProfile_.MeasureHeight(GetTheme(), s);
        cardProfile_.SetBounds(RectF(rx, ry, colW, h1));
        ry += h1 + kRowGap * s * 2;

        cardAbout_.SetTitle(L"About ModernDesign");
        cardAbout_.SetContent(
            L"Zero external dependencies.\n"
            L"Only Win32 + Direct2D + DirectWrite.\n"
            L"\n"
            L"Press Space to toggle theme.");
        float h2 = cardAbout_.MeasureHeight(GetTheme(), s);
        cardAbout_.SetBounds(RectF(rx, ry, colW, h2));

        // 主题切换开关回调（放在 OnLayout 外避免重复绑定）
        static bool bound = false;
        if (!bound) {
            bound = true;
            toggleTheme_.SetText(L"Dark mode");
            toggleTheme_.SetChangedCallback([this](bool on) {
                if (on != !GetTheme().lightMode) {
                    ToggleTheme();
                }
            });
            toggleTheme_.SetBounds(RectF(x, ClientHeight() - 48.0f * s, colW, kRowHeight * s));
        }
    }

    bool OnUpdate(float dt) override {
        bool animating = false;
        animating |= btnStandard_.Update(dt);
        animating |= btnAccent_.Update(dt);
        animating |= btnSubtle_.Update(dt);
        animating |= btnDisabled_.Update(dt);
        animating |= chkA_.Update(dt);
        animating |= chkB_.Update(dt);
        animating |= toggleA_.Update(dt);
        animating |= toggleB_.Update(dt);
        animating |= toggleC_.Update(dt);
        animating |= toggleTheme_.Update(dt);
        animating |= radioA_.Update(dt);
        animating |= radioB_.Update(dt);
        animating |= slider_.Update(dt);
        animating |= progressIndeterminate_.Update(dt);
        animating |= ringIndeterminate_.Update(dt);
        return animating;
    }

    void OnRender() override {
        float s = DpiScale();
        const Theme& theme = GetTheme();

        title_.Draw(*this, theme, s);
        subtitle_.Draw(*this, theme, s);

        // 分割线
        DrawLine(kMargin * s, dividerY_, kMargin * s + dividerW_, dividerY_,
                 1.0f * s, theme.CardBorder());

        // Buttons
        btnStandard_.Draw(*this, theme, s);
        btnAccent_.Draw(*this, theme, s);
        btnSubtle_.Draw(*this, theme, s);
        btnDisabled_.Draw(*this, theme, s);

        // CheckBoxes
        chkA_.Draw(*this, theme, s);
        chkB_.Draw(*this, theme, s);

        // Toggles
        toggleA_.Draw(*this, theme, s);
        toggleB_.Draw(*this, theme, s);
        toggleC_.Draw(*this, theme, s);
        toggleTheme_.Draw(*this, theme, s);

        // Radios
        radioA_.Draw(*this, theme, s);
        radioB_.Draw(*this, theme, s);

        // Slider
        slider_.Draw(*this, theme, s);

        // Progress
        progressDeterminate_.Draw(*this, theme, s);
        progressIndeterminate_.Draw(*this, theme, s);
        ringIndeterminate_.Draw(*this, theme, s);
        ringDeterminate_.Draw(*this, theme, s);

        // Cards
        cardProfile_.Draw(*this, theme, s);
        cardAbout_.Draw(*this, theme, s);
    }

    void OnMouseMove(float x, float y) override {
        btnStandard_.OnMouseMove(x, y);
        btnAccent_.OnMouseMove(x, y);
        btnSubtle_.OnMouseMove(x, y);
        btnDisabled_.OnMouseMove(x, y);
        chkA_.OnMouseMove(x, y);
        chkB_.OnMouseMove(x, y);
        toggleA_.OnMouseMove(x, y);
        toggleB_.OnMouseMove(x, y);
        toggleC_.OnMouseMove(x, y);
        toggleTheme_.OnMouseMove(x, y);
        radioA_.OnMouseMove(x, y);
        radioB_.OnMouseMove(x, y);
        slider_.OnMouseMove(x, y);
    }

    void OnMouseLeave() override {
        btnStandard_.OnMouseLeave();
        btnAccent_.OnMouseLeave();
        btnSubtle_.OnMouseLeave();
        btnDisabled_.OnMouseLeave();
        chkA_.OnMouseLeave();
        chkB_.OnMouseLeave();
        toggleA_.OnMouseLeave();
        toggleB_.OnMouseLeave();
        toggleC_.OnMouseLeave();
        toggleTheme_.OnMouseLeave();
        radioA_.OnMouseLeave();
        radioB_.OnMouseLeave();
        slider_.OnMouseLeave();
    }

    void OnMouseDown(float x, float y) override {
        btnStandard_.OnMouseDown(x, y);
        btnAccent_.OnMouseDown(x, y);
        btnSubtle_.OnMouseDown(x, y);
        btnDisabled_.OnMouseDown(x, y);
        radioA_.OnMouseDown(x, y);
        radioB_.OnMouseDown(x, y);
        slider_.OnMouseDown(x, y);
    }

    void OnMouseUp(float x, float y) override {
        btnStandard_.OnMouseUp(x, y);
        btnAccent_.OnMouseUp(x, y);
        btnSubtle_.OnMouseUp(x, y);
        btnDisabled_.OnMouseUp(x, y);
        chkA_.OnMouseUp(x, y);
        chkB_.OnMouseUp(x, y);
        toggleA_.OnMouseUp(x, y);
        toggleB_.OnMouseUp(x, y);
        toggleC_.OnMouseUp(x, y);
        toggleTheme_.OnMouseUp(x, y);
        radioA_.OnMouseUp(x, y);
        radioB_.OnMouseUp(x, y);
        slider_.OnMouseUp(x, y);
    }

    void OnThemeChanged() override {
        toggleTheme_.SetIsOn(!GetTheme().lightMode);
    }

    void OnKeyDown(int vk) override {
        if (vk == VK_SPACE) {
            ToggleTheme();
        }
    }

private:
    float dividerY_ = 0.0f;
    float dividerW_ = 0.0f;

    TextBlock title_;
    TextBlock subtitle_;

    Button btnStandard_;
    Button btnAccent_;
    Button btnSubtle_;
    Button btnDisabled_;

    CheckBox chkA_;
    CheckBox chkB_;

    ToggleSwitch toggleA_;
    ToggleSwitch toggleB_;
    ToggleSwitch toggleC_;
    ToggleSwitch toggleTheme_;

    RadioButton radioA_;
    RadioButton radioB_;

    Slider slider_;
    ProgressBar progressDeterminate_;
    ProgressBar progressIndeterminate_;
    ProgressRing ringIndeterminate_;
    ProgressRing ringDeterminate_;

    Card cardProfile_;
    Card cardAbout_;
};

// ============================================================
// 入口
// ============================================================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    DemoWindow app;
    HRESULT hr = app.Initialize(hInstance, nCmdShow);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"ModernDesign 初始化失败（需要 Win10 1809+）",
                    L"ModernDesign", MB_ICONERROR);
        return 1;
    }
    return app.Run();
}