# Desktop macOS ARM64 port TODO

Build a faithful desktop Tiberian Sun port from this fork's OpenTS engine,
using Cocoa windows, Metal rendering, and keyboard and mouse controls. Use
the desktop source as the implementation baseline; the mobile ARM64 port is
outside this plan.

This file owns the implementation checklist. [Mac port status](docs/MACOS_PORT.md)
records verified results, and [Building OpenTS](docs/BUILDING.md) owns build
commands. Review date: 2026-10-06.

Check a task only after its acceptance criteria pass. Record the configuration,
scenario, result, and relevant limitations in the status document. Keep
proprietary assets, original executables, and build output outside Git.

## Completed foundation

- [x] Add ARM64 Debug and Release CMake presets for the desktop shell and
  selected engine tests.
- [x] Build ten asset-free engine harnesses and pass all ten in both
  configurations, including Blowfish vectors and decoding tests.
- [x] Build `OpenTSMacShell.app` using SDL's Cocoa window and the desktop
  engine's bgfx frame presenter with Metal.
- [x] Run the 60-frame startup smoke check in Debug and Release and verify
  native Mach-O ARM64 executables.

These results cover a generated diagnostic frame and selected components.
Full engine startup, real assets, and gameplay remain unverified on macOS.

## Next tasks

1. **M1.1:** Resolve the Metal reference-count warnings during Debug shutdown.
2. **M2.1:** Separate the engine build from Windows sources, resources, and
   compiler options so Apple Clang can expose the remaining compilation gaps.
3. **M3.1:** Define the required asset version and validate the supplied GDI
   disc against it before connecting asset loading to game startup.

Work through the milestones in order. The asset inventory in M3.1 can proceed
alongside the build work. Multiplayer and distribution follow local gameplay
validation.

## M1: Reliable desktop shell

Review [the Mac application](platform/macos/main.cpp) and
[the frame presenter](code/bgfxbackend.cpp).

- [ ] **M1.1:** Trace texture, shader, transient buffer, and Metal device
  ownership through shutdown; fix or establish the cause of each Debug
  reference-count warning.
- [ ] **M1.2:** Interactively validate resizing, Retina scaling, Command+Return
  fullscreen, mouse capture, focus loss, Escape, and window close.
- [ ] **M1.3:** Exercise repeated launch and exit, renderer startup failure,
  and closing during presentation; verify orderly cleanup.

Complete when both configurations display the diagnostic frame, handle the
listed interactions, and exit without unresolved resource warnings or crashes.
Capture an image to verify the displayed result in addition to frame submission.

## M2: Native engine build and platform services

Review [the engine build](code/CMakeLists.txt),
[third-party dependencies](thirdparty/CMakeLists.txt),
[Windows declarations](code/win.h), and [startup](code/startup.cpp).

- [ ] **M2.1:** Introduce a reusable engine target and select platform sources
  explicitly. Keep Windows resource compilation, import libraries, MSVC
  flags, manifests, and debugger settings on the Windows path.
- [ ] **M2.2:** Build the remaining pinned dependencies on ARM64: miniaudio,
  LZO, RmlUi, FreeType, and Dear ImGui. Use the existing dependency versions.
- [ ] **M2.3:** Inventory Windows headers, handle types, calling conventions,
  intrinsics, and x86 assumptions. Replace each required dependency with a
  defined platform interface or an equivalent implementation.
- [ ] **M2.4:** Audit integer widths, pointer conversions, packing, and class
  layouts. Preserve required 32-bit values where macOS's 64-bit `long` would
  change asset, save, or packet representations.
- [ ] **M2.5:** Port required timing, threads, synchronization, diagnostics,
  startup, and shutdown services. Audit remaining COM and ABI dependencies;
  retain the existing class factory and persistence interfaces where suitable.
- [ ] **M2.6:** Expand asset-free tests for each ported boundary. Resolve the
  `void *` deletion and `AbilityFlagsType` copy warnings with evidence for
  ownership and copy behavior.

Complete when the full engine compiles and links in ARM64 Debug and Release,
the affected tests pass, and each remaining unsupported service is documented.
Run the affected Windows build checks on a Windows host before treating the
shared changes as verified there.

## M3: Original assets and file access

Review [archive loading](code/mixfile.cpp), [file access](code/rawfile.cpp),
[data directories](code/gamedirs.cpp), and [initialization](code/init.cpp).

- [ ] **M3.1:** Record the asset and behavior target. OpenTS targets Tiberian
  Sun 2.03 Firestorm; establish whether the supplied 1999 GDI disc needs
  additional data or patches for this fork. List required archives and the
  content available from this disc.
- [ ] **M3.2:** Port file enumeration, path handling, file metadata, and data
  lookup. Test case-sensitive paths, spaces, and Unicode. Keep game data
  separate from writable settings, saves, logs, and screenshots.
- [ ] **M3.3:** Run the engine's MIX reader natively against `TIBSUN.MIX` and
  nested archives. Verify encrypted headers, name lookup, offsets, checksums
  where present, and representative INI, SHP, VXL, HVA, AUD, and VQA reads.
- [ ] **M3.4:** Add synthetic archive fixtures for malformed headers, truncated
  entries, nested lookup, and decryption. Keep automated tests asset-free;
  use the local disc for manual compatibility checks.
