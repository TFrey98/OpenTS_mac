---
title: Build and run
summary: Builds the Debug or Release application with Xcode and runs it against a directory of game data.
category: getting-started
source_files:
  - docs/BUILDING.md
  - platform/macos/OpenTS.xcodeproj/project.pbxproj
  - code/startup.cpp
  - code/gamedirs.cpp
related:
  - type: using
    id: game-data
  - type: using
    id: developer-build-troubleshooting
---

Install Xcode 27 or newer on a Mac with Apple silicon. The repository's `docs/BUILDING.md` covers toolchain details and options.

The renderer, the audio layer, and the interface toolkits are Git submodules. Fetch them before the first build:

```sh title="Terminal"
git submodule update --init --recursive
xcodebuild -project platform/macos/OpenTS.xcodeproj -scheme OpenTS \
  -configuration Debug -derivedDataPath build/xcode build
```

The same build runs from Xcode by opening `platform/macos/OpenTS.xcodeproj` and choosing the `OpenTS` scheme. The Debug build writes `OpenTS.app` to `build/xcode/Build/Products/Debug/`, with the repository's `ui/` directory of interface documents, styles, and font inside the bundle. Use `-configuration Release` to write it to `build/xcode/Build/Products/Release/` instead. The game's strings are compiled into the application.

Put the required game data in `Run/`, then launch the application and name that data directory:

```sh title="Terminal"
open build/xcode/Build/Products/Debug/OpenTS.app --args -DATADIR="$PWD/Run"
```

Use an absolute `-DATADIR` path. The game switches to the directory holding its executable, inside the application bundle, before it reads the command line, so a relative path is read from there. Launch the application with `open`, so that it becomes the active application and its window receives focus.

Saved games and settings go to `~/Library/Application Support/OpenTS` unless [`-USERDIR`](/using/command-line/user-directory/) names another directory.
