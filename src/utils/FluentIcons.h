#pragma once

#include "Geometry.h"
#include "utils/Color.h"

namespace ModernDesign {

class Renderer;

// ============================================================
// FluentIcon — 官方 Fluent 图标（Fluent UI System Icons）
//
// 几何数据取自 microsoft/fluentui-system-icons @ 1.1.343（MIT License），
// 由 tools/gen_fluent_icons.py 抽取官方 SVG 的 path d 原文写入
// utils/FluentIconsData.h；本项目只负责缩放/旋转/填充，不改写路径。
//
// 与「系统字体图标」的区别：
//   WinUI 本身用 Segoe Fluent Icons 字体渲染 E7xx 码点，本项目不用字体，
//   而是直接把官方开源图标集的矢量路径喂给 Direct2D 路径几何 ——
//   零依赖、可随应用分发、跨 Windows 版本一致。
//   完整许可见仓库根目录 THIRD_PARTY_NOTICES.md。
// ============================================================
enum class FluentIcon {
    Home,          // 16px
    Grid,          // 16px
    Person,        // 16px
    Settings,      // 16px
    Navigation,    // 16px —— 汉堡（GlobalNavButton）
    ChevronDown,   // 12px
    ChevronUp,     // 12px
    ChevronUpDown, // 16px
    ChevronLeft,   // 12px（ChevronDown 旋转 -90°）
    Checkmark,     // 16px
    RadioButton,   // 16px（filled，菜单单选标记）
    Info,          // 20px
};

// 在 (x, y) 为左上角、边长 size 的方盒内绘制图标
// rotRad：绕图标盒中心的旋转（弧度），例如 chevron 展开动画用 pi
void DrawFluentIcon(Renderer& renderer, FluentIcon icon,
                    float x, float y, float size, const Color& color,
                    float rotRad = 0.0f);

// 在 box 内居中绘制（按图标盒居中，不做墨迹范围收紧）
void DrawFluentIconCentered(Renderer& renderer, FluentIcon icon,
                            const RectF& box, float size, const Color& color,
                            float rotRad = 0.0f);

} // namespace ModernDesign