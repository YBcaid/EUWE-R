# EUWE-R (Work In Progress)

**EUWE-R** is a Qt6.8/C++ desktop application that extracts unfamiliar English words from various documents and images, looks up dictionary definitions, and helps you build a personal vocabulary database.

> 🚧 **This project is under active development.**  
> The current codebase is a major refactoring of the original EUWE. Not all features are complete. Collaboration and feedback are welcome.

## Features

- **Text extraction** – Extract English words from plain text, PDF, DOCX, PPTX, HTML, and Markdown.
- **OCR support** – Recognize text from images (PNG, JPG, etc.) via an external OCR engine.
- **Dictionary lookup** – Query word meanings and phonetic symbols from a built‑in SQLite dictionary.
- **Vocabulary management** – Maintain a “known words” database (words you already know).
- **Export** – Save new words as CSV, PDF, DOC, or JSON.
- **Modern UI** – Built with Qt Quick/QML, supporting dark/light themes.

## Current Status

The project is being **fully rewritten** to improve modularity and maintainability. Some components are still empty stubs or under testing. The main goal is to create a stable, extensible tool for language learners.

## Build Requirements

- Qt 6.8+ (widgets, sql, axcontainer, pdf, quick, quickcontrols2)
- C++17 compiler
- External OCR executable (if using OCR features)

## Contributing

Pull requests and issue reports are welcome. Please note the codebase is evolving rapidly.

---

📌 *For detailed documentation, please wait for the first stable release.*

---

# EUWE-R（开发中）

**EUWE-R** 是一个基于 Qt6.8/C++ 的桌面工具，可以从多种文档和图片中提取陌生英语单词，查询词典释义，并帮助你建立个人生词库。

> 🚧 **本项目正在积极开发中。**  
> 当前代码是对原 EUWE 的重构版本，尚未完成全部功能。欢迎协作与反馈。

## 功能概览

- **文本提取** – 从纯文本、PDF、DOCX、PPTX、HTML、Markdown 中提取英语单词。
- **OCR 识别** – 通过外部 OCR 引擎识别图片中的文字。
- **词典查询** – 从内置 SQLite 词典中查询单词释义和音标。
- **生词管理** – 维护“熟词库”（已掌握的单词）。
- **导出** – 将生词导出为 CSV、PDF、DOC 或 JSON 格式。
- **现代化界面** – 使用 Qt Quick/QML 实现，支持深色/浅色主题。

## 当前状态

项目正在**完全重构**，以提高模块化和可维护性。部分组件尚为空壳或处于测试阶段。主要目标是打造一个稳定、可扩展的语言学习工具。

## 构建要求

- Qt 6.8+（widgets, sql, axcontainer, pdf, quick, quickcontrols2）
- C++17 编译器
- 外部 OCR 可执行文件（如需使用 OCR 功能）

## 参与贡献

欢迎提交 Pull Request 和 Issue。请注意代码库正在快速演进。

---

📌 *详细文档请等待首个稳定版本发布。*
