#include "pch.h"
#include "utils/Logger.h"

namespace ModernDesign {

static void Emit(const wchar_t* tag, const std::wstring& msg) {
    std::wstring line = std::wstring(L"[ModernDesign] ") + tag + L" " + msg;
    OutputDebugStringW(line.c_str());
    OutputDebugStringW(L"\n");
}

void LogInfo(const std::wstring& msg) { Emit(L"[INFO]", msg); }
void LogWarn(const std::wstring& msg) { Emit(L"[WARN]", msg); }

void LogError(const std::wstring& msg, HRESULT hr) {
    std::wstring full = msg;
    if (hr != S_OK) {
        wchar_t buf[16];
        swprintf_s(buf, L"%08X", static_cast<unsigned>(hr));
        full += L" (HRESULT=0x";
        full += buf;
        full += L")";
    }
    Emit(L"[ERROR]", full);
}

} // namespace ModernDesign