#pragma once
// CardPage — Card + SettingsCard
#include "DemoPage.h"
#include "controls/Card.h"
#include "controls/SettingsCard.h"
#include "controls/ToggleSwitch.h"

using namespace ModernDesign::Controls;
class DemoWindow;

namespace ModernDesign::Demo {

class CardPage : public DemoPage {
public:
    explicit CardPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Card"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    Card simpleCard_;
    SettingsCard setA_, setB_;
    ToggleSwitch togA_;
};

} // namespace ModernDesign::Demo
