cmake_minimum_required(VERSION 3.20)

project(ModernDesign
    VERSION 0.1.0
    DESCRIPTION "Fluent Design (WinUI 3 style) framework in pure C++"
    LANGUAGES CXX
)

# ============================================================
# 全局设置
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "ModernDesign 仅支持 Windows 平台")
endif()

# 输出目录：build/bin/<Config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
foreach(cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin/${cfg})
endforeach()

# 导出编译数据库（供 clangd / IDE 使用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# 依赖查找
# ============================================================
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CompilerWarnings)
setup_global_warnings()

include(FindDirect2D)
include(FindDirectWrite)
include(FindWIC)

# ============================================================
# 核心框架库（静态库，供 DemoApp 复用）
# ============================================================
set(MD_CORE_SOURCES
    src/core/Theme.cpp
    src/core/Renderer.cpp
)

set(MD_APP_SOURCES
    src/app/App.cpp
)

set(MD_CONTROL_SOURCES
    src/controls/Button.cpp
    src/controls/CheckBox.cpp
    src/controls/ToggleSwitch.cpp
    src/controls/RadioButton.cpp
    src/controls/ProgressBar.cpp
    src/controls/ProgressRing.cpp
    src/controls/Slider.cpp
    src/controls/Card.cpp
    src/controls/TextBlock.cpp
)

set(MD_UTIL_SOURCES
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
)

add_library(ModernDesign STATIC
    ${MD_UTIL_SOURCES}
    ${MD_CORE_SOURCES}
    ${MD_APP_SOURCES}
    ${MD_CONTROL_SOURCES}
)

# 预编译头：加速编译
target_precompile_headers(ModernDesign PRIVATE src/pch.h)

# 包含目录
target_include_directories(ModernDesign PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- 链接的系统库 ----
# 只用 Windows 自带 DLL，保持零第三方依赖
target_link_libraries(ModernDesign PUBLIC
    d2d1              # Direct2D 1.1
    dwrite            # DirectWrite（文本排版）
    dwmapi            # DWM（窗口属性、圆角、backdrop）
    dxgi              # DXGI（像素格式）
    dxguid            # DXGI GUID
    d3d11             # D3D11（HwndRenderTarget 依赖）
    windowscodecs     # WIC（图像解码）
    uxtheme           # SetWindowCompositionAttribute
    shcore            # SetProcessDpiAwareness
    user32
    gdi32
    shell32
    ole32
    uuid
)

# ---- 输出文件名 ----
set_target_properties(ModernDesign PROPERTIES
    OUTPUT_NAME "ModernDesign"
    DEBUG_POSTFIX ""
)

# ============================================================
# 演示应用
# ============================================================
add_subdirectory(samples/DemoApp)

# ============================================================
# 安装规则（可选）
# ============================================================
include(GNUInstallDirs)
install(TARGETS ModernDesign ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ModernDesign
        FILES_MATCHING PATTERN "*.h")

# ============================================================
# 摘要
# ============================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS " ModernDesign ${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS " Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS " Output     : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")