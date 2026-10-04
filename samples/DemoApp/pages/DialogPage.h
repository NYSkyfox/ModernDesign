#pragma once
// DialogPage — ContentDialog
#include "DemoPage.h"
#include "controls/Button.h"

class DemoWindow;

namespace ModernDesign::Demo {

class DialogPage : public DemoPage {
public:
    explicit DialogPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Dialog"; }
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;
};

} // namespace ModernDesign::Demo
