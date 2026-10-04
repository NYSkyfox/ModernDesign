// ButtonsPage — Button + HyperlinkButton
#include "ButtonsPage.h"
#include "DemoApp.h"
#include <shellapi.h>

using namespace ModernDesign;

namespace ModernDesign::Demo {

void ButtonsPage::Bind() {
    btnStd_.SetText(L"Standard");
    btnStd_.SetVariant(ButtonVariant::Standard);
    btnAccent_.SetText(L"Accent");
    btnAccent_.SetVariant(ButtonVariant::Accent);
    btnDisabled_.SetText(L"Disabled");
    btnDisabled_.SetVariant(ButtonVariant::Standard);
    btnDisabled_.SetEnabled(false);

    linkA_.SetText(L"ModernDesign on GitHub");
    linkA_.SetNavigateUri(L"https://github.com/NYSkyfox/ModernDesign");
    linkA_.SetClickCallback([] {
        ShellExecuteW(nullptr, L"open", L"https://github.com/NYSkyfox/ModernDesign",
                      nullptr, nullptr, SW_SHOWNORMAL);
    });
    linkB_.SetText(L"Unavailable link");
    linkB_.SetEnabled(false);
}

void ButtonsPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();

    owner_->DrawText( L"Button", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    btnStd_.SetBounds(RectF(x, y, 96.0f * s, kRowHeight * s));
    btnAccent_.SetBounds(RectF(x + 108.0f * s, y, 96.0f * s, kRowHeight * s));
    btnDisabled_.SetBounds(RectF(x + 216.0f * s, y, 96.0f * s, kRowHeight * s));
    y += (kRowHeight + kGroupGap) * s;

    owner_->DrawText( L"HyperlinkButton", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    linkA_.SetBounds(RectF(x, y, linkA_.MeasureWidth(*owner_, s), kRowHeight * s));
    linkB_.SetBounds(RectF(x + 200.0f * s, y, linkB_.MeasureWidth(*owner_, s), kRowHeight * s));
}

bool ButtonsPage::Update(float dt) {
    bool anim = false;
    anim |= btnStd_.Update(dt); anim |= btnAccent_.Update(dt); anim |= btnDisabled_.Update(dt);
    anim |= linkA_.Update(dt); anim |= linkB_.Update(dt);
    return anim;
}

void ButtonsPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    btnStd_.Draw(r, t, s); btnAccent_.Draw(r, t, s); btnDisabled_.Draw(r, t, s);
    linkA_.Draw(r, t, s); linkB_.Draw(r, t, s);
}

void ButtonsPage::OnMouseMove(float x, float y) {
    btnStd_.OnMouseMove(x, y); btnAccent_.OnMouseMove(x, y); btnDisabled_.OnMouseMove(x, y);
    linkA_.OnMouseMove(x, y); linkB_.OnMouseMove(x, y);
}
void ButtonsPage::OnMouseDown(float x, float y) {
    btnStd_.OnMouseDown(x, y); btnAccent_.OnMouseDown(x, y); btnDisabled_.OnMouseDown(x, y);
    linkA_.OnMouseDown(x, y); linkB_.OnMouseDown(x, y);
}
void ButtonsPage::OnMouseUp(float x, float y) {
    btnStd_.OnMouseUp(x, y); btnAccent_.OnMouseUp(x, y); btnDisabled_.OnMouseUp(x, y);
    linkA_.OnMouseUp(x, y); linkB_.OnMouseUp(x, y);
}
void ButtonsPage::OnMouseLeave() {
    btnStd_.OnMouseLeave(); btnAccent_.OnMouseLeave(); btnDisabled_.OnMouseLeave();
    linkA_.OnMouseLeave(); linkB_.OnMouseLeave();
}

} // namespace ModernDesign::Demo