- [ ] **M3.5:** Load rules, art, palettes, fonts, and GDI mission data through
  the native engine. Report missing or incompatible files with their names
  and expected locations.

Complete when the application reads the required assets from an explicitly
selected data directory without modifying the source disc files. The earlier
archive investigation does not establish integration with the game engine.

## M4: Engine startup, main menu, and desktop input

Review [startup](code/startup.cpp), [initialization](code/init.cpp),
[the SDL window layer](code/sdl/sdlwindow.cpp), and
[language resources](code/language/CMakeLists.txt).

- [ ] **M4.1:** Connect the Mac application to real engine initialization,
  the main loop, and shutdown, replacing the generated diagnostic frame.
- [ ] **M4.2:** Port the engine's SDL window and input layer. Map keyboard
  modifiers and mouse coordinates consistently; verify hotkeys, selection
  dragging, cursor behavior, edge scrolling, capture, and focus recovery.
- [ ] **M4.3:** Make language strings and required dialog resources available
  without `Language.dll`. Package and load the existing UI documents, fonts,
  and textures in the application bundle.
- [ ] **M4.4:** Validate the main menu, options, campaign selection, and
  skirmish setup with keyboard and mouse at multiple window sizes.

Complete when the native application reaches a functional main menu with real
assets, accepts input, and returns through engine shutdown cleanly.

## M5: Rendering, sound, and movies

Review [the frame presenter](code/bgfxbackend.cpp),
[the audio device](code/audio/audiodevice_ma.cpp), and `code/vqalib/`.

- [ ] **M5.1:** Compare real game frames with a documented reference:
  palettes, sprites, terrain, fog, lighting, voxels, cursor, sidebar, and
  UI overlays. Verify scaling and coordinate alignment on Retina displays.
- [ ] **M5.2:** Enable miniaudio's macOS device backend. Verify effects,
  speech, music, volume controls, streaming, and recovery after device changes.
- [ ] **M5.3:** Validate VQA video and audio together, including movie start,
  skip, completion, focus changes, and return to the menu or mission.
- [ ] **M5.4:** Measure frame timing and audio stability during a populated
  mission in Debug and Release; record the hardware and scenario before
  making performance changes.

Complete when a real mission and its movies render correctly and play sound
without sustained playback glitches or timing drift.

## M6: Campaign, skirmish, and persistence

Review [the main loop](code/mainloop.cpp), [save streams](code/savestream.cpp),
[save files](code/savefile.cpp), and the existing scenario and save harnesses.

- [ ] **M6.1:** Select reference scenarios and record expected rules, timing,
  AI, movement, combat, mission triggers, victory, and defeat. Record any
  known OpenTS differences relevant to the faithful port.
- [ ] **M6.2:** Play the first GDI mission through completion, then validate
  the available GDI campaign routes and skirmish sessions.
- [ ] **M6.3:** Validate Nod and Firestorm campaigns once their required
  assets are available. Keep content that cannot be tested marked pending.
- [ ] **M6.4:** Define save compatibility explicitly. Verify Mac save/load,
  quicksave, autosave, settings persistence, and damaged-save rejection.
  Detect incompatible layouts before accepting saves from other builds.
- [ ] **M6.5:** Compare deterministic simulation output across Debug and
  Release and against the selected Windows reference. Audit random number
  generation, floating-point contraction, and ordering where results diverge.

Complete when campaign and skirmish checks pass for the declared content,
Mac saves restore the tested state, and compatibility limits are documented.

## M7: Networking and cross-platform state

Review [the existing POSIX socket backend](code/netsocket_posix.cpp) and
[current save and network limits](docs/BUILDING.md#save-and-network-compatibility-between-the-platforms).

- [ ] **M7.1:** Compile and exercise the existing POSIX backend on macOS;
  test interface selection, broadcast, nonblocking I/O, and socket errors.
  Extend tests to cover the real backend as well as simulated transport.
- [ ] **M7.2:** Define protocol widths and architecture identification;
  reject incompatible peers before admitting them to a game. Version identity
  alone does not establish layout or simulation compatibility.
- [ ] **M7.3:** Validate Mac-to-Mac lobby setup, match startup, sustained
  simulation, disconnect handling, and multiplayer save/load.
- [ ] **M7.4:** Establish Mac-to-Windows interoperability only after packet
  layout and simulation comparisons pass; document tested build combinations.

Complete when compatible peers play the reference sessions without desyncs and
incompatible peers receive a clear refusal.

## M8: macOS application delivery

- [ ] **M8.1:** Package the full game as an ARM64 `.app` with its icon, UI,
  fonts, license notices, and required libraries. Keep game assets external.
- [ ] **M8.2:** Provide desktop data-directory selection and a writable user
  directory. Verify Finder launch independently of the working directory.
- [ ] **M8.3:** Add macOS Debug and Release CI for the engine and asset-free
  tests. Keep GUI and proprietary-asset checks as documented manual validation.
- [ ] **M8.4:** Choose and verify the minimum macOS version. Test installation
  on a clean Apple Silicon Mac without developer tools or Rosetta.
- [ ] **M8.5:** Prepare signing, notarization, and distribution instructions
  after gameplay validation. Record regression results and unresolved issues
  before declaring a playable release.

Complete when the packaged application runs the validated content on the
declared macOS versions and its setup instructions match a clean installation.
