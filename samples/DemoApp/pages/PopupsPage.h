#pragma once
// PopupsPage — Flyout + MenuFlyout + ToolTip
#include "DemoPage.h"
#include "controls/Button.h"

using namespace ModernDesign::Controls;
class DemoWindow;

namespace ModernDesign::Demo {

class PopupsPage : public DemoPage {
public:
    explicit PopupsPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Popups"; }
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;
};

} // namespace ModernDesign::Demo
