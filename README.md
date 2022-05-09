# Asynchronous Log File Reader [![Code license](https://img.shields.io/github/license/work-examples/async-log-reader)](LICENSE)

**Example project:** Log file reader and filter (`grep` analog).
Application reads specified file and writes matchig lines to `stdout`.
It supports simple regex (`fnmatch` algorithm).
Reading is implemented using direct Win32 API with a number of approaches, including asynchronous WinAPI.
C++ exceptions are not used (forbidden).

**Language**: `C++17`  
**Dependencies**: none  
**Software requirements**: `Visual Studio 2019`  
**Operation systems**: `Windows`

| Branch      | CI Build Status                                                                                                                                                                                                              | CodeQL Code Analysis                                                                                                                                                                                                                                             |
|-------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **master**  | [![CI status](https://github.com/work-examples/async-log-reader/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/work-examples/async-log-reader/actions/workflows/build.yml?query=branch%3Amaster)   | [![CodeQL Code Analysis Status](https://github.com/work-examples/async-log-reader/actions/workflows/codeql-analysis.yml/badge.svg?branch=master)](https://github.com/work-examples/async-log-reader/actions/workflows/codeql-analysis.yml?query=branch%3Amaster) |
| **develop** | [![CI status](https://github.com/work-examples/async-log-reader/actions/workflows/build.yml/badge.svg?branch=develop)](https://github.com/work-examples/async-log-reader/actions/workflows/build.yml?query=branch%3Adevelop) | \[not applicable\]                                                                                                                                                                                                                                               |

## C++ Programmer's Test Task Description

Detailed task description is provided in a separate document:

- [C++ Programmer's Test Task Description](docs/task-description.md) (`docs/task-description.md`)

## Test Task Implementation Notes

Original implementation notes letter is provided here in two languages:

- [EN: Test Task Implementation Notes](docs/implementation-notes-letter.md) (in English, `docs/implementation-notes-letter.md`)
- [RU: Заметки по решению](docs/implementation-notes-letter.ru.md) (in Russian, `docs/implementation-notes-letter.ru.md`)

---
