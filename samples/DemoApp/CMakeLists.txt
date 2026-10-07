cmake_minimum_required(VERSION 3.20)

project(DemoApp)

# 链接到 ModernDesign 静态库
add_executable(DemoAppDemo main.cpp
    DemoApp.h
    DemoPage.h
    pages/HomePage.cpp
    pages/ButtonsPage.cpp
    pages/SelectionPage.cpp
    pages/SlidersPage.cpp
    pages/ExpanderPage.cpp
    pages/CardPage.cpp
    pages/ShowcasePage.cpp
    pages/PopupsPage.cpp
    pages/DialogPage.cpp
    pages/SettingsPage.cpp
)

target_include_directories(DemoAppDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pages
)

target_link_libraries(DemoAppDemo PRIVATE
    ModernDesign
)

# 链接相同的系统库
target_link_libraries(DemoAppDemo PRIVATE
    d2d1 dwrite dwmapi dxgi dxguid d3d11 windowscodecs uxtheme shcore user32 gdi32 shell32 ole32 uuid
)

# Windows GUI 应用（用 wWinMain，而非控制台 main）
set_target_properties(DemoAppDemo PROPERTIES
    WIN32_EXECUTABLE ON
)