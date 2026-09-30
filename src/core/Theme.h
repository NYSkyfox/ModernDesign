#pragma once

#include "Geometry.h"
#include <functional>
#include <string>
#include <vector>

namespace ModernDesign {

// ============================================================
// 设计令牌 (Theme Tokens)
// 所有控件的颜色/间距/圆角从这里获取
// ============================================================

struct Theme {
    // === 颜色（全部为 0~1 浮点，与 Color 兼容）===
    // 窗口背景
    float windowBgR = 0.949f;  // #F2F2F2
    float windowBgG = 0.949f;
    float windowBgB = 0.949f;
    // 内容背景
    float contentBgR = 1.0f;   // #FFFFFF
    float contentBgG = 1.0f;
    float contentBgB = 1.0f;
    // 卡片背景
    float cardBgR = 1.0f;      // #FFFFFF
    float cardBgG = 1.0f;
    float cardBgB = 1.0f;
    // 卡片边框
    float cardBorderR = 0.0f;  // #000000
    float cardBorderG = 0.0f;
    float cardBorderB = 0.0f;
    float cardBorderA = 0.08f;
    // 文本主色
    float textPrimaryR = 0.0f;  // #000000
    float textPrimaryG = 0.0f;
    float textPrimaryB = 0.0f;
    // 文本次色
    float textSecondaryR = 0.376f;  // #605E5C
    float textSecondaryG = 0.369f;
    float textSecondaryB = 0.365f;
    // 文字在强调色上
    float textOnAccentR = 1.0f;   // #FFFFFF
    float textOnAccentG = 1.0f;
    float textOnAccentB = 1.0f;
    // 强调色（默认墨绿 #013220；运行时优先取 Windows 强调色）
    float accentR = 0.00392f;     // #013220
    float accentG = 0.19608f;
    float accentB = 0.12549f;
    // 按钮填充
    float buttonFillR = 0.949f;   // #F2F2F2
    float buttonFillG = 0.949f;
    float buttonFillB = 0.949f;
    // 按钮边框
    float buttonBorderR = 0.0f;   // #000000
    float buttonBorderG = 0.0f;
    float buttonBorderB = 0.0f;
    float buttonBorderA = 0.12f;
    // 按钮文字
    float buttonTextR = 0.0f;     // #000000
    float buttonTextG = 0.0f;
    float buttonTextB = 0.0f;
    // Hover 光晕
    float revealBrushR = 0.0f;    // #000000
    float revealBrushG = 0.0f;
    float revealBrushB = 0.0f;
    float revealBrushA = 0.06f;

    // === 圆角 ===
    float cornerRadiusSmall = 2.0f;   // 按钮等小控件
    float cornerRadiusMedium = 4.0f;  // Card
    float cornerRadiusLarge = 8.0f;   // 容器

    // === 间距 ===
    float spacingXxs = 2.0f;
    float spacingXs = 4.0f;
    float spacingS = 8.0f;
    float spacingM = 16.0f;
    float spacingL = 24.0f;
    float spacingXl = 32.0f;

    // === 字号 ===
    float fontSizeCaption = 12.0f;
    float fontSizeBody = 14.0f;
    float fontSizeSubhead = 17.0f;
    float fontSizeTitle = 26.0f;
    float fontSizeDisplay = 40.0f;

    // === 阴影等级（elevation）===
    float elevation0 = 0.0f;
    float elevation1 = 2.0f;
    float elevation2 = 4.0f;
    float elevation3 = 8.0f;
    float elevation4 = 16.0f;

    // 当前主题模式（浅色/深色）
    bool lightMode = true;

    // 快捷获取 Color
    Color WindowBg() const { return Color(windowBgR, windowBgG, windowBgB, 1.0f); }
    Color ContentBg() const { return Color(contentBgR, contentBgG, contentBgB, 1.0f); }
    Color CardBg() const { return Color(cardBgR, cardBgG, cardBgB, 1.0f); }
    Color CardBorder() const { return Color(cardBorderR, cardBorderG, cardBorderB, cardBorderA); }
    Color TextPrimary() const { return Color(textPrimaryR, textPrimaryG, textPrimaryB, 1.0f); }
    Color TextSecondary() const { return Color(textSecondaryR, textSecondaryG, textSecondaryB, 1.0f); }
    Color TextOnAccent() const { return Color(textOnAccentR, textOnAccentG, textOnAccentB, 1.0f); }
    Color Accent() const { return Color(accentR, accentG, accentB, 1.0f); }
    Color ButtonFill() const { return Color(buttonFillR, buttonFillG, buttonFillB, 1.0f); }
    Color ButtonBorder() const { return Color(buttonBorderR, buttonBorderG, buttonBorderB, buttonBorderA); }
    Color ButtonText() const { return Color(buttonTextR, buttonTextG, buttonTextB, 1.0f); }
    Color RevealBrush() const { return Color(revealBrushR, revealBrushG, revealBrushB, revealBrushA); }

    // 切换到浅色/深色
    void SetLightMode(bool light);

    // 设置强调色（0~1，运行时从 Windows 注册表读取，失败取默认墨绿）
    void SetAccent(float r, float g, float b);

    // 深色模式默认值（覆盖上面字段）
    void ApplyDark();
};

// 主题事件回调
using ThemeChangedCallback = std::function<void()>;

// 全局主题（单例，简化初始样板代码）
class ThemeManager {
public:
    static ThemeManager& Instance();
    const Theme& Current() const { return theme_; }

    void SetLightMode(bool light) {
        theme_.SetLightMode(light);
        if (onChanged_) onChanged_();
    }

    void OnChanged(ThemeChangedCallback cb) { onChanged_ = cb; }
    ThemeChangedCallback onChanged_;

private:
    Theme theme_;
};

} // namespace ModernDesign