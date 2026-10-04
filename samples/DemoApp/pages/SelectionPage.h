#pragma once
// SelectionPage — CheckBox + RadioButton + ToggleSwitch
#include "DemoPage.h"
#include "controls/CheckBox.h"
#include "controls/RadioButton.h"
#include "controls/ToggleSwitch.h"

class DemoWindow;

namespace ModernDesign::Demo {

class SelectionPage : public DemoPage {
public:
    explicit SelectionPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Selection"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    CheckBox chkChecked_, chkUnchecked_, chkDisabled_;
    RadioButton radioA_, radioB_, radioC_;
    ToggleSwitch togOn_, togOff_, togDisabled_;
};

} // namespace ModernDesign::Demo
