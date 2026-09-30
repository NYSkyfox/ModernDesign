# ModernDesign — Fluent Design (WinUI 3 风格) 原生 C++ 框架

**目标**：纯 C++ 原生实现 Fluent Design 控件库 + 演示应用，零第三方依赖，单文件 exe 分发。

## 构建方式

```powershell
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 目录结构

```
ModernDesign/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── app/          窗口、渲染上下文、输入管理、主题
│   ├── core/         设计令牌、动画、布局、阴影、亚克力、DPI
│   ├── controls/     控件库（Button / ToggleSwitch / CheckBox / …）
│   ├── utils/        颜色、数学、ComPtr、字符串、日志
│   └── resources/    代码内嵌资源 ID
├── samples/DemoApp/  演示应用
├── tests/            单元测试（可选）
├── docs/             设计令牌、控件规范、架构、构建指南
└── build/            CMake 输出（不入库）
```

## 依赖

- Windows SDK 10.0.17763.0+（Win10 1809+）
- Visual Studio 2022（MSVC v143）
- CMake 3.20+
- C++17

## 许可证

MIT