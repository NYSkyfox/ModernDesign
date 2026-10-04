// HomePage — 框架概览
#include "HomePage.h"
#include "DemoApp.h"
#include "utils/FluentIcons.h"
#include <shellapi.h>

using namespace ModernDesign;

namespace ModernDesign::Demo {

void HomePage::Bind() {
    cardInput_.SetHeader(L"Input");
    cardInput_.SetDescription(L"Buttons, Selection, Sliders");
    cardInput_.SetHeaderIcon(static_cast<int>(FluentIcon::Checkmark));
    cardInput_.SetClickable(true);
    cardInput_.SetActionIconVisible(true);

    cardContainer_.SetHeader(L"Container");
    cardContainer_.SetDescription(L"Expander, Card, Popups");
    cardContainer_.SetHeaderIcon(static_cast<int>(FluentIcon::Grid));
    cardContainer_.SetClickable(true);
    cardContainer_.SetActionIconVisible(true);

    cardPopup_.SetHeader(L"Dialog");
    cardPopup_.SetDescription(L"ContentDialog");
    cardPopup_.SetHeaderIcon(static_cast<int>(FluentIcon::Info));
    cardPopup_.SetClickable(true);
    cardPopup_.SetActionIconVisible(true);

    linkRepo_.SetText(L"github.com/NYSkyfox/ModernDesign");
    linkRepo_.SetNavigateUri(L"https://github.com/NYSkyfox/ModernDesign");
    linkRepo_.SetClickCallback([] {
        ShellExecuteW(nullptr, L"open", L"https://github.com/NYSkyfox/ModernDesign",
                      nullptr, nullptr, SW_SHOWNORMAL);
    });
}

void HomePage::Layout() {
    float s = owner_->DpiScale();
    float x = owner_->ContX(), y = owner_->PageTop();
    float w = owner_->ContW();
    const float cardH = 72.0f, cardGap = 4.0f;

    // 简介文字
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    owner_->DrawText(L"Pure C++ / Win32 / Direct2D", x, y, w, 20.0f * s,
             L"Segoe UI", 12.0f * s, DWRITE_FONT_WEIGHT_NORMAL, t.TextSecondary(),
             DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    y += 28.0f * s;

    // 三个入口卡片
    cardInput_.SetBounds(RectF(x, y, w, cardH * s));
    y += (cardH + cardGap) * s;
    cardContainer_.SetBounds(RectF(x, y, w, cardH * s));
    y += (cardH + cardGap) * s;
    cardPopup_.SetBounds(RectF(x, y, w, cardH * s));
    y += (cardH + cardGap) * s + 12.0f * s;

    // 仓库链接
    linkRepo_.SetBounds(RectF(x, y, linkRepo_.MeasureWidth(*owner_, s), kRowHeight * s));
}

bool HomePage::Update(float dt) {
    bool anim = false;
    anim |= cardInput_.Update(dt); anim |= cardContainer_.Update(dt); anim |= cardPopup_.Update(dt);
    anim |= linkRepo_.Update(dt);
    return anim;
}

void HomePage::Draw() {
    Renderer& r = *owner_;
    const Theme& t = owner_->GetTheme();
    float s = owner_->DpiScale();
    cardInput_.Draw(r, t, s); cardContainer_.Draw(r, t, s); cardPopup_.Draw(r, t, s);
    linkRepo_.Draw(r, t, s);
}

void HomePage::OnMouseMove(float x, float y) {
    cardInput_.OnMouseMove(x, y); cardContainer_.OnMouseMove(x, y); cardPopup_.OnMouseMove(x, y);
    linkRepo_.OnMouseMove(x, y);
}
void HomePage::OnMouseDown(float x, float y) {
    cardInput_.OnMouseDown(x, y); cardContainer_.OnMouseDown(x, y); cardPopup_.OnMouseDown(x, y);
    linkRepo_.OnMouseDown(x, y);
}
void HomePage::OnMouseUp(float x, float y) {
    cardInput_.OnMouseUp(x, y); cardContainer_.OnMouseUp(x, y); cardPopup_.OnMouseUp(x, y);
    linkRepo_.OnMouseUp(x, y);
}
void HomePage::OnMouseLeave() {
    cardInput_.OnMouseLeave(); cardContainer_.OnMouseLeave(); cardPopup_.OnMouseLeave();
    linkRepo_.OnMouseLeave();
}

} // namespace ModernDesign::Demo