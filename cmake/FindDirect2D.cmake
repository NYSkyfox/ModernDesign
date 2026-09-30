# FindDirect2D.cmake
#
# 定位 Windows SDK 中的 Direct2D 库（d2d1.lib）。
# Direct2D 是 Windows SDK 的一部分，随 SDK 一起提供，无需额外下载。

include(CheckCXXCompilerFlag)

# 仅在 Windows 平台生效
if(NOT WIN32)
    message(FATAL_ERROR "FindDirect2D.cmake: Direct2D 仅支持 Windows 平台")
endif()

# ---- Direct2D 1.1（最低 Win8）----
# d2d1.lib 是 Direct2D 1.1 的入口，向下兼容 1.0 API
find_library(D2D1_LIBRARIES d2d1)

if(D2D1_LIBRARIES)
    set(D2D1_FOUND TRUE)
    message(STATUS "Found Direct2D: ${D2D1_LIBRARIES}")
else()
    message(WARNING "FindDirect2D.cmake: d2D1.lib 未找到，请确认已安装 Windows SDK 10.0.17763.0+")
endif()

# ---- 导出目标，方便下游统一链接 ----
if(D2D1_FOUND AND NOT TARGET ModernDesign::Direct2D)
    add_library(ModernDesign::Direct2D INTERFACE IMPORTED)
    set_target_properties(ModernDesign::Direct2D PROPERTIES
        INTERFACE_LINK_LIBRARIES "${D2D1_LIBRARIES}"
    )
endif()

mark_as_advanced(D2D1_LIBRARIES)