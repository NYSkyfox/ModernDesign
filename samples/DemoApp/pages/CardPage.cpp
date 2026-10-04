// CardPage — Card + SettingsCard
#include "CardPage.h"
#include "DemoApp.h"
#include "utils/FluentIcons.h"

using namespace ModernDesign;

namespace ModernDesign::Demo {

void CardPage::Bind() {
    simpleCard_.SetTitle(L"Simple card");
    simpleCard_.SetContent(L"A plain Card with title and content text.");
    setA_.SetHeader(L"Dark mode");
    setA_.SetDescription(L"Switch between light and dark");
    setA_.SetHeaderIcon(static_cast<int>(FluentIcon::Home));
    setA_.SetContentWidth(52.0f);
    togA_.SetText(L"");
    togA_.SetIsOn(true);
}

void CardPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();

    owner_->DrawText( L"Card", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    simpleCard_.SetBounds(RectF(x, y, w, 80.0f * s));
    y += 80.0f * s + 12.0f * s;

    owner_->DrawText( L"SettingsCard", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;

    setA_.SetBounds(RectF(x, y, w, 72.0f * s));
    togA_.SetBounds(setA_.ContentRect(s));
    y += 72.0f * s + 4.0f * s;

    setB_.SetHeader(L"Notifications");
    setB_.SetDescription(L"Send me weekly digest");
    setB_.SetHeaderIcon(static_cast<int>(FluentIcon::Grid));
    setB_.SetContentWidth(52.0f);
    setB_.SetBounds(RectF(x, y, w, 72.0f * s));
}

bool CardPage::Update(float dt) {
    bool anim = false;
    anim |= setA_.Update(dt); anim |= setB_.Update(dt);
    anim |= togA_.Update(dt);
    return anim;
}

void CardPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    simpleCard_.Draw(r, t, s);
    setA_.Draw(r, t, s); setB_.Draw(r, t, s);
    togA_.Draw(r, t, s);
}

void CardPage::OnMouseMove(float x, float y) {
    setA_.OnMouseMove(x, y); setB_.OnMouseMove(x, y);
    togA_.OnMouseMove(x, y);
}
void CardPage::OnMouseDown(float x, float y) {
    setA_.OnMouseDown(x, y); setB_.OnMouseDown(x, y);
    togA_.OnMouseDown(x, y);
}
void CardPage::OnMouseUp(float x, float y) {
    setA_.OnMouseUp(x, y); setB_.OnMouseUp(x, y);
    togA_.OnMouseUp(x, y);
}
void CardPage::OnMouseLeave() {
    setA_.OnMouseLeave(); setB_.OnMouseLeave();
    togA_.OnMouseLeave();
}

} // namespace ModernDesign::Demo