// ExpanderPage — 可折叠容器示例
#include "ExpanderPage.h"
#include "DemoApp.h"
#include "utils/FluentIcons.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace ModernDesign::Demo {

void ExpanderPage::Bind() {
    // ---------- A：HeaderIcon + Description + Content ----------
    expanderA_.SetHeader(L"Feature");
    expanderA_.SetDescription(L"Collapsible container demo");
    expanderA_.SetIsExpanded(true);
    expanderA_.SetContentHeight(64.0f);
    expanderA_.SetHeaderIcon([](Renderer& r, const Theme& th, float sc, const RectF& box) {
        DrawFluentIcon(r, FluentIcon::Info, box.x, box.y, box.w, th.TextSecondary());
    });
    expanderA_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
        expAToggle_.Draw(r, t, sc);
    });
    expanderA_.SetContentUpdateFn([this](float dt) { return expAToggle_.Update(dt); });
    expanderA_.SetContentInputFn(
        [this](float px, float py) { expAToggle_.OnMouseDown(px, py); },
        [this](float px, float py) { expAToggle_.OnMouseUp(px, py); },
        [this](float px, float py) { expAToggle_.OnMouseMove(px, py); },
        [this](float, float)       { expAToggle_.OnMouseLeave(); });
    expAToggle_.SetText(L"Enable feature");
    expAToggle_.SetIsOn(true);

    // ---------- B：HeaderControls ----------
    expanderB_.SetHeader(L"Notifications");
    expanderB_.SetIsExpanded(true);
    expanderB_.SetContentHeight(64.0f);
    expanderB_.SetHeaderControls(88.0f, [this](Renderer& r, const Theme& t, float sc, const RectF&) {
        expHeaderBtn_.Draw(r, t, sc);
    });
    expanderB_.SetHeaderControlsInputFn(
        [this](float px, float py) { expHeaderBtn_.OnMouseDown(px, py); },
        [this](float px, float py) { expHeaderBtn_.OnMouseUp(px, py); },
        [this](float px, float py) { expHeaderBtn_.OnMouseMove(px, py); },
        [this](float, float)       { expHeaderBtn_.OnMouseLeave(); });
    expHeaderBtn_.SetText(L"Reset");
    expHeaderBtn_.SetVariant(ButtonVariant::Standard);
    expanderB_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
        expBToggle_.Draw(r, t, sc);
    });
    expanderB_.SetContentUpdateFn([this](float dt) { return expBToggle_.Update(dt); });
    expanderB_.SetContentInputFn(
        [this](float px, float py) { expBToggle_.OnMouseDown(px, py); },
        [this](float px, float py) { expBToggle_.OnMouseUp(px, py); },
        [this](float px, float py) { expBToggle_.OnMouseMove(px, py); },
        [this](float, float)       { expBToggle_.OnMouseLeave(); });
    expBToggle_.SetText(L"Enable notifications");
    expBToggle_.SetIsOn(true);

    // ---------- C：ExpandDirection = Up ----------
    expanderC_.SetDirection(Expander::Direction::Up);
    expanderC_.SetHeader(L"Advanced");
    expanderC_.SetDescription(L"ExpandDirection = Up");
    expanderC_.SetIsExpanded(true);
    expanderC_.SetContentHeight(64.0f);
    expanderC_.SetContentDrawFn([this](Renderer& r, const Theme& t, float sc, const RectF&) {
        expCToggle_.Draw(r, t, sc);
    });
    expanderC_.SetContentUpdateFn([this](float dt) { return expCToggle_.Update(dt); });
    expanderC_.SetContentInputFn(
        [this](float px, float py) { expCToggle_.OnMouseDown(px, py); },
        [this](float px, float py) { expCToggle_.OnMouseUp(px, py); },
        [this](float px, float py) { expCToggle_.OnMouseMove(px, py); },
        [this](float, float)       { expCToggle_.OnMouseLeave(); });
    expCToggle_.SetText(L"Advanced mode");
    expCToggle_.SetIsOn(false);
}

void ExpanderPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();
    float pad = 16.0f * s;
    float rowW = w - 2.0f * pad;
    float mb = Expander::kMarginBottom * s;

    {
        float h = expanderA_.VisibleHeightDip() * s;
        expanderA_.SetScale(s);
        expanderA_.SetBounds(RectF(x, y, w, h));
        expAToggle_.SetBounds(RectF(x + pad, y + expanderA_.HeaderHeight() * s + pad, rowW, kRowHeight * s));
        y += h + mb;
    }
    {
        float h = expanderB_.VisibleHeightDip() * s;
        expanderB_.SetScale(s);
        expanderB_.SetBounds(RectF(x, y, w, h));
        RectF cb = expanderB_.HeaderControlsBox(s);
        expHeaderBtn_.SetBounds(RectF(cb.x, cb.y + (cb.h - 32.0f * s) * 0.5f, cb.w, 32.0f * s));
        expBToggle_.SetBounds(RectF(x + pad, y + expanderB_.HeaderHeight() * s + pad, rowW, kRowHeight * s));
        y += h + mb;
    }
    {
        float h = expanderC_.VisibleHeightDip() * s;
        expanderC_.SetScale(s);
        expanderC_.SetBounds(RectF(x, y, w, h));
        expCToggle_.SetBounds(RectF(x + pad, y + pad, rowW, kRowHeight * s));
        y += h + mb;
    }
}

bool ExpanderPage::Update(float dt) {
    bool anim = false;
    anim |= expanderA_.Update(dt); anim |= expanderB_.Update(dt); anim |= expanderC_.Update(dt);
    anim |= expAToggle_.Update(dt); anim |= expBToggle_.Update(dt); anim |= expCToggle_.Update(dt);
    anim |= expHeaderBtn_.Update(dt);
    return anim;
}

void ExpanderPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    expanderA_.Draw(r, t, s);
    expanderB_.Draw(r, t, s);
    expanderC_.Draw(r, t, s);
}

void ExpanderPage::OnMouseMove(float x, float y) {
    expanderA_.OnMouseMove(x, y); expanderB_.OnMouseMove(x, y); expanderC_.OnMouseMove(x, y);
}
void ExpanderPage::OnMouseDown(float x, float y) {
    expanderA_.OnMouseDown(x, y); expanderB_.OnMouseDown(x, y); expanderC_.OnMouseDown(x, y);
}
void ExpanderPage::OnMouseUp(float x, float y) {
    expanderA_.OnMouseUp(x, y); expanderB_.OnMouseUp(x, y); expanderC_.OnMouseUp(x, y);
}
void ExpanderPage::OnMouseLeave() {
    expanderA_.OnMouseLeave(); expanderB_.OnMouseLeave(); expanderC_.OnMouseLeave();
}

} // namespace ModernDesign::Demo