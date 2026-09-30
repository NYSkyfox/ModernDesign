#pragma once

#include <windows.h>
#include <string>

namespace ModernDesign {

// 日志：输出到调试器 + 可选文件
void LogInfo(const std::wstring& msg);
void LogWarn(const std::wstring& msg);
void LogError(const std::wstring& msg, HRESULT hr = S_OK);

} // namespace ModernDesign