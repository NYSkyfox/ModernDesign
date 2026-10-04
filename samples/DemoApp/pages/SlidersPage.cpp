// SlidersPage — Slider + ProgressBar + ProgressRing
#include "SlidersPage.h"
#include "DemoApp.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace ModernDesign::Demo {

void SlidersPage::Bind() {
    slider_.SetProgress(0.5f);
    progBar_.SetProgress(0.65f);
}

void SlidersPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();

    owner_->DrawText( L"Slider", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;
    slider_.SetBounds(RectF(x, y, w, kRowHeight * s));
    y += (kRowHeight + kGroupGap) * s;

    owner_->DrawText( L"ProgressBar", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;
    progBar_.SetBounds(RectF(x, y, w, 12.0f * s));
    y += 12.0f * s + kGroupGap * s;

    owner_->DrawText( L"ProgressRing", x, y, w, 24.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
             owner_->GetTheme().TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 32.0f * s;
    float ringSize = 32.0f * s;
    progRing_.SetBounds(RectF(x, y, ringSize, ringSize));
}

bool SlidersPage::Update(float dt) {
    bool anim = false;
    anim |= slider_.Update(dt);
    anim |= progBar_.Update(dt);
    anim |= progRing_.Update(dt);
    return anim;
}

void SlidersPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    slider_.Draw(r, t, s);
    progBar_.Draw(r, t, s);
    progRing_.Draw(r, t, s);
}

void SlidersPage::OnMouseMove(float x, float y) { slider_.OnMouseMove(x, y); }
void SlidersPage::OnMouseDown(float x, float y) { slider_.OnMouseDown(x, y); }
void SlidersPage::OnMouseUp(float x, float y) { slider_.OnMouseUp(x, y); }
void SlidersPage::OnMouseLeave() { slider_.OnMouseLeave(); }

} // namespace ModernDesign::Demo