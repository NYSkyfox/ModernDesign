# FindWAM.cmake
#
# 定位 Windows Animation Manager 库（animation.lib）。
# WAM 提供 UI 动画引擎（如控件状态切换的缓动动画），
# 比手写计时器更平滑、更省电。
#
# 注意：WAM 是 Windows 10 1607+ 的组件，低版本系统不可用。
# 项目应做优雅降级：WAM 不可用时回退到自实现的缓动函数。

if(NOT WIN32)
    message(FATAL_ERROR "FindWAM.cmake: WAM 仅支持 Windows 平台")
endif()

# animation.lib 是 WAM 的静态导入库
find_library(WAM_LIBRARIES animation)

if(WAM_LIBRARIES)
    set(WAM_FOUND TRUE)
    message(STATUS "Found WAM: ${WAM_LIBRARIES}")
else()
    message(WARNING "FindWAM.cmake: animation.lib 未找到（Win10 1607+ 可用），将回退到自实现缓动")
endif()

# ---- 导出目标 ----
if(WAM_FOUND AND NOT TARGET ModernDesign::WAM)
    add_library(ModernDesign::WAM INTERFACE IMPORTED)
    set_target_properties(ModernDesign::WAM PROPERTIES
        INTERFACE_LINK_LIBRARIES "${WAM_LIBRARIES}"
    )
endif()

mark_as_advanced(WAM_LIBRARIES)