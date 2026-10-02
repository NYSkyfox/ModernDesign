#pragma once
// SettingsPage — 设置卡片示例页
#include "DemoPage.h"
#include "controls/SettingsCard.h"
#include "controls/ToggleSwitch.h"

using namespace ModernDesign::Controls;

class DemoWindow;

namespace ModernDesign::Demo {

class SettingsPage : public DemoPage {
public:
    explicit SettingsPage(DemoWindow* owner) : DemoPage(owner) {}

    std::wstring Title() const override { return L"Settings"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;
    void OnThemeChanged() override;

private:
    SettingsCard setCardA_, setCardB_, setCardC_, setCardD_;
    ToggleSwitch setTogA_, setTogB_, setTogC_;
    bool setBound_ = false;
};

} // namespace ModernDesign::Demo