# Building OpenTS

> [!IMPORTANT]
> OpenTS targets macOS on Apple silicon only. The Xcode project builds the
> game, a development shell, and ten engine test harnesses. The game needs the
> original game data to run. [Mac port status](MACOS_PORT.md) records what has
> been verified and the known gaps.

## Supported target

| Component | Requirement |
| --- | --- |
| Host and target | macOS on Apple silicon (`arm64`) |
| Minimum macOS | 13.0, set in the project but not yet tested on that version |
| Toolchain | Xcode 27 or newer |
| C++ language level | C++20 (`gnu++20`) |
| Configurations | Debug and Release |

No other build tools are needed.

## Dependencies

Fetch the third-party submodules once after cloning, or clone with
`git clone --recurse-submodules`:

```sh
git submodule update --init --recursive
```

The renderer uses [bgfx](https://github.com/bkaradzic/bgfx), vendored through
`thirdparty/bgfx.cmake` at a tested tag. That submodule contains bgfx, bx, and
bimg as nested submodules. The Xcode project compiles bgfx from its
amalgamated source as Objective-C++, and bimg from the sources that
`bgfx.cmake` selects.

[SDL](https://github.com/libsdl-org/SDL) 3 is vendored through `thirdparty/SDL`
at a tested tag and linked statically. The Xcode project compiles the sources
that SDL's own Xcode project builds for macOS, with SDL's stock macOS
configuration header. SDL's Xcode project cannot be referenced directly,
because its macOS 10.13 deployment target is below the minimum Xcode 27
accepts.

The audio layer uses [miniaudio](https://github.com/mackron/miniaudio),
vendored through `thirdparty/miniaudio` at a tested tag and compiled as one
translation unit from `thirdparty/miniaudio-impl.c`.

The user interface toolkits are [RmlUi](https://github.com/mikke89/RmlUi),
with [FreeType](https://freetype.org) rasterizing its fonts, and
[Dear ImGui](https://github.com/ocornut/imgui) for developer overlays. They are
vendored through `thirdparty/RmlUi`, `thirdparty/freetype`, and
`thirdparty/imgui` at tested tags.

Compression uses [LZO](https://www.oberhumer.com/opensource/lzo/) 2.10,
vendored under `thirdparty/lzo`. Upstream publishes releases as a tarball
rather than through a repository, so this copy is checked in instead of pinned
as a submodule. It holds only the LZO1X-1 sources the engine calls; take a
later release by extracting it over the files already there, in a separate
change.

The Xcode project builds every dependency as a static library with the options
the engine expects: miniaudio without its engine, node graph, resource
manager, generation, or encoding; FreeType with its stock options; RmlUi's
core with the FreeType font engine; and Dear ImGui's core without obsolete
functions. Update a pinned tag in a separate change.

## Build with Xcode

Open `platform/macos/OpenTS.xcodeproj` in Xcode and choose a scheme:

| Scheme | Result |
| --- | --- |
| `OpenTS` | `OpenTS.app`, the game |
| `OpenTSEngine` | The engine library alone |
| `OpenTSMacShell` | `OpenTSMacShell.app`, the desktop window and frame presenter |
| `PortableTests` | Builds the ten engine harnesses and runs them; the build fails if one fails |

From Terminal, build into `build/xcode` with:

```sh
xcodebuild -project platform/macos/OpenTS.xcodeproj -scheme OpenTSMacShell \
  -configuration Debug -derivedDataPath build/xcode build
xcodebuild -project platform/macos/OpenTS.xcodeproj -scheme PortableTests \
  -configuration Debug -derivedDataPath build/xcode build
```

Use `-configuration Release` for an optimized build. Products are written to
`build/xcode/Build/Products/<configuration>/`. Builds are signed to run
locally. The applications depend only on system libraries.

### Game

The engine compiles with C++20, `-fms-extensions` for its
`__declspec(property)` declarations, and `-fno-fast-math -ffp-contract=off`
so its floating-point results match in every configuration. Debug defines
`_DEBUG`. `OpenTS.app` force-loads the engine and VQA libraries, because the
engine's classes register themselves from static initializers that nothing
else refers to. The `ui/` folder is copied into the bundle's `Resources`.

To install it, copy the Release build to Applications:

```sh
ditto build/xcode/Build/Products/Release/OpenTS.app /Applications/OpenTS.app
```

Opened from Finder with no data directory named, the game looks for
`TIBSUN.MIX` in the folder chosen on an earlier launch, then in
`~/Library/Application Support/OpenTS/Data`, the folder holding the application
bundle, and the executable's own folder. When none holds it, an Open panel asks
for the folder, and the answer is kept in
`~/Library/Application Support/OpenTS/data-folder.txt`; delete that file to be
asked again. The icon is `platform/macos/AppIcon.icns`, rendered from
`code/resources/app-icon/opents.svg`.

From Terminal, start the game with `open`, so it becomes the active application
and its window receives focus:

```sh
open build/xcode/Build/Products/Debug/OpenTS.app --args -XC
```

`-XC` mirrors the debug log to standard error. The log itself is written to
`~/Library/Logs/OpenTS`.

### Desktop shell

The shell creates a Cocoa window through SDL and presents a generated RGB565
diagnostic frame through the engine's bgfx frame presenter on Metal. The game,
assets, menus, and simulation are not loaded. The shell supports window
resizing, Retina drawable sizing, mouse capture during clicks, Escape to close,
and Command+Return to toggle desktop fullscreen. These input handlers are
development scaffolding for the game application.

For a bounded startup and presentation check:

```sh
build/xcode/Build/Products/Debug/OpenTSMacShell.app/Contents/MacOS/OpenTSMacShell --smoke-test
```

This opens a window, presents 60 frames, and closes. It exits with a failure
if renderer startup or presentation fails, if the selected backend is not
Metal, or if the loop cannot present 60 frames within 15 seconds. It needs an
active macOS desktop session.

### Engine tests

The `PortableTests` harnesses compile the engine sources they exercise and
need no game assets. They cover color conversion, VQA frame decoding, LCW
compression and block streams, audio rings, audio handles, audio levels,
priority queue ordering, shape facings, and Blowfish encryption. Tests with
floating-point contracts are compiled with `-fno-fast-math` and
`-ffp-contract=off`. Each harness's output is printed in the build log when
it fails.

The other harnesses under `tests/` are not yet in the Xcode project. Passing
these ten establishes behavior only for the components they exercise.

## Build identity

The project version is the SemVer string in the top-level `VERSION` file,
prerelease label included. It must match the development entry in the
manual's release registry; `python manual/tools/manage.py check` verifies
this.

The engine reads two generated headers built from that version and the
repository state:

| Header | Contents |
| --- | --- |
| `opents_version.h` | The version components, the version string, a prerelease flag, and the packed version number |
| `opents_build.h` | The commit, branch, commit date, whether tracked files were modified, and the version as it is displayed |

`platform/macos/generate_headers.py` writes them before the engine compiles,
together with `opents_strings.h`, the string-name table, and
`opents_language.h`, the text of the string tables in
`code/language/language.rc`. A header is rewritten only when its contents
change.

The packed version stores the major, minor, and patch components in one byte
each. Saves and network peers reject a different number. Builds within one
release cycle, including prereleases, share it, but their saves, replays, and
network sessions may still be incompatible.

The title screen, version dialog, crash report, and debug log banner read
these headers. A normal build shows the version and commit, such as
`0.1.0 (ab12cd3)`, plus a marker when tracked files are modified. The commit
identifies the build for diagnostics; it is not a save or network
compatibility stamp. An official build shows only its declared version.

Git is optional at build time once the complete source tree is present. Without
Git or repository metadata, the build records the commit as `unknown` and shows
the version without one.

## Continuous integration

There is no macOS continuous integration. Build and test locally with the
commands above.

## Verification boundary

A successful build verifies only that the project compiles, links, passes the
tests, and produces the listed files. Runtime behavior requires separate play
testing.

The repository contains no maps, movies, audio, or other original game assets.
Keep legally obtained runtime data local and outside version control. The
repository safety rules are in [CONTRIBUTING.md](../CONTRIBUTING.md).
