#pragma once
// ExpanderPage — 可折叠容器
#include "DemoPage.h"
#include "controls/Expander.h"
#include "controls/ToggleSwitch.h"

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
    Expander expA_, expB_;
    ToggleSwitch togA_, togB_;
};

} // namespace ModernDesign::Demo