cmake_minimum_required(VERSION 3.20)

project(DemoApp)

# 链接到 ModernDesign 静态库
add_executable(DemoAppDemo main.cpp)

target_include_directories(DemoAppDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(DemoAppDemo PRIVATE
    ModernDesign
)

# 链接相同的系统库
target_link_libraries(DemoAppDemo PRIVATE
    d2d1 dwrite dwmapi dxgi dxguid d3d11 windowscodecs uxtheme shcore user32 gdi32 shell32 ole32 uuid
)

# Windows GUI 应用
set_target_properties(DemoAppDemo PROPERTIES
    WINDOWS_EXECUTABLE ON
)