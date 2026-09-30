# CompilerWarnings.cmake
#
# 统一编译警告等级设置。
# 目标：把尽可能多的潜在问题提升为警告（而非静默忽略），
# 同时抑制一些已知的、无意义的警告（如 MSVC 标准库的 C4577 等）。
#
# 用法：
#   include(CompilerWarnings)
#   apply_compiler_warnings(<target>)

include(CheckCXXCompilerFlag)

# ---- 全局警告等级 ----
# /W4：高级警告（比 /W3 更严，但不包含 /WX 之外的误报）
# /wd4005：禁用"宏重定义"（常见于 Windows 头文件的 _WIN32_WINNT 重定义）
# /wd4251：禁用"dllexport/dllimport"不匹配（模板类导出问题，非本项目关注点）
# /wd4275：禁用"派生类与基类 DLL 链接不匹配"
# /wd4505：禁用"未引用本地函数"（模板实例化后可能残留）
# /wd4514：禁用"未引用内联函数"（优化后可能残留）
# /wd4867：禁用"非标准形式的 sizeof"（某些 Windows 头文件用法）
# /wd4996：禁用"不推荐使用的函数/变量"（如 strcpy、strncpy 等）

set(MODERN_DESIGN_WARNING_FLAGS
    /W4
    /wd4005
    /wd4251
    /wd4275
    /wd4505
    /wd4514
    /wd4867
    /wd4996
    /wd5045
    /permissive-       # 强制标准 C++（禁用 MSVC 扩展）
)

# ---- 应用到目标 ----
function(apply_compiler_warnings TARGET)
    target_compile_options(${TARGET} PRIVATE ${MODERN_DESIGN_WARNING_FLAGS})
    message(STATUS "CompilerWarnings: applied to ${TARGET}")
endfunction()

# ---- 也作为全局默认 ----
# 通过 add_compile_options 在顶层统一设置，避免每个目标重复调用
function(setup_global_warnings)
    add_compile_options(${MODERN_DESIGN_WARNING_FLAGS})
    message(STATUS "CompilerWarnings: applied globally")
endfunction()