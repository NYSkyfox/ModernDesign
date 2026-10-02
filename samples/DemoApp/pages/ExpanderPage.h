#pragma once
// ExpanderPage — 可折叠容器示例页
#include "DemoPage.h"
#include "controls/Expander.h"
#include "controls/ToggleSwitch.h"
#include "controls/Button.h"

using namespace ModernDesign::Controls;

class DemoWindow;

namespace ModernDesign::Demo {

class ExpanderPage : public DemoPage {
public:
    explicit ExpanderPage(DemoWindow* owner) : DemoPage(owner) {}

    std::wstring Title() const override { return L"Expander"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    Expander expanderA_, expanderB_, expanderC_;
    ToggleSwitch expAToggle_, expBToggle_, expCToggle_;
    Button expHeaderBtn_;
};

} // namespace ModernDesign::Demo