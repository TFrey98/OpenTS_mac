---
title: Developer-build troubleshooting
summary: Checks the supported toolchain, target architecture, output location, and local game-data tree.
category: troubleshooting
source_files:
  - docs/BUILDING.md
  - platform/macos/OpenTS.xcodeproj/project.pbxproj
  - code/gamedirs.cpp
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

## The build fails before compiling

A missing header from a `thirdparty/` directory such as `thirdparty/bgfx.cmake` means the clone did not fetch the submodules. Run `git submodule update --init --recursive` and build again.

Use Xcode 27 or newer on Apple silicon. The project builds only for `arm64`; other architectures and other build tools are unsupported.

## The application is not where expected

Builds write `OpenTS.app` to `build/xcode/Build/Products/<configuration>/` when built with `-derivedDataPath build/xcode`, and copy nothing into `Run/`. A build started from Xcode without that option writes to Xcode's own DerivedData folder; Product > Show Build Folder in Finder opens it.

## The application cannot initialize game data

Name the game data directory with [`-DATADIR=<path>`](/using/command-line/data-directory/), using an absolute path. Point it at a folder holding the game's `.MIX` archives; the repository and the build directory contain no game assets.

If the named path does not exist or is not a directory, the game shows a message that the data directory cannot be used, and exits. If the directory exists but lacks the archives, the game reports that it failed to initialize.

## The window never appears

An executable started directly from Terminal does not become the active application, and the game waits for its window to receive focus. Start the application with `open` instead, as [Build and run](/using/build-and-run/) shows.
