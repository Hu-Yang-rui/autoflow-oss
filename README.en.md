# AutoFlow ⚡

> A visual desktop automation tool (RPA) for non-programmers — drag, connect, and run. No code required.

<p align="center">
  <a href="./README.md">中文</a> · <strong>English</strong>
</p>

<p align="center">
  <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" alt="C++20"></a>
  <a href="https://www.qt.io/"><img src="https://img.shields.io/badge/Qt-6.8-41CD52?style=flat-square&logo=qt&logoColor=white" alt="Qt 6.8"></a>
  <a href="https://www.microsoft.com/windows"><img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white" alt="Windows"></a>
  <a href="./LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue?style=flat-square" alt="License"></a>
</p>

AutoFlow is a Windows desktop RPA-style tool: drag instruction nodes from the left panel onto the canvas, connect them into a flowchart, configure parameters, and hit run — to automate tasks such as "open a webpage → wait for it to load → screenshot recognition → click / write to a file".

## 📑 Table of Contents

- [✨ Features](#-features)
- [📸 Screenshots](#-screenshots)
- [🛠 Tech Stack](#-tech-stack)
- [📁 Project Structure](#-project-structure)
- [🏗 Architecture](#-architecture)
- [📄 License](#-license)
- [⚠️ Disclaimer](#️-disclaimer)

## ✨ Features

- **Pure C++ / Qt 6** — framework, business logic, and styling are all C++; the UI uses Qt Widgets + QSS.
- **Drag-and-drop flow editing** — instructions are grouped into Image / Input / Data / Flow / AI, composed as nodes and edges into a visual flowchart.
- **Local OCR (offline & free)** — built on the Windows built-in OCR (`Windows.Media.Ocr`). It can read a whole screen of text, or **locate a given text and click it**, with zero external dependencies, no network, and no token cost.
- **Plugin-style instructions** — adding a new instruction only requires implementing an `IInstruction` subclass and registering it; the core engine stays untouched.
- **Variable system** — string / number / boolean / list / object, referenced via `${var}` and `${obj.key}`, with a live variable panel.
- **Four flow structures** — conditional branch (true / false), loop, jump, and sub-flow.
- **Real-time logs & variable monitoring** — each step's success / failure and duration are listed, and the running node is highlighted.

## 📸 Screenshots

<!-- Add software screenshots or a GIF demo here -->

## 🛠 Tech Stack

| Area | Choice | Notes |
|---|---|---|
| Language | C++20 | No Python / Java / C# / JS |
| UI | Qt 6 Widgets + QSS | Styles live in `resources/*.qss` |
| OCR | Windows built-in OCR (`Windows.Media.Ocr`) | Offline & free, can locate text coordinates |
| Image recognition | OpenCV (optional) | Template matching, `AUTOPLOW_WITH_OPENCV` flag |
| Color detection | Win32 (built-in) | No extra dependency |
| HTTP | WinINET (system API) | Reads the system proxy automatically |
| JSON | nlohmann/json | header-only, vendored |
| Excel | libxlsxwriter (optional) | `AUTOPLOW_WITH_XLSXWRITER` flag |
| Input simulation | Win32 `SendInput` / `SetCursorPos` | Built-in |
| Screen capture | Win32 `BitBlt` | Built-in |
| Build | CMake + Ninja + MSVC | Single-directory portable `.exe` |

## 📁 Project Structure

```
autoflow-oss/
├── CMakeLists.txt / build.bat / release.bat
├── third_party/          # nlohmann/json (header-only)
├── resources/            # icons / light & dark QSS / translations / qrc
├── examples/             # example flows (.json)
└── src/
    ├── main.cpp          # entry point
    ├── core/             # engine: variables / expressions / flow model / execution engine / records / scheduler
    ├── instructions/     # instructions: IInstruction interface + registry + built-ins
    ├── infra/            # infrastructure: input / screen / HTTP / OCR / image / Excel
    └── ui/               # UI: main window / canvas / nodes / edges / param panel / log / variables / theme
```

## 🏗 Architecture

```
UI layer (Qt Widgets + QSS)
   ↓ signals / slots
Engine layer (FlowModel parsing + ExecutionEngine background-thread scheduling)
   ↓ plugin interface IInstruction::execute(ctx, params)
Instruction layer (registry + built-in instructions)
   ↓
Infrastructure layer (OCR / OpenCV / SendInput / BitBlt / WinINET)
```

- The execution engine runs on a `std::thread` and reports progress via signals, so the UI never freezes.
- Instruction metadata (name / category / parameter schema / output ports) automatically drives the instruction panel, parameter panel, and canvas rendering.

## 📄 License

[MIT](./LICENSE)

## ⚠️ Disclaimer

This project is AI-generated; the content may contain shortcomings or quality issues. Your understanding is appreciated.
