#pragma once

#include <windows.h>
#include <string>

namespace ModernDesign {

// 字符串工具
std::wstring ANSIToWide(const std::string& s);
std::string WideToANSI(const std::wstring& s);
std::wstring UTF8ToWide(const std::string& s);

// 格式化（自动分配长度）
std::wstring Format(const wchar_t* fmt, ...);

} // namespace ModernDesign