// HomePage — 基础控件合集
#include "HomePage.h"
#include "DemoApp.h"
#include <shellapi.h>

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace ModernDesign::Demo {

void HomePage::Bind() {
    homeBtnStd_.SetText(L"Standard");
    homeBtnStd_.SetVariant(ButtonVariant::Standard);
    homeBtnAcc_.SetText(L"Accent");
    homeBtnAcc_.SetVariant(ButtonVariant::Accent);

    homeChk_.SetText(L"CheckBox checked");
    homeChk_.SetChecked(true);
    homeTog_.SetText(L"Toggle on");
    homeTog_.SetIsOn(true);

    homeRadioA_.SetText(L"RadioButton unchecked");
    homeRadioB_.SetText(L"RadioButton checked");
    homeRadioB_.SetSelected(true);

    homeProg_.SetProgress(0.6f);

    homeLinkA_.SetText(L"Learn more about Modern Design");
    homeLinkA_.SetNavigateUri(L"https://github.com/NYSkyfox/ModernDesign");
    homeLinkA_.SetClickCallback([] {
        ShellExecuteW(nullptr, L"open", L"https://github.com/NYSkyfox/ModernDesign",
                      nullptr, nullptr, SW_SHOWNORMAL);
    });
    homeLinkB_.SetText(L"Unavailable link");
    homeLinkB_.SetEnabled(false);
}

void HomePage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();

    homeBtnStd_.SetBounds(RectF(x, y, 96.0f * s, kRowHeight * s));
    homeBtnAcc_.SetBounds(RectF(x + 108.0f * s, y, 96.0f * s, kRowHeight * s));
    owner_->dlgBtn_.SetBounds(RectF(x + 216.0f * s, y, 120.0f * s, kRowHeight * s));
    y += kRowHeight * s + kGroupGap * s;

    homeChk_.SetBounds(RectF(x, y, owner_->ContW(), kRowHeight * s));
    y += kRowHeight * s + kRowGap * s;

    homeTog_.SetBounds(RectF(x, y, owner_->ContW(), kRowHeight * s));
    y += kRowHeight * s + kGroupGap * s;

    homeRadioA_.SetBounds(RectF(x, y, owner_->ContW(), kRowHeight * s));
    y += kRowHeight * s + kRowGap * s;
    homeRadioB_.SetBounds(RectF(x, y, owner_->ContW(), kRowHeight * s));
    y += kRowHeight * s + kGroupGap * s;

    homeSlider_.SetBounds(RectF(x, y, owner_->ContW(), kRowHeight * s));
    y += kRowHeight * s + kGroupGap * s;

    homeProg_.SetBounds(RectF(x, y, owner_->ContW(), 12.0f * s));
    y += 12.0f * s + kGroupGap * s;

    homeLinkA_.SetBounds(RectF(x, y, homeLinkA_.MeasureWidth(*owner_, s), kRowHeight * s));
    homeLinkB_.SetBounds(RectF(x + 28.0f * s, y, homeLinkB_.MeasureWidth(*owner_, s), kRowHeight * s));
    y += kRowHeight * s + kGroupGap * s;

    // 浮出层触发按钮（Flyout / MenuFlyout / ToolTip）
    owner_->flyBtn_.SetBounds(RectF(x, y, 118.0f * s, kRowHeight * s));
    owner_->menuBtn_.SetBounds(RectF(x + 130.0f * s, y, 118.0f * s, kRowHeight * s));
    owner_->tipHost_.SetBounds(RectF(x + 260.0f * s, y, 150.0f * s, kRowHeight * s));
}

