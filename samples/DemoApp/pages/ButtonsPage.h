#pragma once
// ButtonsPage — Button + HyperlinkButton
#include "DemoPage.h"
#include "controls/Button.h"
#include "controls/HyperlinkButton.h"

class DemoWindow;

namespace ModernDesign::Demo {

class ButtonsPage : public DemoPage {
public:
    explicit ButtonsPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Buttons"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    Button btnStd_, btnAccent_, btnDisabled_;
    HyperlinkButton linkA_, linkB_;
};

} // namespace ModernDesign::Demo
