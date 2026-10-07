# Archived Windows build

OpenTS targets macOS on Apple silicon only. These files built and shipped the
Windows game and are kept for reference; nothing in the macOS build reads them.
Each file sits at its original path below this directory.

| Path | What it was |
| --- | --- |
| `CMakeLists.txt`, `code/`, `thirdparty/`, `tests/` `CMakeLists.txt` files | The CMake build for the Visual Studio `Win32` and `x64` targets, including each test harness's sources and definitions |
| `cmake/` | Build-stamp and string-table header generators, the toolkit header check, and the clang-cl cross-build toolchain |
| `code/file_win.cpp`, `code/netsocket_win32.cpp` | Win32 file search and Winsock backends; the POSIX versions remain in `code/` |
| `code/Sun.rc`, `code/except.rc`, `code/*.ico`, `code/sun.manifest`, `code/sun.natvis` | Windows resources, the application manifest, and the Visual Studio debugger visualizers |
| `.github/` | The engine CI, nightly, release, and pull-request comment workflows, and the MSVC problem matcher |
| `.vscode/` | Visual Studio Code settings for the CMake build |

The project version moved from `project(OpenTS VERSION ...)` to the top-level
`VERSION` file. The string tables in `code/language/language.rc` stay in
`code/`, since the macOS build still needs their text.
