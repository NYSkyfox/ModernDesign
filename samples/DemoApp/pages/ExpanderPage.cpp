// ExpanderPage — 可折叠容器
#include "ExpanderPage.h"
#include "DemoApp.h"
#include "utils/FluentIcons.h"

using namespace ModernDesign;

namespace ModernDesign::Demo {

void ExpanderPage::Bind() {
    expA_.SetHeader(L"Feature");
    expA_.SetDescription(L"Collapsible container with content");
    expA_.SetIsExpanded(true);
    expA_.SetContentHeight(56.0f);
    expA_.SetHeaderIcon([](Renderer& r, const Theme& th, float sc, const RectF& box) {
        DrawFluentIcon(r, FluentIcon::Info, box.x, box.y, box.w, th.TextSecondary());
    });
    expA_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
        togA_.Draw(r, t, sc);
    });
    expA_.SetContentUpdateFn([this](float dt) { return togA_.Update(dt); });
    expA_.SetContentInputFn(
        [this](float px, float py) { togA_.OnMouseDown(px, py); },
        [this](float px, float py) { togA_.OnMouseUp(px, py); },
        [this](float px, float py) { togA_.OnMouseMove(px, py); },
        [this](float, float)       { togA_.OnMouseLeave(); });
    togA_.SetText(L"Enable feature");
    togA_.SetIsOn(true);

    expB_.SetHeader(L"Notifications");
    expB_.SetDescription(L"Second expander");
    expB_.SetIsExpanded(false);
    expB_.SetContentHeight(56.0f);
    expB_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
        togB_.Draw(r, t, sc);
    });
    expB_.SetContentUpdateFn([this](float dt) { return togB_.Update(dt); });
    expB_.SetContentInputFn(
        [this](float px, float py) { togB_.OnMouseDown(px, py); },
        [this](float px, float py) { togB_.OnMouseUp(px, py); },
        [this](float px, float py) { togB_.OnMouseMove(px, py); },
        [this](float, float)       { togB_.OnMouseLeave(); });
    togB_.SetText(L"Enable notifications");
    togB_.SetIsOn(false);
}

void ExpanderPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();
    float pad = 16.0f * s;
    float rowW = w - 2.0f * pad;
    float mb = Expander::kMarginBottom * s;

    {
        float h = expA_.VisibleHeightDip() * s;
        expA_.SetScale(s);
        expA_.SetBounds(RectF(x, y, w, h));
        togA_.SetBounds(RectF(x + pad, y + expA_.HeaderHeight() * s + pad, rowW, kRowHeight * s));
        y += h + mb;
    }
    {
        float h = expB_.VisibleHeightDip() * s;
        expB_.SetScale(s);
        expB_.SetBounds(RectF(x, y, w, h));
        togB_.SetBounds(RectF(x + pad, y + pad, rowW, kRowHeight * s));
        y += h + mb;
    }
}

bool ExpanderPage::Update(float dt) {
    bool anim = false;
    anim |= expA_.Update(dt); anim |= expB_.Update(dt);
    anim |= togA_.Update(dt); anim |= togB_.Update(dt);
    return anim;
}

void ExpanderPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    expA_.Draw(r, t, s);
    expB_.Draw(r, t, s);
}

void ExpanderPage::OnMouseMove(float x, float y) { expA_.OnMouseMove(x, y); expB_.OnMouseMove(x, y); }
void ExpanderPage::OnMouseDown(float x, float y) { expA_.OnMouseDown(x, y); expB_.OnMouseDown(x, y); }
void ExpanderPage::OnMouseUp(float x, float y) { expA_.OnMouseUp(x, y); expB_.OnMouseUp(x, y); }
void ExpanderPage::OnMouseLeave() { expA_.OnMouseLeave(); expB_.OnMouseLeave(); }

} // namespace ModernDesign::Demo