# FindDirectWrite.cmake
#
# 定位 Windows SDK 中的 DirectWrite 库（dwrite.lib）。
# DirectWrite 提供字体排版、文本布局、字形度量等能力，
# 是 Fluent Design 中文本渲染（对齐、裁剪、字体回退）的底层依赖。

if(NOT WIN32)
    message(FATAL_ERROR "FindDirectWrite.cmake: DirectWrite 仅支持 Windows 平台")
endif()

# ---- DirectWrite 1.1（最低 Win8）----
# dwrite.lib 是 DirectWrite 1.1 的入口，向下兼容 1.0 API
find_library(DWRITE_LIBRARIES dwrite)

if(DWRITE_LIBRARIES)
    set(DWRITE_FOUND TRUE)
    message(STATUS "Found DirectWrite: ${DWRITE_LIBRARIES}")
else()
    message(WARNING "FindDirectWrite.cmake: dwrite.lib 未找到，请确认已安装 Windows SDK 10.0.17763.0+")
endif()

# ---- 导出目标，方便下游统一链接 ----
if(DWRITE_FOUND AND NOT TARGET ModernDesign::DirectWrite)
    add_library(ModernDesign::DirectWrite INTERFACE IMPORTED)
    set_target_properties(ModernDesign::DirectWrite PROPERTIES
        INTERFACE_LINK_LIBRARIES "${DWRITE_LIBRARIES}"
    )
endif()

mark_as_advanced(DWRITE_LIBRARIES)