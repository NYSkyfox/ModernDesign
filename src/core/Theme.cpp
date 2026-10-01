#include "pch.h"
#include "core/Theme.h"

namespace ModernDesign {

void Theme::SetLightMode(bool light) {
    lightMode = light;
    if (light) {
        // 恢复浅色（重置为默认）
        windowBgR = windowBgG = windowBgB = 0.949f;
        contentBgR = contentBgG = contentBgB = 1.0f;
        cardBgR = cardBgG = cardBgB = 1.0f;
        cardBorderR = cardBorderG = cardBorderB = 0.0f;
        cardBorderA = 0.08f;
        textPrimaryR = textPrimaryG = textPrimaryB = 0.0f;
        textSecondaryR = 0.376f; textSecondaryG = 0.369f; textSecondaryB = 0.365f;
        textDisabledR = textDisabledG = textDisabledB = 0.0f;
        textDisabledA = 0.36f;
        navContentBgR = navContentBgG = navContentBgB = 0.976f;  // #F9F9F9
        textOnAccentR = textOnAccentG = textOnAccentB = 1.0f;
        // accent 不在此处重置：它由 SetAccent 独立管理（优先取 Windows 强调色）
        buttonFillR = buttonFillG = buttonFillB = 0.949f;
        buttonBorderR = buttonBorderG = buttonBorderB = 0.0f;
        buttonBorderA = 0.12f;
        buttonTextR = buttonTextG = buttonTextB = 0.0f;
        revealBrushR = revealBrushG = revealBrushB = 0.0f;
        revealBrushA = 0.06f;
    } else {
        ApplyDark();
    }
}

void Theme::SetAccent(float r, float g, float b) {
    accentR = r;
    accentG = g;
    accentB = b;
}

void Theme::ApplyDark() {
    windowBgR = windowBgG = windowBgB = 0.0f;          // #000000
    contentBgR = contentBgG = contentBgB = 0.078f;     // #141414
    cardBgR = cardBgG = cardBgB = 0.110f;             // #1C1C1C
    cardBorderR = cardBorderG = cardBorderB = 0.267f; // #444444
    cardBorderA = 0.4f;
    textPrimaryR = textPrimaryG = textPrimaryB = 1.0f;       // #FFFFFF
    textSecondaryR = 0.8f; textSecondaryG = 0.8f; textSecondaryB = 0.8f;
    textDisabledR = textDisabledG = textDisabledB = 1.0f;   // rgba(255,255,255,.36)
    textDisabledA = 0.36f;
    navContentBgR = navContentBgG = navContentBgB = 0.157f;  // #282828
    textOnAccentR = textOnAccentG = textOnAccentB = 1.0f;    // #FFFFFF
    // accent 保持不变
    buttonFillR = buttonFillG = buttonFillB = 0.110f;  // #1C1C1C
    buttonBorderR = buttonBorderG = buttonBorderB = 0.267f;
    buttonBorderA = 0.2f;
    buttonTextR = buttonTextG = buttonTextB = 1.0f;
    revealBrushR = revealBrushG = revealBrushB = 1.0f;
    revealBrushA = 0.08f;
}

ThemeManager& ThemeManager::Instance() {
    static ThemeManager inst;
    return inst;
}

} // namespace ModernDesign