#pragma once
// ShowcasePage — B1 新增控件：InfoBadge / PipsPager / RatingControl / PersonPicture
#include "DemoPage.h"
#include "controls/InfoBadge.h"
#include "controls/PipsPager.h"
#include "controls/RatingControl.h"
#include "controls/PersonPicture.h"

class DemoWindow;

namespace ModernDesign::Demo {

class ShowcasePage : public DemoPage {
public:
    explicit ShowcasePage(DemoWindow* owner) : DemoPage(owner) {}
    std::wstring Title() const override { return L"Showcase"; }
    void Bind() override;
    void Layout() override;
    bool Update(float dt) override;
    void Draw() override;
    void OnMouseMove(float x, float y) override;
    void OnMouseUp(float x, float y) override;
    void OnMouseLeave() override;

private:
    InfoBadge badgeValue_;
    InfoBadge badgeCount_;
    InfoBadge badgeIcon_;
    InfoBadge badgeText_;

    PipsPager pips_;
    RatingControl rating_;
    RatingControl ratingRo_;

    PersonPicture ppAvail_;
    PersonPicture ppBusy_;
    PersonPicture ppAway_;
    PersonPicture ppOffline_;
};

} // namespace ModernDesign::Demo