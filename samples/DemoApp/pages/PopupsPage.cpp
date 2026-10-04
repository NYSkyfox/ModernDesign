// PopupsPage — Flyout + MenuFlyout + ToolTip 触发器
#include "PopupsPage.h"
#include "DemoApp.h"

using namespace ModernDesign;

namespace ModernDesign::Demo {

void PopupsPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();

    owner_->flyBtn_.SetBounds(RectF(x, y, 118.0f * s, kRowHeight * s));
    owner_->menuBtn_.SetBounds(RectF(x + 130.0f * s, y, 118.0f * s, kRowHeight * s));
    owner_->tipHost_.SetBounds(RectF(x + 260.0f * s, y, 150.0f * s, kRowHeight * s));
}

bool PopupsPage::Update(float dt) {
    bool anim = false;
    anim |= owner_->flyBtn_.Update(dt);
    anim |= owner_->menuBtn_.Update(dt);
    anim |= owner_->tipHost_.Update(dt);
    return anim;
}

void PopupsPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    owner_->flyBtn_.Draw(r, t, s);
    owner_->menuBtn_.Draw(r, t, s);
    owner_->tipHost_.Draw(r, t, s);
}

void PopupsPage::OnMouseMove(float x, float y) {
    owner_->flyBtn_.OnMouseMove(x, y);
    owner_->menuBtn_.OnMouseMove(x, y);
    owner_->tipHost_.OnMouseMove(x, y);
}
void PopupsPage::OnMouseDown(float x, float y) {
    owner_->flyBtn_.OnMouseDown(x, y);
    owner_->menuBtn_.OnMouseDown(x, y);
    owner_->tipHost_.OnMouseDown(x, y);
}
void PopupsPage::OnMouseUp(float x, float y) {
    owner_->flyBtn_.OnMouseUp(x, y);
    owner_->menuBtn_.OnMouseUp(x, y);
    owner_->tipHost_.OnMouseUp(x, y);
}
void PopupsPage::OnMouseLeave() {
    owner_->flyBtn_.OnMouseLeave();
    owner_->menuBtn_.OnMouseLeave();
    owner_->tipHost_.OnMouseLeave();
}

} // namespace ModernDesign::Demo