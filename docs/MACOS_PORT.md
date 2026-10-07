# Native desktop macOS ARM64 port

The fork builds a desktop development application and ten engine test
harnesses on Apple Silicon. The application uses the engine's frame presenter
to display a diagnostic frame through Metal. The full game executable still
uses the Windows build and application layer. The Mac targets build from an
Xcode project without CMake; [Building OpenTS](BUILDING.md#macos-desktop-build-with-xcode)
owns the commands and requirements.

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
save-game, or multiplayer compatibility. Windows configurations have not
been rebuilt in this environment.

## Xcode project

`platform/macos/OpenTS.xcodeproj` replaced the earlier CMake presets, which
have been removed; the CMake build is now Windows-only. It was
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

## Remaining work

The [port TODO](../TODO.md) owns the prioritized milestones and completion
criteria, from validating the shell's window handling and building the native
engine through asset loading, the main menu, gameplay, networking, and distribution.
