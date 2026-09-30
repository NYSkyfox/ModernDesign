#pragma once

#include <windows.h>
#include <wrl/client.h>

namespace ModernDesign {

// ComPtr 别名（放顶层，全项目统一使用）
template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// 安全释放
template <typename T>
inline void SafeRelease(T** pp) {
    if (*pp) {
        (*pp)->Release();
        *pp = nullptr;
    }
}

} // namespace ModernDesign