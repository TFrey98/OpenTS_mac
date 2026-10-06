# Native desktop macOS ARM64 port

The fork builds a desktop development application and ten engine test
harnesses on Apple Silicon. The application uses the engine's frame presenter
to display a diagnostic frame through Metal. The full game executable still
uses the Windows build and application layer. [Building OpenTS](BUILDING.md#macos-desktop-window-and-frame-presenter)
owns the commands and requirements for the Mac configurations.

The source for this port is the desktop OpenTS engine in this fork. The
application will use macOS windows, Metal rendering, and keyboard and mouse
controls. "Portable" in the test configuration means engine components that
compile across operating systems.

The port will retain OpenTS's game logic and original asset formats. Its
source baseline targets Tiberian Sun 2.03 Firestorm. Compatibility with the
1999 GDI disc assets has not been established for a native game build.

## Verified components

Both Debug and Release desktop shells created a Cocoa window with a 1920 by
1440 drawable and presented 60 RGB565 frames through Metal, then exited
successfully.
The native executable is Mach-O ARM64. This verifies startup and frame
submission; window resizing, mouse capture, and keyboard handlers have not
received interactive validation. The bgfx Debug build emitted reference-count
warnings during shutdown; GPU resource lifetimes need further investigation.

The portable suite compiles the existing engine implementations directly.
Its tests exercise recorded output for color conversion, VQA frame decoding
and LCW compression, LCW block stream bounds, audio queues and levels,
priority queue tie ordering, and shape facings. A Blowfish test checks
encryption, decryption, and in-place decryption against known vectors.

Debug and Release builds passed all ten harnesses locally with Apple Clang
21.0.0 and CMake 4.4.4 on macOS ARM64. Clang reported inherited warnings for
deleting `void *` buffers in `code/buff.cpp` and copying `AbilityFlagsType`
with `memcpy` in `code/ability.hh`. These tests do not establish full engine,
save-game, or multiplayer compatibility. Windows configurations have not
been rebuilt in this environment.

## Remaining work

The [port TODO](../TODO.md) owns the prioritized milestones and completion
criteria, from resolving shell shutdown warnings and building the native engine
through asset loading, the main menu, gameplay, networking, and distribution.
