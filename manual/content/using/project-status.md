---
title: Project status
summary: This fork runs OpenTS natively on Apple silicon Macs; single-player campaigns and cinematics work.
category: getting-started
source_files:
  - README.md
  - docs/BUILDING.md
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

This fork ports the reconstructed Tiberian Sun engine to macOS on Apple silicon, its only target. It builds a native `OpenTS.app` with Xcode, drawing through Metal and playing sound through Core Audio.

The single-player game runs from the original game data: the GDI and Nod campaigns, the Firestorm expansion, and the cinematics. Multiplayer, and saves exchanged with the Windows build of upstream OpenTS, have not been tested. The repository's `docs/MACOS_PORT.md` lists the known gaps.

Upstream OpenTS release 0.1.0, which this fork starts from, ran the full Tiberian Sun 2.03 Firestorm game on Windows.

## Builds

There are no prebuilt releases of this fork; build it from source. OpenTS does not distribute the original game assets. They come from an existing copy of Tiberian Sun; [Game data](/using/game-data/) covers where the game finds them.

## Toolchain and targets

- Xcode 27 or newer
- macOS on Apple silicon (`arm64`)
- C++20
- Debug and Release configurations

[Build and run](/using/build-and-run/) gives the commands.
