# AutoFlow ⚡

> 面向零基础用户的可视化桌面自动化工具（RPA）——拖拽指令、连线编排、一键运行，无需写代码。

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" alt="C++20">
  <img src="https://img.shields.io/badge/Qt-6.8-41CD52?style=flat-square&logo=qt&logoColor=white" alt="Qt 6.8">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white" alt="Windows">
  <img src="https://img.shields.io/badge/License-MIT-blue?style=flat-square" alt="License">
</p>

AutoFlow 是一款 Windows 桌面端 RPA 类工具：用户从左侧指令面板拖拽指令到画布，用连线编排成流程图，配置参数后一键运行，即可自动完成「打开网页 → 等待加载 → 截图识别 → 点击 / 写入文件」等任务。

## 📑 目录

- [✨ 特性](#-特性)
- [📸 界面预览](#-界面预览)
- [🛠 技术栈](#-技术栈)
- [📁 目录结构](#-目录结构)
- [🏗 架构](#-架构)
- [📄 许可证](#-许可证)
- [⚠️ 免责声明](#️-免责声明)

## ✨ 特性

- **纯 C++ / Qt 6**：框架、业务、样式全部使用 C++，UI 采用 Qt Widgets + QSS。
- **拖拽式流程编排**：指令按「图像 / 键鼠 / 数据 / 流程 / AI」五类分组，以节点 + 连线组成可视化流程图。
- **本地 OCR（离线免费）**：基于 Windows 内置 OCR（Windows.Media.Ocr），可识别整页文字，也可**定位指定文字并点击**，零外部依赖、不联网、不消耗 token。
- **指令插件化**：新增指令只需实现 `IInstruction` 子类并注册，核心引擎无需改动。
- **变量系统**：支持字符串 / 数字 / 布尔 / 列表 / 对象，`${var}`、`${obj.key}` 引用，运行面板实时查看。
- **四种流程结构**：条件分支（真 / 假）、循环、跳转、子流程。
- **实时日志与变量监控**：每步成功 / 失败与耗时逐条展示，运行中节点高亮。

## 📸 界面预览

<!-- 在此处添加软件截图或 GIF 演示 -->

## 🛠 技术栈

| 项 | 选型 | 说明 |
|---|---|---|
| 语言 | C++20 | 无 Python / Java / C# / JS |
| UI | Qt 6 Widgets + QSS | 样式集中在 `resources/*.qss` |
| OCR | Windows 内置 OCR（Windows.Media.Ocr） | 离线免费，可定位文字坐标 |
| 图像识别 | OpenCV（可选） | 模板匹配找图，`AUTOPLOW_WITH_OPENCV` 开关 |
| 找色 | Win32（内置） | 无需额外依赖 |
| HTTP | WinINET（系统 API） | 自动读取系统代理 |
| JSON | nlohmann/json | header-only，已 vendor |
| Excel | libxlsxwriter（可选） | `AUTOPLOW_WITH_XLSXWRITER` 开关 |
| 键鼠模拟 | Win32 `SendInput` / `SetCursorPos` | 内置 |
| 屏幕抓取 | Win32 `BitBlt` | 内置 |
| 构建 | CMake + Ninja + MSVC | 产物为单目录绿色 `.exe` |

## 📁 目录结构

```
autoflow-oss/
├── CMakeLists.txt / build.bat / release.bat
├── third_party/          # nlohmann/json（header-only）
├── resources/            # 图标 / 深浅色 QSS / 翻译 / qrc
├── examples/             # 示例流程（.json）
└── src/
    ├── main.cpp          # 入口
    ├── core/             # 引擎层：变量 / 表达式 / 流程模型 / 执行引擎 / 运行记录 / 调度器
    ├── instructions/     # 指令层：IInstruction 接口 + 注册表 + 内置指令
    ├── infra/            # 基础设施：键鼠 / 屏幕 / HTTP / OCR / 图像 / Excel
    └── ui/               # UI：主窗口 / 画布 / 节点 / 连线 / 参数面板 / 日志 / 变量 / 主题
```

## 🏗 架构

```
UI 层（Qt Widgets + QSS）
   ↓ 信号 / 槽
引擎层（FlowModel 解析 + ExecutionEngine 后台线程调度）
   ↓ 插件接口 IInstruction::execute(ctx, params)
指令层（注册表 + 内置指令）
   ↓
基础设施层（OCR / OpenCV / SendInput / BitBlt / WinINET）
```

- 执行引擎在 `std::thread` 后台运行，通过信号回报进度，界面不卡顿。
- 指令元数据（名称 / 分类 / 参数 schema / 输出端口）自动驱动指令面板、参数面板与画布渲染。

## 📄 许可证

[MIT](./LICENSE)

## ⚠️ 免责声明

本项目由 AI 生成，内容可能存在不足或质量问题，敬请谅解。
