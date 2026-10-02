#pragma once
// HomePage — 基础控件合集页
#include "DemoPage.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ToggleSwitch.h"
#include "controls/RadioButton.h"
#include "controls/Slider.h"
#include "controls/ProgressBar.h"
#include "controls/HyperlinkButton.h"

using namespace ModernDesign::Controls;

class DemoWindow;

namespace ModernDesign::Demo {

class HomePage : public DemoPage {
public:
    explicit HomePage(DemoWindow* owner) : DemoPage(owner) {}

    std::wstring Title() const override { return L"Home"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    Button homeBtnStd_, homeBtnAcc_;
    CheckBox homeChk_;
    ToggleSwitch homeTog_;
    RadioButton homeRadioA_, homeRadioB_;
    Slider homeSlider_;
    ProgressBar homeProg_;
    HyperlinkButton homeLinkA_, homeLinkB_;
};

} // namespace ModernDesign::Demo