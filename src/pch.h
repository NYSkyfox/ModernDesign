#pragma once

// ============================================================
// ModernDesign — 预编译头
// 目标：Windows 10 1809+ (build 17763) / Windows 11
// 依赖：仅 Windows 自带 DLL（d2d1 / dwrite / dwmapi / dxgi / d3d11 / windowscodecs）
// ============================================================

// ---- 强制 Unicode（宽字符 API）----
// 必须在所有 Win32 头之前定义，否则 Win32 API 默认走 ANSI，
// 导致 DrawText / CreateWindowEx 等映射到 *A 版本，与 wchar_t* 不匹配。
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Win10 1809（Acrylic / Mica 所需下限）
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif

// ---- Win32 ----
#include <windows.h>
#include <windowsx.h>  // GET_X_LPARAM / GET_Y_LPARAM

// ---- COM ----
#include <objbase.h>

// ---- Direct2D / DirectWrite ----
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <dxgi1_2.h>

// ---- DWM / 缩放 ----
#include <dwmapi.h>
#include <shellscalingapi.h>

// ---- COM 智能指针 ----
#include <wrl/client.h>

// ---- 标准库 ----
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// ---- 项目内部（顺序有依赖：Color.h 提供数学便捷函数）----
#include "utils/ComPtr.h"
#include "utils/Color.h"
#include "utils/Logger.h"
#include "utils/StringUtils.h"