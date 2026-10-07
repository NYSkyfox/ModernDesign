#include "pch.h"
#include "core/Renderer.h"
#include "utils/FluentIcons.h"
#include "utils/FluentIconsData.h"

namespace ModernDesign {

namespace {

const FluentIconData::IconPath& PathOf(FluentIcon icon) {
    switch (icon) {
    case FluentIcon::Home:        return FluentIconData::kHome;
    case FluentIcon::Grid:        return FluentIconData::kGrid;
    case FluentIcon::Person:      return FluentIconData::kPerson;
    case FluentIcon::Settings:    return FluentIconData::kSettings;
    case FluentIcon::Navigation:  return FluentIconData::kNavigation;
    case FluentIcon::ChevronDown: return FluentIconData::kChevronDown;
    case FluentIcon::ChevronUp:   return FluentIconData::kChevronUp;
    case FluentIcon::ChevronUpDown: return FluentIconData::kChevronUpDown;
    case FluentIcon::ChevronLeft: return FluentIconData::kChevronDown;
    case FluentIcon::ChevronRight: return FluentIconData::kChevronRight;
    case FluentIcon::ArrowLeft: return FluentIconData::kArrowLeft;
    case FluentIcon::Checkmark:   return FluentIconData::kCheckmark;
    case FluentIcon::RadioButton: return FluentIconData::kRadioButton;
    case FluentIcon::Info:        return FluentIconData::kInfo;
    case FluentIcon::Star:        return FluentIconData::kStar;
    case FluentIcon::StarFilled:  return FluentIconData::kStarFilled;
    case FluentIcon::PersonCircle: return FluentIconData::kPersonCircle;
    case FluentIcon::Subtract:    return FluentIconData::kSubtract;
    case FluentIcon::Add:         return FluentIconData::kAdd;
    case FluentIcon::DismissCircle: return FluentIconData::kDismissCircle;
    case FluentIcon::ErrorCircle: return FluentIconData::kErrorCircle;
    case FluentIcon::CheckmarkCircle: return FluentIconData::kCheckmarkCircle;
    case FluentIcon::InfoCircle:  return FluentIconData::kInfoCircle;
    case FluentIcon::Alert:       return FluentIconData::kAlert;
    }
    return FluentIconData::kHome;
}

} // namespace

void DrawFluentIcon(Renderer& renderer, FluentIcon icon,
                    float x, float y, float size, const Color& color, float rotRad) {
    const FluentIconData::IconPath& p = PathOf(icon);
    renderer.FillSvgPath(p.d, p.viewBox, x, y, size, color, rotRad);
}

void DrawFluentIconCentered(Renderer& renderer, FluentIcon icon,
                            const RectF& box, float size, const Color& color,
                            float rotRad) {
    DrawFluentIcon(renderer, icon,
                   box.x + (box.w - size) * 0.5f,
                   box.y + (box.h - size) * 0.5f,
                   size, color, rotRad);
}

} // namespace ModernDesign