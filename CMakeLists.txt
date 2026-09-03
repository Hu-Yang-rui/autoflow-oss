cmake_minimum_required(VERSION 3.21)
project(AutoFlow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------------------------------------
# 可选第三方能力开关（Phase 2 / Phase 3 重功能，默认 OFF，保证 MVP 零外部依赖）
# ---------------------------------------------------------------------------
option(AUTOPLOW_WITH_OPENCV     "启用 OpenCV 图像识别(找图/找色)" OFF)
option(AUTOPLOW_WITH_TESSERACT  "启用 Tesseract OCR"               OFF)
option(AUTOPLOW_WITH_XLSXWRITER "启用 libxlsxwriter Excel 写入"    OFF)
option(AUTOPLOW_WITH_OPENSSL    "启用 cpp-httplib HTTPS(需 OpenSSL)" OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(AUTOPLOW_SOURCES
    src/main.cpp

    src/core/Variable.cpp
    src/core/Expression.cpp
    src/core/FlowModel.cpp
    src/core/ExecutionEngine.cpp
    src/core/RunRecord.cpp
    src/core/Scheduler.cpp
    src/core/Settings.cpp

    src/instructions/InstructionRegistry.cpp
    src/instructions/Builtins.cpp
    src/instructions/FlowInstructions.cpp
    src/instructions/InputInstructions.cpp
    src/instructions/DataInstructions.cpp
    src/instructions/VisionInstructions.cpp
    src/instructions/SystemInstructions.cpp

    src/infra/InputSimulator.cpp
    src/infra/ScreenCapture.cpp
    src/infra/HttpClient.cpp
    src/infra/OcrWin.cpp

    src/ui/MainWindow.cpp
    src/ui/InstructionPanel.cpp
    src/ui/FlowCanvas.cpp
    src/ui/NodeItem.cpp
    src/ui/EdgeItem.cpp
    src/ui/ParamPanel.cpp
    src/ui/CropOverlay.cpp
    src/ui/LogPanel.cpp
    src/ui/VariablePanel.cpp
    src/ui/ThemeManager.cpp
    src/ui/TitleBar.cpp
    src/ui/IconButton.cpp
    src/ui/Stepper.cpp
    src/ui/HotkeyManager.cpp
    src/ui/ThemeToggle.cpp
    src/ui/SettingsDialog.cpp
    src/ui/TemplateDialog.cpp
    src/ui/DynamicIsland.cpp
    src/ui/RunBorder.cpp
)

# 翻译（i18n）：源串为中文，英文翻译已预生成并打包进 qrc（":/autoflow_en.qm"）。
# 修改界面文字后重新生成：
#   C:\Qt\6.8.3\msvc2022_64\bin\lupdate.exe src -ts translations\autoflow_en.ts
#   python translations\fill_en.py        （填充/更新英文译文）
#   C:\Qt\6.8.3\msvc2022_64\bin\lrelease.exe translations\autoflow_en.ts -qm resources\autoflow_en.qm
add_executable(AutoFlow WIN32 ${AUTOPLOW_SOURCES} resources/resources.qrc resources/app.rc)

target_include_directories(AutoFlow PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib
)

target_link_libraries(AutoFlow PRIVATE
    Qt6::Widgets
    Qt6::Network
    ws2_32
    wininet       # WinINET：和 PowerShell/.NET 同一套 API，代理自动处理
    WindowsApp    # Windows 内置 OCR（Windows.Media.Ocr）等 WinRT API
)

if(MSVC)
    target_compile_options(AutoFlow PRIVATE /utf-8 /W3 /EHsc)
    # 单目录绿色免安装产物
    set_target_properties(AutoFlow PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# 静态链接 VC 运行时，避免目标机缺 vcruntime（可选：发布时打开）
option(AUTOPLOW_STATIC_CRT "静态链接 MSVC 运行时" OFF)
if(AUTOPLOW_STATIC_CRT AND MSVC)
    set_target_properties(AutoFlow PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ---- OpenCV (Phase 2) ----
if(AUTOPLOW_WITH_OPENCV)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc)
    target_link_libraries(AutoFlow PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(AutoFlow PRIVATE AUTOPLOW_HAS_OPENCV=1)
endif()

# ---- Tesseract (Phase 2) ----
if(AUTOPLOW_WITH_TESSERACT)
    find_package(Tesseract REQUIRED)
    target_link_libraries(AutoFlow PRIVATE Tesseract::libtesseract)
    target_compile_definitions(AutoFlow PRIVATE AUTOPLOW_HAS_TESSERACT=1)
endif()

# ---- libxlsxwriter (Phase 3) ----
if(AUTOPLOW_WITH_XLSXWRITER)
    find_package(XLSXWriter REQUIRED)
    target_link_libraries(AutoFlow PRIVATE xlsxwriter)
    target_compile_definitions(AutoFlow PRIVATE AUTOPLOW_HAS_XLSXWRITER=1)
endif()

# ---- OpenSSL (HTTPS) ----
if(AUTOPLOW_WITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(AutoFlow PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(AutoFlow PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT=1)
endif()

# 部署：windeployqt 收集 Qt DLL，生成绿色免安装目录
if(WIN32)
    get_target_property(_qmake Qt6::qmake LOCATION)
    get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
    add_custom_target(deploy
        COMMAND "${_qt_bin}/windeployqt.exe" --no-translations --compiler-runtime "$<TARGET_FILE:AutoFlow>"
        COMMENT "运行 windeployqt 收集依赖"
        DEPENDS AutoFlow
        VERBATIM)
endif()
