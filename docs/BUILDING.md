# Building OpenTS

> [!IMPORTANT]
> OpenTS targets macOS on Apple silicon only. The Xcode project currently
> builds a development shell and ten engine test harnesses; the full game does
> not yet compile or link. [Mac port status](MACOS_PORT.md) records what has
> been verified, and the [port TODO](../TODO.md) tracks the remaining work.

## Supported target

| Component | Requirement |
| --- | --- |
| Host and target | macOS on Apple silicon (`arm64`) |
| Minimum macOS | 13.0, provisional until [M8.4](../TODO.md#m8-macos-application-delivery) records the tested version |
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

miniaudio, RmlUi, FreeType, Dear ImGui, and LZO are not yet built by the Xcode
project; [M2.2](../TODO.md#m2-native-engine-build-and-platform-services) adds
them. Update a pinned tag in a separate change.

## Build with Xcode

Open `platform/macos/OpenTS.xcodeproj` in Xcode and choose a scheme:

| Scheme | Result |
| --- | --- |
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
locally. The application depends only on system libraries.

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

The Xcode project does not generate them yet; the engine target will. The
archived generators in `archive/windows/cmake/` show what they contain.

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

There is no macOS continuous integration yet;
[M8.3](../TODO.md#m8-macos-application-delivery) adds Debug and Release
builds of the engine and the asset-free tests.

## Verification boundary

A successful build verifies only that the project compiles, links, passes the
tests, and produces the listed files. Runtime behavior requires separate play
testing.

The repository contains no maps, movies, audio, or other original game assets.
Keep legally obtained runtime data local and outside version control. The
repository safety rules are in [CONTRIBUTING.md](../CONTRIBUTING.md).
