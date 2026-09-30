# FindWIC.cmake
#
# 定位 Windows Imaging Component 库（windowscodecs.lib）。
# WIC 用于加载 PNG/JPEG 等位图资源，常用于：
#   - Acrylic 噪点纹理生成
#   - 图标资源（PNG）解码
#   - 位图 brush 缓存
#
# 注意：WIC 是 Windows 8+ 的组件，Win7 不可用。

if(NOT WIN32)
    message(FATAL_ERROR "FindWIC.cmake: WIC 仅支持 Windows 平台")
endif()

# windowscodecs.lib 是 WIC 的静态导入库
find_library(WIC_LIBRARIES windowscodecs)

if(WIC_LIBRARIES)
    set(WIC_FOUND TRUE)
    message(STATUS "Found WIC: ${WIC_LIBRARIES}")
else()
    message(WARNING "FindWIC.cmake: windowscodecs.lib 未找到（Win8+ 可用）")
endif()

# ---- 导出目标 ----
if(WIC_FOUND AND NOT TARGET ModernDesign::WIC)
    add_library(ModernDesign::WIC INTERFACE IMPORTED)
    set_target_properties(ModernDesign::WIC PROPERTIES
        INTERFACE_LINK_LIBRARIES "${WIC_LIBRARIES}"
    )
endif()

mark_as_advanced(WIC_LIBRARIES)