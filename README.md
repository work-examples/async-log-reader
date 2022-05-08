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

## Test Task Description

Detailed task description is provided in a separate document:

- [task-description.md](docs/task-description.md)

## Task Implementation Remarks

Original implementation remarks letter in Russian is provided here:

- [solution-notes-letter.ru.md](docs/solution-notes-letter.ru.md)

---