bool HomePage::Update(float dt) {
    bool anim = false;
    anim |= homeBtnStd_.Update(dt); anim |= homeBtnAcc_.Update(dt);
    anim |= homeChk_.Update(dt); anim |= homeTog_.Update(dt);
    anim |= homeRadioA_.Update(dt); anim |= homeRadioB_.Update(dt);
    anim |= homeSlider_.Update(dt);
    anim |= homeLinkA_.Update(dt); anim |= homeLinkB_.Update(dt);
    anim |= owner_->dlgBtn_.Update(dt);
    anim |= owner_->flyBtn_.Update(dt); anim |= owner_->menuBtn_.Update(dt);
    anim |= owner_->tipHost_.Update(dt);
    return anim;
}

void HomePage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    homeBtnStd_.Draw(r, t, s); homeBtnAcc_.Draw(r, t, s);
    owner_->dlgBtn_.Draw(r, t, s);
    homeChk_.Draw(r, t, s); homeTog_.Draw(r, t, s);
    homeRadioA_.Draw(r, t, s); homeRadioB_.Draw(r, t, s);
    homeSlider_.Draw(r, t, s); homeProg_.Draw(r, t, s);
    homeLinkA_.Draw(r, t, s); homeLinkB_.Draw(r, t, s);
    owner_->flyBtn_.Draw(r, t, s); owner_->menuBtn_.Draw(r, t, s);
    owner_->tipHost_.Draw(r, t, s);
}

void HomePage::OnMouseMove(float x, float y) {
    homeBtnStd_.OnMouseMove(x, y); homeBtnAcc_.OnMouseMove(x, y);
    homeChk_.OnMouseMove(x, y); homeTog_.OnMouseMove(x, y);
    homeRadioA_.OnMouseMove(x, y); homeRadioB_.OnMouseMove(x, y);
    homeSlider_.OnMouseMove(x, y);
    homeLinkA_.OnMouseMove(x, y); homeLinkB_.OnMouseMove(x, y);
    owner_->flyBtn_.OnMouseMove(x, y); owner_->menuBtn_.OnMouseMove(x, y);
    owner_->tipHost_.OnMouseMove(x, y);
}
void HomePage::OnMouseDown(float x, float y) {
    homeBtnStd_.OnMouseDown(x, y); homeBtnAcc_.OnMouseDown(x, y);
    homeChk_.OnMouseDown(x, y); homeTog_.OnMouseDown(x, y);
    homeRadioA_.OnMouseDown(x, y); homeRadioB_.OnMouseDown(x, y);
    homeSlider_.OnMouseDown(x, y);
    homeLinkA_.OnMouseDown(x, y); homeLinkB_.OnMouseDown(x, y);
    owner_->flyBtn_.OnMouseDown(x, y); owner_->menuBtn_.OnMouseDown(x, y); owner_->tipHost_.OnMouseDown(x, y);
}
void HomePage::OnMouseUp(float x, float y) {
    homeBtnStd_.OnMouseUp(x, y); homeBtnAcc_.OnMouseUp(x, y);
    homeChk_.OnMouseUp(x, y); homeTog_.OnMouseUp(x, y);
    homeRadioA_.OnMouseUp(x, y); homeRadioB_.OnMouseUp(x, y);
    homeSlider_.OnMouseUp(x, y);
    homeLinkA_.OnMouseUp(x, y); homeLinkB_.OnMouseUp(x, y);
    owner_->flyBtn_.OnMouseUp(x, y); owner_->menuBtn_.OnMouseUp(x, y); owner_->tipHost_.OnMouseUp(x, y);
}
void HomePage::OnMouseLeave() {
    homeBtnStd_.OnMouseLeave(); homeBtnAcc_.OnMouseLeave();
    homeChk_.OnMouseLeave(); homeTog_.OnMouseLeave();
    homeRadioA_.OnMouseLeave(); homeRadioB_.OnMouseLeave();
    homeSlider_.OnMouseLeave();
    homeLinkA_.OnMouseLeave(); homeLinkB_.OnMouseLeave();
    owner_->flyBtn_.OnMouseLeave(); owner_->menuBtn_.OnMouseLeave(); owner_->tipHost_.OnMouseLeave();
}

} // namespace ModernDesign::Demo