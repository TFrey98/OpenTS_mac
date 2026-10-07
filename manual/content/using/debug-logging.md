---
title: Debug logs and console
summary: Every run writes a timestamped log to ~/Library/Logs/OpenTS, and the log can be mirrored to the terminal to watch it live.
category: troubleshooting
source_files:
  - code/dbgprint.cpp
  - code/startup.cpp
  - code/init.cpp
related:
  - type: using
    id: developer-build-troubleshooting
  - type: command
    id: launch:console-debug
---

## Where the log is written

Every run, in Release and Debug builds alike, writes a new log to `~/Library/Logs/OpenTS`, where Console.app also lists it. The file is named for the local time the process started:

```
~/Library/Logs/OpenTS/DEBUG_17-08-2026_06-00-35.LOG
```

Because each run gets its own file, the log from a run that crashed is still there after the next launch. When two processes start within the same second, the second adds its process id to the name.

If the folder or the file cannot be created, the game still runs, and its output still reaches the console when one is open.

At startup, the game deletes logs last written more than 14 days ago. A single log stops growing at 64 MB, and its last line then says that the size limit was reached.

## What the log opens with

Every log opens with the OpenTS wordmark and a banner that identifies the build. For example:

```
Version  : OpenTS 0.2.0 (arm64 release build)
Commit   : f6842d2c on main (modified)
Committed: 2026-08-17 05:30:39
Started  : 2026-08-17 06:00:35
System   : macOS 27.0.1 (26A434)
Options  : -XC
```

- `Version` gives the architecture and whether this is a release or debug build.
- `modified` means the build was made from a working copy with edits, so it does not exactly match the named commit.
- `System` gives the macOS version and build.
- `Options` lists the launch options the game was started with.

## Reading the rest

Each line after the banner starts with the time it was written:

```
[06:00:35.412] Video: renderer is Metal
```

Some lines are written in several parts while the engine works through a step. Such a line carries one time, from when its first part was written:

```
[06:19:45.401] Bootstrap..... PATCH.MIX EXPAND01.MIX CACHE.MIX ...OK
```

## Watching the log live

Debug builds always mirror the log to standard error. Release builds do so when [`-XC`](/using/command-line/console-debug) is passed. Run from Terminal or Xcode, standard error is that window, so it shows the whole log as it is written, including startup. Started from Finder or with `open`, the game has no terminal, so read the file instead.

Standard error also carries text that the game has no other place to display, such as the help that `-?` prints.

After printing the help, and when it rejects an option or cannot use a directory named on the command line, the game waits for Return before it exits when it was started from a terminal, so the text stays readable.

## Before sharing a log

A log describes what the engine did, and in multiplayer that includes other people. Expect to find player names as typed in the lobby and the network addresses of the machines in the game. Read a log before attaching it to a public bug report. The same applies to an [out-of-sync report](/using/out-of-sync-reports/).
