---
title: Crash reports
summary: macOS records a crash in its own crash report; attach that report and the run's debug log to a bug report.
category: troubleshooting
source_files:
  - code/except.cpp
  - code/startup.cpp
  - code/init.cpp
related:
  - type: using
    id: debug-logging
  - type: command
    id: launch:exception-test
---

## Where a crash is written

The game writes no crash files of its own. When it crashes, macOS writes a crash report to `~/Library/Logs/DiagnosticReports`, named for the game and the time, such as:

```
~/Library/Logs/DiagnosticReports/OpenTS-2026-08-17-061945.ips
```

Console.app lists these reports under Crash Reports, and macOS offers to show one in a dialog after the crash. The report holds the reason the process stopped, the stack of every thread, and the libraries loaded.

## Reporting a crash

Attach the crash report together with that run's [debug log](/using/debug-logging/) from `~/Library/Logs/OpenTS`. The log's last lines show what the engine was doing, and its banner identifies the build the report came from.

When a debugger such as Xcode's is attached to the game, the debugger stops at the crash instead, and macOS writes no report.

## Testing crash reporting

The [`-EXCEPTIONTEST`](/using/command-line/exception-test/) option belongs to the Windows build's own crash handler. On macOS it does nothing.
