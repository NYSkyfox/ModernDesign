#!/usr/bin/env python3
# ============================================================
# 生成 src/utils/FluentIconsData.h
#
# 数据来源：microsoft/fluentui-system-icons（MIT License）
#   https://github.com/microsoft/fluentui-system-icons
#
# 做法：从上游仓库指定 tag 拉取 SVG，抽取 <path d="..."> 原文
#       （不做任何几何改写，保持与官方 SVG 逐字节一致，
#         方便将来升级时直接 diff 官方文件）
#
# 用法：
#   python3 tools/gen_fluent_icons.py
# ============================================================
import os
import re
import sys
import urllib.parse
import urllib.request

# 固定上游 tag —— 升级图标集时改这里，然后重新运行本脚本
UPSTREAM_TAG = "1.1.343"
RAW = "https://raw.githubusercontent.com/microsoft/fluentui-system-icons/{tag}/assets/{path}.svg"

# (C++ 常量名, 上游资产路径, 文件标签)
ICONS = [
    ("Home",         "Home/SVG/ic_fluent_home_16_regular",            "fluent_home_16_regular"),
    ("Grid",         "Grid/SVG/ic_fluent_grid_16_regular",            "fluent_grid_16_regular"),
    ("Person",       "Person/SVG/ic_fluent_person_16_regular",        "fluent_person_16_regular"),
    ("Settings",     "Settings/SVG/ic_fluent_settings_16_regular",    "fluent_settings_16_regular"),
    ("Navigation",   "Navigation/SVG/ic_fluent_navigation_16_regular", "fluent_navigation_16_regular"),
    ("ChevronDown",  "Chevron Down/SVG/ic_fluent_chevron_down_12_regular", "fluent_chevron_down_12_regular"),
    ("ChevronUp",    "Chevron Up/SVG/ic_fluent_chevron_up_12_regular",     "fluent_chevron_up_12_regular"),
    ("ChevronUpDown", "Chevron Up Down/SVG/ic_fluent_chevron_up_down_16_regular", "fluent_chevron_up_down_16_regular"),
    ("Info",         "Info/SVG/ic_fluent_info_20_regular",            "fluent_info_20_regular"),
]

OUT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "..", "src", "utils", "FluentIconsData.h")

WRAP = 108  # 路径字符串每行最大字符数


def fetch(path: str) -> str:
    url = RAW.format(tag=UPSTREAM_TAG, path=urllib.parse.quote(path))
    with urllib.request.urlopen(url, timeout=30) as resp:
        return resp.read().decode("utf-8")


def wrap_literal(d: str, indent: str) -> str:
    """把路径字符串按 WRAP 折行，输出 C++ 相邻字符串字面量拼接。"""
    parts = []
    rest = d
    while len(rest) > WRAP:
        cut = rest.rfind(' ', 0, WRAP)
        if cut < WRAP // 2:
            cut = WRAP
        parts.append(rest[:cut])
        rest = rest[cut:]
    parts.append(rest)
    if len(parts) == 1:
        return f'{indent}"{parts[0]}"'
    lines = [f'{indent}"{parts[0]}"']
    for p in parts[1:-1]:
        lines.append(f'{indent}"{p}"')
    lines.append(f'{indent}"{parts[-1]}"')
    return "\n".join(lines)


def main() -> int:
    entries = []
    for name, path, label in ICONS:
        svg = fetch(path)
        vb = re.search(r'viewBox="0 0 ([\d.]+) ([\d.]+)"', svg)
        if not vb:
            print(f"!! {label}: 找不到 viewBox", file=sys.stderr)
            return 1
        w, h = float(vb.group(1)), float(vb.group(2))
        if abs(w - h) > 0.001:
            print(f"!! {label}: viewBox 非正方形 {w}x{h}", file=sys.stderr)
            return 1
        ds = re.findall(r'<path[^>]*?\sd="([^"]+)"', svg)
        if len(ds) != 1:
            print(f"!! {label}: 期望 1 个 path，实得 {len(ds)}", file=sys.stderr)
            return 1
        entries.append((name, path, w, ds[0]))
        print(f"ok  {label:34s} viewBox {w:g}  d={len(ds[0])} 字符")

    out = []
    out.append("// ============================================================")
    out.append("// ⚠ 本文件由 tools/gen_fluent_icons.py 自动生成，请勿手工修改。")
    out.append("//")
    out.append("// 数据来源：microsoft/fluentui-system-icons（MIT License）")
    out.append(f"//           https://github.com/microsoft/fluentui-system-icons  tag {UPSTREAM_TAG}")
    out.append("// 几何为官方 SVG 的 path d 原文，未做任何改写。")
    out.append("// ============================================================")
    out.append("#pragma once")
    out.append("")
    out.append("namespace ModernDesign {")
    out.append("namespace FluentIconData {")
    out.append("")
    out.append("struct IconPath {")
    out.append("    const char* d;      // SVG path 的 d 属性（原文）")
    out.append("    float       viewBox; // 源坐标系边长（12 / 16 / 20）")
    out.append("};")
    out.append("")
    for name, path, vb, d in entries:
        out.append(f"// {path}.svg  ({vb:g}x{vb:g})")
        out.append(f"inline constexpr IconPath k{name}{{")
        out.append(wrap_literal(d, "    "))
        out.append(f"    , {vb:g}f }};")
        out.append("")
    out.append("} // namespace FluentIconData")
    out.append("} // namespace ModernDesign")
    out.append("")

    with open(OUT_PATH, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))
    print(f"\n已写出 {os.path.normpath(OUT_PATH)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
