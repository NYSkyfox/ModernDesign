#pragma once
// SlidersPage — Slider + ProgressBar + ProgressRing
#include "DemoPage.h"
#include "controls/Slider.h"
#include "controls/ProgressBar.h"
#include "controls/ProgressRing.h"

using namespace ModernDesign::Controls;
class DemoWindow;

namespace ModernDesign::Demo {

class SlidersPage : public DemoPage {
public:
    explicit SlidersPage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Sliders"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseDown(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    Slider slider_;
    ProgressBar progBar_;
    ProgressRing progRing_;
};

} // namespace ModernDesign::Demo
