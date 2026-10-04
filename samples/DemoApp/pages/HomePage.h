#pragma once
// HomePage — 框架概览页
#include "DemoPage.h"
#include "controls/SettingsCard.h"
#include "controls/HyperlinkButton.h"

using namespace ModernDesign::Controls;
class DemoWindow;

namespace ModernDesign::Demo {

class HomePage : public DemoPage {
public:
    explicit HomePage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Modern Design"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    SettingsCard cardInput_, cardContainer_, cardPopup_;
    HyperlinkButton linkRepo_;
};

} // namespace ModernDesign::Demo