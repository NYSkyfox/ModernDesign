// DialogPage — ContentDialog 触发器
#include "DialogPage.h"
#include "DemoApp.h"

using namespace ModernDesign;

namespace ModernDesign::Demo {

void DialogPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    owner_->dlgBtn_.SetBounds(RectF(x, y, 120.0f * s, kRowHeight * s));
}

bool DialogPage::Update(float dt) {
    return owner_->dlgBtn_.Update(dt);
}

void DialogPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    owner_->dlgBtn_.Draw(r, t, s);
}

void DialogPage::OnMouseMove(float x, float y) { owner_->dlgBtn_.OnMouseMove(x, y); }
void DialogPage::OnMouseDown(float x, float y) { owner_->dlgBtn_.OnMouseDown(x, y); }
void DialogPage::OnMouseUp(float x, float y) { owner_->dlgBtn_.OnMouseUp(x, y); }
void DialogPage::OnMouseLeave() { owner_->dlgBtn_.OnMouseLeave(); }

} // namespace ModernDesign::Demo