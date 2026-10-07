# macOS port status

OpenTS targets macOS on Apple silicon only. The fork builds a development
application and ten engine test harnesses from an Xcode project. The
application uses the engine's frame presenter to display a diagnostic frame
through Metal; the full game does not yet compile or link.
[Building OpenTS](BUILDING.md#build-with-xcode) owns the commands and
requirements.

The game will use Cocoa windows, Metal rendering, and keyboard and mouse
controls.

The port will retain OpenTS's game logic and original asset formats. Its
source baseline targets Tiberian Sun 2.03 Firestorm. Compatibility with the
1999 GDI disc assets has not been established for a native game build.

## Verified components

Both Debug and Release desktop shells created a Cocoa window with a 1920 by
1440 drawable and presented 60 RGB565 frames through Metal, then exited
successfully.
The native executable is Mach-O ARM64. This verifies startup and frame
submission; window resizing, mouse capture, and keyboard handlers have not
received interactive validation.

## Debug shutdown reference counts

bgfx's Debug Metal renderer warns when a released object still has other
references. Before the fix, shutdown destroyed the frame presenter's resources
while up to three submitted frames could still hold them. In 6 of 25 Debug
smoke runs, this produced extra warnings for the frame texture, transient vertex
buffers, and the swap-chain depth/stencil texture. Those objects were released
when the frames retired; they did not leak. `Backend_Shutdown` now submits
three empty frames before destroying resources. Fifty consecutive Debug runs
produced only the three warnings below, and ten Release runs produced none.

The remaining warnings come from bgfx's checks, not from leaked objects:

- `SH 12: vs_ocornut_imgui` and `SH 13: fs_ocornut_imgui`, with one reference
  each: bgfx reuses one pipeline descriptor, which still references the last
  pipeline's shader functions when the shaders are destroyed. Instrumented
  counts were 2 when the pipeline was created and 1 when each shader was
  destroyed; the pipeline state did not reference the functions. Clearing the
  descriptor's functions in `destroyShader` removed both warnings. The
  descriptor releases the functions during renderer shutdown.
- `?!`, with 37 to 47 references: this is the shared system Metal device.
  `MTLCreateSystemDefaultDevice` returned the same object both times it was
  called, and the device still had 37 references after bgfx released its own
  and drained its autorelease pool. Metal and the window's layer hold the
  remaining references, so the check's expected count of 0 does not apply.

Removing the shader warnings requires a bgfx change: release the descriptor's
functions in `destroyShader`. The device check can be suppressed only in
bgfx.

The portable suite compiles the existing engine implementations directly.
Its tests exercise recorded output for color conversion, VQA frame decoding
and LCW compression, LCW block stream bounds, audio queues and levels,
priority queue tie ordering, and shape facings. A Blowfish test checks
encryption, decryption, and in-place decryption against known vectors.

Debug and Release builds passed all ten harnesses locally with Apple Clang
21.0.0 and CMake 4.4.4 on macOS ARM64. Clang reported inherited warnings for
deleting `void *` buffers in `code/buff.cpp` and copying `AbilityFlagsType`
with `memcpy` in `code/ability.hh`. These tests do not establish full engine,
save-game, or multiplayer compatibility.

## Xcode project

`platform/macos/OpenTS.xcodeproj` replaced the earlier CMake presets. It was
checked with Xcode 27.0 (27A266a) and the macOS 27.0 SDK. Both configurations
built the shell and all ten harnesses, and the `PortableTests` scheme passed
all ten. Clang reported the same inherited `buff.cpp` and `ability.hh`
warnings. The Debug shell presented 60 frames through Metal in five
consecutive smoke runs, with only the three bgfx warnings explained above.
The Release shell completed three runs without warnings. The executable is
Mach-O ARM64 and links only system libraries.

The project compiles SDL statically from its source instead of referencing
SDL's Xcode project. SDL 3.4.16's project sets a macOS 10.13 deployment
target, which Xcode 27 rejects, and its only override is a file inside the
submodule. Unlike the earlier CMake build, this SDL build includes its audio,
GPU, joystick, haptic, and other subsystems; the shell does not initialize
them.

## Engine build

The `OpenTSEngine` target compiles all of the engine except the Windows
string DLL's entry point, and `OpenTS.app` links it with the VQA library and
every third-party dependency. Debug and Release both build and link with Apple
Clang, C++20, and `-fms-extensions`, which accepts the engine's
`__declspec(property)` declarations. The engine is compiled with
`-fno-fast-math -ffp-contract=off`, matching the Windows build's
floating-point model. The application links only system libraries.

Run with no game data, the Debug application logged its startup: the
single-instance lock, the game directories, Core Audio output, the Metal
renderer, the RmlUi shell and its fonts, surface allocation, and encryption
key setup. It then reported that it could not open `CACHE.MIX`. A bare
executable started from a background terminal waits for window focus that
never comes; launch the bundle with `open` instead.

The build reports 22 warnings in the application and several hundred in the
engine. They include 99 `-Wshorten-64-to-32` truncations for the M2.4 audit,
and 33 `-Wformat` mismatches in log calls, which `DebugString`'s format
checking now reveals.

### Win32 replacements

`code/win.h` defines the Win32 names the engine still uses for macOS:
fixed-width types, so `DWORD` and `LONG` stay 32-bit; `FILETIME`, which save
files store in its Windows representation; the system time functions;
millisecond timing; and the drag threshold. `code/vkey.h` holds the
virtual-key table with Windows values, which saved hotkeys store. Other Win32
calls were replaced in the files that used them:

- `main` replaces `WinMain`. A `flock` lock file in `$TMPDIR` replaces the
  single-instance mutexes; the Westwood AutoPlay mutex has no counterpart.
- The SDL window layer creates a Metal window and hands the renderer its Cocoa
  window. Mouse capture uses SDL's own state, and the window takes focus only
  from the game's other windows, never from another application.
- The language strings are compiled in. `platform/macos/generate_headers.py`
  turns the string tables in `code/language/language.rc` into
  `opents_language.h`, with all 750 strings.
- Game surfaces hold their pixels in memory, and a size-changing copy between
  them is scaled nearest-neighbour, as Windows' `COLORONCOLOR` stretch did.
  `code/surftext.cpp` draws the remaining TrueType text, the end credits and
  the tactical caption, with FreeType and macOS's Arial. It follows Win32
  `CreateFont` sizing, using the OS/2 table's Windows ascent and descent.
- The load, save, and random-map dialogs, game directories, and save files use
  the portable file search and POSIX I/O. Save files are flushed with
  `F_FULLFSYNC`.
- The debug log writes to `~/Library/Logs/OpenTS` and mirrors to standard
  error when the console is requested.
- CPU identity reports the Apple chip name and the P6 family, so the
  family-gated timing paths keep their modern-hardware branch. The tick counter
  is `mach_absolute_time`, with its rate taken from the timebase.
- Best-fit code page lookup uses `iconv` transliteration. Its close matches can
  differ from Windows' best-fit tables.
- The sync recorder takes call-site offsets from the executable's Mach-O
  header. `Describe_Code_Address` names exported functions through `dladdr`.
- The UI asks for macOS's Microsoft Sans Serif and Arial font files. The
  Windows bitmap fonts it also tries do not exist on macOS, so the TrueType
  faces answer for them.

Fixed while porting:

- The VQA `SN2J` record used `long` and was 20 bytes instead of 12.
- The sync recorder truncated 64-bit return addresses to 32 bits before
  taking their offsets.
- The random-map cache cleanup deleted the wrong path, so it never removed old
  previews.
- The POSIX file search kept a pointer to its caller's pattern string.

Behavior that differs from Windows: a key already held when the window regains
focus is not reported as held until it is pressed again, and the Westwood
Online serial lookup finds no serial. The crash handler is a stub; macOS writes
its own crash reports. The `sdlkeys` contract test loads Windows keyboard
layouts and must be rewritten for macOS, and the 40 harnesses not yet in the
Xcode project still need porting.

## Game data

The supplied image is the 1999 GDI disc, `CD1_GDI.iso`, from the freeware
release. OpenTS has no disc check: it reads the archives from the data
directory named with `-DATADIR=`. The disc's own copy protection only guarded
the original `GAME.EXE`, which OpenTS does not use. Six archives from the
image, `INSTALL/TIBSUN.MIX` and `MAPS01.MIX`, `MOVIES01.MIX`, `MULTI.MIX`,
`SCORES.MIX`, and `SIDECD01.MIX` from its root, were copied into the
Git-ignored `Run/` directory. `TIBSUN.MIX` holds the cache, local, conquer,
sound, and speech archives that startup opens.

With that directory, the Debug application completed `Game Init Completed`.
It read the rules, sides, sound, and theme files, started the title music,
and opened `menu.rml`. From the menu, the first GDI mission, `GDI1A.MAP`,
loaded with its briefing and in-game music, and the log recorded no errors.
The user confirmed the mission opened on screen. Rendering, input, and
gameplay have not yet been checked against the original. The Nod disc,
`CD2_Nod.iso`, and the Firestorm disc, `CD_3-Firestorm.iso`, were added later.
`Run/` now also holds the Nod disc's `MAPS02.MIX` and `MOVIES02.MIX`. From the
Firestorm disc it takes `EXPAND01.MIX`, `WDTVOX.MIX`, and `MULTI.MIX` from
`Install/`, and `MAPS03.MIX`, `MOVIES03.MIX`, `E01SCD01.MIX`, `E01SCD02.MIX`,
`SCORES01.MIX`, `WDT.MIX`, `SIDECD01.MIX`, and `SIDECD02.MIX` from its root.
Where the Firestorm disc carries a file that also exists on the 1999 discs,
its copy is used. The Nod disc's `TIBSUN.MIX` and `MULTI.MIX` match the GDI
disc's. The game has not yet been run with these files, and whether they
match the 2.03 Firestorm baseline is still M3.1's question.

Disc images and archives are ignored anywhere in the repository (`*.iso`,
`*.mix`), and none were ever committed.

Two faults were fixed on the way:

- The SHA-1 digest union declared its five words as `unsigned long`, so on
  macOS the digest was 40 bytes with its bytes misplaced, and every digest
  check failed. `CACHE.MIX` carries a digest, so startup reported it could not
  load. The words are now `uint32_t`, and a `static_assert` holds the size at
  20 bytes.
- The player's files now default to `~/Library/Application Support/OpenTS`.
  They went to the working directory, which is the executable's folder inside
  the application bundle, and writing `sun.ini` there broke the bundle's code
  signature. `create_directories` reports false for a path ending in a
  separator even when it creates the folder, so the folder itself is now
  checked afterwards.

## Remaining work

The [port TODO](../TODO.md) owns the prioritized milestones and completion
criteria, from validating the shell's window handling and building the native
engine through asset loading, the main menu, gameplay, networking, and distribution.
