// ShowcasePage — B1 新增控件展示
#include "ShowcasePage.h"
#include "DemoApp.h"

using namespace ModernDesign;

namespace ModernDesign::Demo {

static const wchar_t* kSec = L"Segoe UI";

void ShowcasePage::Bind() {
    badgeValue_.SetValue(3);
    badgeCount_.SetValue(128);
    badgeCount_.SetBadgeMaxCount(99);
    badgeIcon_.SetIcon(FluentIcon::Alert);
    badgeIcon_.SetBackground(Color::Hex(0xFFB900));
    badgeIcon_.SetForeground(Color(0, 0, 0, 1.0f));
    badgeText_.SetText(L"NEW");

    pips_.SetPageIndexCount(5);
    pips_.SetSelectedIndex(1);

    rating_.SetMaximumValue(5);
    rating_.SetValue(3);
    ratingRo_.SetMaximumValue(5);
    ratingRo_.SetValue(4);
    ratingRo_.SetIsReadOnly(true);

    ppAvail_.SetBadge(PersonPicture::Badge::Available);
    ppBusy_.SetBadge(PersonPicture::Badge::Busy);
    ppAway_.SetBadge(PersonPicture::Badge::Away);
    ppOffline_.SetBadge(PersonPicture::Badge::Offline);
}

void ShowcasePage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();

    auto section = [&](const std::wstring& t) {
        owner_->DrawText(t, x, y, w, 24.0f * s, kSec, 12.0f * s,
                         DWRITE_FONT_WEIGHT_SEMI_BOLD,
                         owner_->GetTheme().TextSecondary(),
                         DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        y += 32.0f * s;
    };

    // ---- InfoBadge ----
    section(L"InfoBadge");
    float badgeH = 22.0f * s;
    {
        float bx = x;
        badgeValue_.SetBounds(RectF(bx, y, 34.0f * s, badgeH)); bx += 52.0f * s;
        badgeCount_.SetBounds(RectF(bx, y, 52.0f * s, badgeH)); bx += 72.0f * s;
        badgeIcon_.SetBounds(RectF(bx, y, badgeH, badgeH));     bx += 44.0f * s;
        badgeText_.SetBounds(RectF(bx, y, 56.0f * s, badgeH));
    }
    y += badgeH + kGroupGap * s;

    // ---- PipsPager ----
    section(L"PipsPager");
    pips_.SetBounds(RectF(x, y, w, 24.0f * s));
    y += 24.0f * s + kGroupGap * s;

    // ---- RatingControl ----
    section(L"RatingControl");
    rating_.SetBounds(RectF(x, y, w, 32.0f * s));
    y += 40.0f * s;
    owner_->DrawText(L"Read-only", x, y, w, 20.0f * s, kSec, 11.0f * s,
                     DWRITE_FONT_WEIGHT_NORMAL,
                     owner_->GetTheme().TextSecondary(),
                     DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 28.0f * s;
    ratingRo_.SetBounds(RectF(x, y, w, 32.0f * s));
    y += 32.0f * s + kGroupGap * s;

    // ---- PersonPicture ----
    section(L"PersonPicture");
    {
        float d = 40.0f * s;
        float gap = 20.0f * s;
        float px = x;
        ppAvail_.SetBounds(RectF(px, y, d, d));     px += d + gap;
        ppBusy_.SetBounds(RectF(px, y, d, d));      px += d + gap;
        ppAway_.SetBounds(RectF(px, y, d, d));      px += d + gap;
        ppOffline_.SetBounds(RectF(px, y, d, d));
    }
}

bool ShowcasePage::Update(float dt) {
    bool a = false;
    a |= badgeValue_.Update(dt);
    a |= pips_.Update(dt);
    a |= rating_.Update(dt);
    a |= ratingRo_.Update(dt);
    return a;
}

void ShowcasePage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    badgeValue_.Draw(r, t, s);
    badgeCount_.Draw(r, t, s);
    badgeIcon_.Draw(r, t, s);
    badgeText_.Draw(r, t, s);
    pips_.Draw(r, t, s);
    rating_.Draw(r, t, s);
    ratingRo_.Draw(r, t, s);
    ppAvail_.Draw(r, t, s);
    ppBusy_.Draw(r, t, s);
    ppAway_.Draw(r, t, s);
    ppOffline_.Draw(r, t, s);
}

void ShowcasePage::OnMouseMove(float x, float y) { rating_.OnMouseMove(x, y); }
void ShowcasePage::OnMouseUp(float x, float y) {
    rating_.OnMouseUp(x, y);
    pips_.OnMouseUp(x, y);
}
void ShowcasePage::OnMouseLeave() { rating_.OnMouseLeave(); }

} // namespace ModernDesign::Demo