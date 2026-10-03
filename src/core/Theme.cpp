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
        // ContentDialog（浅色）
        dialogBgR = dialogBgG = dialogBgB = 0.953f;          // #F3F3F3
        dialogContentBgR = dialogContentBgG = dialogContentBgB = 1.0f;  // #FFFFFF
        dialogContentBgA = 1.0f;
        dialogCmdBgR = dialogCmdBgG = dialogCmdBgB = 0.953f; // #F3F3F3
        dialogBorderR = dialogBorderG = dialogBorderB = 0.459f; // rgba(117,117,117,.4)
        dialogBorderA = 0.4f;
        // Flyout / MenuFlyout / ToolTip（浅色）
        flyoutBgR = flyoutBgG = flyoutBgB = 0.988f;   // rgba(252,252,252,.92)
        flyoutBgA = 0.92f;
        flyoutBorderR = flyoutBorderG = flyoutBorderB = 0.0f;  // rgba(0,0,0,.06)
        flyoutBorderA = 0.06f;
        tipBgA = 0.7176f;                              // 0.92 × 78%
        dividerR = dividerG = dividerB = 0.0f;         // rgba(0,0,0,.06)
        dividerA = 0.06f;
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
    windowBgR = windowBgG = windowBgB = 0.125f;          // #202020 (WinUI 3 dark Mica base)
    contentBgR = contentBgG = contentBgB = 0.078f;     // #141414
    cardBgR = cardBgG = cardBgB = 0.110f;             // #1C1C1C
    cardBorderR = cardBorderG = cardBorderB = 0.267f; // #444444
    cardBorderA = 0.4f;
    textPrimaryR = textPrimaryG = textPrimaryB = 1.0f;       // #FFFFFF
    textSecondaryR = 0.8f; textSecondaryG = 0.8f; textSecondaryB = 0.8f;
    textDisabledR = textDisabledG = textDisabledB = 1.0f;   // rgba(255,255,255,.36)
    textDisabledA = 0.36f;
    // ContentDialog（深色）
    dialogBgR = dialogBgG = dialogBgB = 0.1725f;           // #2C2C2C
    dialogContentBgR = dialogContentBgG = dialogContentBgB = 0.169f; // rgba(43,43,43,0)
    dialogContentBgA = 0.0f;
    dialogCmdBgR = dialogCmdBgG = dialogCmdBgB = 0.1255f;  // #202020
    dialogBorderR = dialogBorderG = dialogBorderB = 0.459f; // rgba(117,117,117,.4)
    dialogBorderA = 0.4f;
    // Flyout / MenuFlyout / ToolTip（深色）
    flyoutBgR = flyoutBgG = flyoutBgB = 0.1725f;  // rgba(44,44,44,.86)
    flyoutBgA = 0.86f;
    flyoutBorderR = flyoutBorderG = flyoutBorderB = 1.0f;  // rgba(255,255,255,.10)
    flyoutBorderA = 0.10f;
    tipBgA = 0.6708f;                              // 0.86 × 78%
    dividerR = dividerG = dividerB = 1.0f;         // rgba(255,255,255,.08)
    dividerA = 0.08f;
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