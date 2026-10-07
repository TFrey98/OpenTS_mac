<p align="center">
  <img src="https://raw.githubusercontent.com/OpenTS-Developers/.github/main/assets/opents-logo.png" alt="OpenTS" width="512">
</p>

<p align="center">
  <a href="https://opents-developers.github.io/OpenTS/"><img src="https://github.com/OpenTS-Developers/OpenTS/actions/workflows/manual-pages.yml/badge.svg" alt="Manual"></a>
  <a href="https://www.patreon.com/c/ZivDero"><img src="https://img.shields.io/badge/Patreon-ZivDero-F96854?logo=patreon&logoColor=white" alt="Patreon"></a>
</p>

# OpenTS

This fork ports OpenTS to macOS on Apple silicon, its only target. The
single-player game runs natively, campaigns and cinematics included, from the
original game data. See [Mac port status](docs/MACOS_PORT.md) and
[build instructions](docs/BUILDING.md).

OpenTS is a community-led, open-source reconstruction of *Command & Conquer:
Tiberian Sun*. Instead of patching or extending the retail executable, it
rebuilds the engine as a standalone program.

OpenTS gives equal weight to two goals: maintaining a playable engine and
providing a capable platform for modding and engine development. Work on one
goal should not come at the expense of the other.

OpenTS is:

- an independent, community-led source reconstruction targeting Tiberian Sun
  2.03 Firestorm;
- a playable engine based on Electronic Arts' GPL-released source for related
  Command & Conquer games and Tiberian Sun-specific reverse engineering; and
- the active base for maintenance, documentation, modernization, bug fixes,
  and new modding capabilities.

OpenTS is not:

- a remaster or remake;
- an official Electronic Arts source release; or
- a distribution of the original game assets.

OpenTS is an independent community project and is not affiliated with or
endorsed by Electronic Arts.

## Community

- Discord: <https://opents.net/discord>
- Bug reports and proposals:
  [GitHub issues](https://github.com/OpenTS-Developers/OpenTS/issues)

## Playing

OpenTS supplies the engine, not the game data. Tiberian Sun's original assets
come from a legally obtained copy of the game; Electronic Arts released it as
freeware. Copy the game's `.MIX` archives into a folder, such as `Run/`, then
build the game and start it with that folder:

```sh
open build/xcode/Build/Products/Debug/OpenTS.app --args -DATADIR="$PWD/Run"
```

[Mac port status](docs/MACOS_PORT.md#game-data) lists the archives each disc
provides.

## Documentation

The [OpenTS manual](https://opents-developers.github.io/OpenTS/) documents
setup, runtime behavior, INI configuration, mapping, and source-level
internals.

## State and plans

This fork's single-player game runs on macOS; the known gaps are listed in
[Mac port status](docs/MACOS_PORT.md#known-gaps). The engine it starts from, upstream
OpenTS 0.1.0, ran the full Tiberian Sun 2.03 Firestorm game on Windows, with
its campaigns, skirmish, and save/load play-tested. The renderer uses
[bgfx](https://github.com/bkaradzic/bgfx) and supports modern resolutions
through 4K, including ultrawide.

Upstream OpenTS has three development milestones:

1. CnCNet and CnCNet client support, including porting the parts of
   [ts-patches](https://github.com/CnCNet/ts-patches) this requires.
2. Feature parity with
   [Vinifera](https://github.com/Vinifera-Developers/Vinifera) and the rest
   of ts-patches.
3. Extending Tiberian Sun with new features, striving toward feature parity
   with Red Alert 2 and Yuri's Revenge, and growing engine capabilities that
   match or exceed the popular Yuri's Revenge engine extensions.

Alongside these goals, the engine is modernized incrementally toward an
entity-component architecture, and new development is shaped so that
migration stays possible. [Project direction](docs/DIRECTION.md) explains the
reasoning.

## Building

This fork builds for macOS on Apple silicon with Xcode.
[Building OpenTS](docs/BUILDING.md) documents the exact requirements,
commands, and outputs.

## Contributing

Bug reports, proposals, documentation improvements, and focused pull requests
are welcome. Review capacity is limited, so discuss non-trivial work with the
maintainers before implementing it. [CONTRIBUTING.md](CONTRIBUTING.md)
explains the current priorities and review policy; source conventions are in
[Style](docs/STYLE.md).

## Origins

OpenTS continues the community reconstruction preserved in the
[TibSun archive](https://github.com/OpenTS-Developers/TibSun), built from
Electronic Arts' published source for related Command & Conquer games and
completed through reverse engineering against the original executable.
[History](docs/HISTORY.md) records the reconstruction's lineage and methods.

## License and acknowledgements

OpenTS is licensed under the GNU General Public License, version 3 or later.
Material derived from Electronic Arts source remains subject to the additional
GPL Section 7 terms in [LICENSE.md](LICENSE.md).
[Third-party notices](THIRD_PARTY_NOTICES.md) identify bundled dependencies
and their licenses.

[ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) thanks the people, projects, and
communities whose work made OpenTS possible.
