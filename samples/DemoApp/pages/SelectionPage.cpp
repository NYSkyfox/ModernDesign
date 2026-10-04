// SelectionPage — CheckBox + RadioButton + ToggleSwitch
#include "SelectionPage.h"
#include "DemoApp.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace ModernDesign::Demo {

void SelectionPage::Bind() {
    chkChecked_.SetText(L"CheckBox checked");
    chkChecked_.SetChecked(true);
    chkUnchecked_.SetText(L"CheckBox unchecked");
    chkUnchecked_.SetChecked(false);
    chkDisabled_.SetText(L"CheckBox disabled");
    chkDisabled_.SetChecked(true);
    chkDisabled_.SetEnabled(false);

    radioA_.SetText(L"Option A");
    radioB_.SetText(L"Option B");
    radioC_.SetText(L"Option C");
    radioB_.SetSelected(true);

    togOn_.SetText(L"Toggle on");
    togOn_.SetIsOn(true);
    togOff_.SetText(L"Toggle off");
    togOff_.SetIsOn(false);
    togDisabled_.SetText(L"Toggle disabled");
    togDisabled_.SetIsOn(true);
    togDisabled_.SetEnabled(false);
}

void SelectionPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();

    owner_->DrawText( L"CheckBox", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    chkChecked_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    chkUnchecked_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    chkDisabled_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kGroupGap) * s;

    owner_->DrawText( L"RadioButton", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    radioA_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    radioB_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    radioC_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kGroupGap) * s;

    owner_->DrawText( L"ToggleSwitch", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    togOn_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    togOff_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kRowGap) * s;
    togDisabled_.SetBounds(RectF(x, y, w, kRowHeight * s));
}

bool SelectionPage::Update(float dt) {
    bool anim = false;
    anim |= chkChecked_.Update(dt); anim |= chkUnchecked_.Update(dt); anim |= chkDisabled_.Update(dt);
    anim |= radioA_.Update(dt); anim |= radioB_.Update(dt); anim |= radioC_.Update(dt);
    anim |= togOn_.Update(dt); anim |= togOff_.Update(dt); anim |= togDisabled_.Update(dt);
    return anim;
}

void SelectionPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    chkChecked_.Draw(r, t, s); chkUnchecked_.Draw(r, t, s); chkDisabled_.Draw(r, t, s);
    radioA_.Draw(r, t, s); radioB_.Draw(r, t, s); radioC_.Draw(r, t, s);
    togOn_.Draw(r, t, s); togOff_.Draw(r, t, s); togDisabled_.Draw(r, t, s);
}

void SelectionPage::OnMouseMove(float x, float y) {
    chkChecked_.OnMouseMove(x, y); chkUnchecked_.OnMouseMove(x, y); chkDisabled_.OnMouseMove(x, y);
    radioA_.OnMouseMove(x, y); radioB_.OnMouseMove(x, y); radioC_.OnMouseMove(x, y);
    togOn_.OnMouseMove(x, y); togOff_.OnMouseMove(x, y); togDisabled_.OnMouseMove(x, y);
}
void SelectionPage::OnMouseDown(float x, float y) {
    chkChecked_.OnMouseDown(x, y); chkUnchecked_.OnMouseDown(x, y); chkDisabled_.OnMouseDown(x, y);
    radioA_.OnMouseDown(x, y); radioB_.OnMouseDown(x, y); radioC_.OnMouseDown(x, y);
    togOn_.OnMouseDown(x, y); togOff_.OnMouseDown(x, y); togDisabled_.OnMouseDown(x, y);
}
void SelectionPage::OnMouseUp(float x, float y) {
    chkChecked_.OnMouseUp(x, y); chkUnchecked_.OnMouseUp(x, y); chkDisabled_.OnMouseUp(x, y);
    radioA_.OnMouseUp(x, y); radioB_.OnMouseUp(x, y); radioC_.OnMouseUp(x, y);
    togOn_.OnMouseUp(x, y); togOff_.OnMouseUp(x, y); togDisabled_.OnMouseUp(x, y);
}
void SelectionPage::OnMouseLeave() {
    chkChecked_.OnMouseLeave(); chkUnchecked_.OnMouseLeave(); chkDisabled_.OnMouseLeave();
    radioA_.OnMouseLeave(); radioB_.OnMouseLeave(); radioC_.OnMouseLeave();
    togOn_.OnMouseLeave(); togOff_.OnMouseLeave(); togDisabled_.OnMouseLeave();
}

} // namespace ModernDesign::Demo