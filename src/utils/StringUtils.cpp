#include "pch.h"
#include "utils/StringUtils.h"
#include <cstdarg>
#include <vector>

namespace ModernDesign {

std::wstring ANSIToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(n - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, out.data(), n);
    return out;
}

std::string WideToANSI(const std::wstring& s) {
    if (s.empty()) return "";
    int n = WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(n - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, out.data(), n, nullptr, nullptr);
    return out;
}

std::wstring UTF8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), n);
    return out;
}

std::wstring Format(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = _vscwprintf(fmt, args);
    va_end(args);
    if (n <= 0) return L"";

    std::vector<wchar_t> buf(static_cast<size_t>(n) + 1);
    va_start(args, fmt);
    vswprintf_s(buf.data(), buf.size(), fmt, args);
    va_end(args);
    return std::wstring(buf.data(), static_cast<size_t>(n));
}

} // namespace ModernDesign