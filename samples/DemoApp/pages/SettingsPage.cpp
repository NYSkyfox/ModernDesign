// SettingsPage — 设置卡片示例
#include "SettingsPage.h"
#include "DemoApp.h"
#include "utils/FluentIcons.h"

using namespace ModernDesign;
using namespace ModernDesign::Controls;

namespace ModernDesign::Demo {

void SettingsPage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    const float cardH = 70.0f;
    const float cardGap = 4.0f;
    const float togW = 52.0f;   // track 40 + pad 12

    setCardA_.SetHeader(L"Appearance");
    setCardA_.SetDescription(L"Choose the app color mode");
    setCardA_.SetHeaderIcon(static_cast<int>(FluentIcon::Home));
    setCardA_.SetContentWidth(togW);
    setCardA_.SetBounds(RectF(x, y, owner_->ContW(), cardH * s));
    setTogA_.SetText(L"");
    setTogA_.SetIsOn(!owner_->GetTheme().lightMode);
    setTogA_.SetBounds(setCardA_.ContentRect(s));
    y += cardH * s + cardGap * s;

    setCardB_.SetHeader(L"Navigation");
    setCardB_.SetDescription(L"Compact pane (48px rail)");
    setCardB_.SetHeaderIcon(static_cast<int>(FluentIcon::Navigation));
    setCardB_.SetContentWidth(togW);
    setCardB_.SetBounds(RectF(x, y, owner_->ContW(), cardH * s));
    setTogB_.SetText(L"");
    setTogB_.SetIsOn(owner_->nav_.IsCompact());
    setTogB_.SetBounds(setCardB_.ContentRect(s));
    y += cardH * s + cardGap * s;

    setCardC_.SetHeader(L"Layout");
    setCardC_.SetDescription(L"Top navigation bar");
    setCardC_.SetHeaderIcon(static_cast<int>(FluentIcon::Grid));
    setCardC_.SetContentWidth(togW);
    setCardC_.SetBounds(RectF(x, y, owner_->ContW(), cardH * s));
    setTogC_.SetText(L"");
    setTogC_.SetIsOn(owner_->nav_.GetDisplayMode() == NavigationView::DisplayMode::Top);
    setTogC_.SetBounds(setCardC_.ContentRect(s));
    y += cardH * s + cardGap * s;

    setCardD_.SetHeader(L"About");
    setCardD_.SetDescription(L"ModernDesign — Fluent framework");
    setCardD_.SetHeaderIcon(static_cast<int>(FluentIcon::Info));
    setCardD_.SetClickable(true);
    setCardD_.SetActionIconVisible(true);
    setCardD_.SetContent(L"v1.0");
    setCardD_.SetContentWidth(36.0f);
    setCardD_.SetBounds(RectF(x, y, owner_->ContW(), cardH * s));

    if (!setBound_) {
        setBound_ = true;
        setTogA_.SetChangedCallback([this](bool on) { if (on != !owner_->GetTheme().lightMode) owner_->ToggleThemePublic(); });
        setTogB_.SetChangedCallback([this](bool on) { owner_->nav_.SetCompact(on); owner_->Invalidate(); });
        setTogC_.SetChangedCallback([this](bool on) {
            owner_->nav_.SetDisplayMode(on ? NavigationView::DisplayMode::Top
                                           : NavigationView::DisplayMode::Left);
            owner_->Invalidate();
        });
    }
}

bool SettingsPage::Update(float dt) {
    bool anim = false;
    anim |= setTogA_.Update(dt); anim |= setTogB_.Update(dt); anim |= setTogC_.Update(dt);
    anim |= setCardA_.Update(dt); anim |= setCardB_.Update(dt);
    anim |= setCardC_.Update(dt); anim |= setCardD_.Update(dt);
    return anim;
}

void SettingsPage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    // 先画卡片（底层容器），再画开关（上层内容）
    setCardA_.Draw(r, t, s); setCardB_.Draw(r, t, s);
    setCardC_.Draw(r, t, s); setCardD_.Draw(r, t, s);
    setTogA_.Draw(r, t, s); setTogB_.Draw(r, t, s); setTogC_.Draw(r, t, s);
}

void SettingsPage::OnMouseMove(float x, float y) {
    setTogA_.OnMouseMove(x, y); setTogB_.OnMouseMove(x, y); setTogC_.OnMouseMove(x, y);
    setCardA_.OnMouseMove(x, y); setCardB_.OnMouseMove(x, y); setCardC_.OnMouseMove(x, y); setCardD_.OnMouseMove(x, y);
}
void SettingsPage::OnMouseDown(float x, float y) {
    setTogA_.OnMouseDown(x, y); setTogB_.OnMouseDown(x, y); setTogC_.OnMouseDown(x, y);
    setCardA_.OnMouseDown(x, y); setCardB_.OnMouseDown(x, y); setCardC_.OnMouseDown(x, y); setCardD_.OnMouseDown(x, y);
}
void SettingsPage::OnMouseUp(float x, float y) {
    setTogA_.OnMouseUp(x, y); setTogB_.OnMouseUp(x, y); setTogC_.OnMouseUp(x, y);
    setCardA_.OnMouseUp(x, y); setCardB_.OnMouseUp(x, y); setCardC_.OnMouseUp(x, y); setCardD_.OnMouseUp(x, y);
}
void SettingsPage::OnMouseLeave() {
    setTogA_.OnMouseLeave(); setTogB_.OnMouseLeave(); setTogC_.OnMouseLeave();
    setCardA_.OnMouseLeave(); setCardB_.OnMouseLeave(); setCardC_.OnMouseLeave(); setCardD_.OnMouseLeave();
}

void SettingsPage::OnThemeChanged() {
    setTogA_.SetIsOn(!owner_->GetTheme().lightMode);
}

} // namespace ModernDesign::Demo