# FTL DS — an unofficial Nintendo DS demake

A from-scratch Nintendo DS engine that plays the base game of **FTL: Faster Than Light** on a Nintendo DS (homebrew flashcart) or in a DS emulator: all nine cruisers in Type A and B, the hangar and original unlocks, 36 weapons, drones, cloaking, augments, boarding, hazards, quests, the hidden Crystal sector, the tutorial, achievements and the three-stage Rebel flagship.

**This repository contains no FTL assets.** No art, music, sound, text, ship layouts or game data from FTL is stored here. You need your own copy of FTL; a setup script reads it on your computer, converts what the DS needs, and builds the ROM locally.

> Unofficial fan project. Not affiliated with, endorsed by or supported by Subset Games. *FTL: Faster Than Light* is © Subset Games; all FTL names and content belong to them. Buy the game: [Steam](https://store.steampowered.com/app/212680/), [GOG](https://www.gog.com/game/faster_than_light), [Humble](https://www.humblebundle.com/store/ftl-faster-than-light).

## What you need

| | |
|---|---|
| **FTL: Faster Than Light** | Your own copy, version 1.6.x (current Steam, GOG, Humble or Epic release). The setup only reads `ftl.dat`; it never changes your game. |
| **Python 3.9+** with **Pillow** | [python.org](https://www.python.org/downloads/). The setup installs Pillow if it is missing. |
| **ffmpeg** | Converts the music and sound. Windows: `winget install Gyan.FFmpeg` · macOS: `brew install ffmpeg` · Linux: `sudo apt install ffmpeg` |
| **A DS toolchain** (to build the ROM) | [BlocksDS](https://blocksds.skylyrac.net/docs/setup/) **or** [Docker](https://www.docker.com/) (the setup then uses the `skylyrac/blocksds` image automatically). |

## Quick start

1. Install FTL from Steam/GOG/etc. as normal, and the tools above.
2. Download this repository (green **Code** button → *Download ZIP*, or `git clone`).
3. Run the setup:
   - **Windows:** double-click `setup.bat`
   - **macOS / Linux:** `./setup.sh`
4. When it finishes you have **`ftl-ds.nds`**. Copy it to your flashcart's microSD card or open it in [melonDS](https://melonds.kuribo64.net/).

The setup finds FTL automatically in Steam libraries (including extra library folders) and common GOG, Humble and Epic install locations. If yours is elsewhere:

```sh
python tools/setup_assets.py --ftl "D:/Games/FTL Faster Than Light"     # folder or ftl.dat
```

No FTL install on this computer? If you own FTL on Steam, the setup can download your copy with Valve's [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD) (you log in to Steam yourself; ownership is checked by Steam):

```sh
python tools/setup_assets.py --steam-download YOUR_STEAM_LOGIN
```

Other options: `--find` lists the copies it can see, `--no-build` only converts assets, `--build-only` rebuilds the ROM from already converted assets.

## What the setup does

1. **Finds and checks your copy.** It opens `ftl.dat` read-only, checks that it is a genuine FTL 1.6 data archive with the files this project needs, and reports whether that exact version has been tested.
2. **Converts assets locally.** 18 import steps read ship layouts, blueprints, events, tutorial text, art and audio from your archive and write DS formats (RGB555 bitmaps, PCM audio, C headers) into `generated/` and `nitrofiles/`. Both folders are git-ignored.
3. **Builds the ROM.** BlocksDS compiles the engine in `source/` together with your converted assets into `ftl-ds.nds`.

The engine code, the importers and the tests are original work in this repository. The converted assets and the ROM contain Subset Games' content, so **keep them for yourself: do not upload, share or distribute `ftl-ds.nds`, `generated/` or `nitrofiles/`.** Point people to this repository instead; they build their own ROM from their own copy.

## Playing

- **Flashcart:** copy `ftl-ds.nds` anywhere on the microSD card. DLDI must be enabled (most kernels do this automatically). Saves go to the card root: `/ftl-ds-campaign.sav` (current run, with `.bak`), `/ftl-ds-profile.dat` (hangar unlocks and achievements).
- **melonDS:** enable DLDI/SD card emulation so the game can save. See [docs/EMULATOR.md](docs/EMULATOR.md).
- Controls and menus: [docs/PLAYING.md](docs/PLAYING.md) and [docs/CONTROLS.md](docs/CONTROLS.md). Touch works everywhere; buttons work on almost every screen.

## For developers

```sh
python tools/setup_assets.py --no-build   # convert assets once
python tools/check.py                     # 137 host tests (gcc/clang); --full adds long campaign runs
make                                      # with BLOCKSDS set, or use --build-only
```

Windows users with the portable toolchain in `.tools/` can keep using `build.ps1` and `tools/check.ps1`.

### Keep the repository clean

Before every commit, run the audit. It fails on any ROM, image, audio, converted file, or on any sentence of game text:

```sh
python tools/audit_clean.py --install-hook          # once: checks every commit automatically
python tools/audit_clean.py --ftl PATH/TO/ftl.dat   # full check against your copy (do this before a release)
```

CI runs the file rules on every push and pull request.

First push of a fresh copy:

```sh
git init
python tools/audit_clean.py --install-hook
python tools/audit_clean.py --ftl PATH/TO/ftl.dat
git add . && git commit -m "Initial commit"
gh repo create ftl-ds --public --source . --push      # or create the repo on github.com and push
``` Never paste FTL text, art or numbers tables into issues, docs or code; describe them, or reference them by their blueprint name (for example `LASER_BURST_2`) so the importer reads them from the player's own archive.

## Project layout

| Path | Contents |
|---|---|
| `source/` | DS engine (C, BlocksDS/libnds + maxmod) |
| `tools/` | Importers (`import_*.py`, `event_compile.py`, …), `setup_assets.py`, `check.py`, `audit_clean.py` |
| `tests/` | Host smoke tests and importer tests |
| `docs/` | Design notes, controls, test checklist, per-version checkpoints ([CHANGELOG](docs/CHANGELOG.md)) |
| `generated/`, `nitrofiles/` | *Created by the setup from your FTL copy; git-ignored* |

## Status

Every base-game cruiser is playable and unlockable the original way. Physical-hardware testing is ongoing; see [docs/DS_TEST_CHECKLIST.md](docs/DS_TEST_CHECKLIST.md) and the [roadmap](docs/ROADMAP.md). Advanced Edition content is out of scope.

## License

The code in this repository is released under the MIT License ([LICENSE](LICENSE)). That license covers only this repository's own code and documentation. It grants no rights to FTL or to anything the setup converts from it; see [NOTICE.md](NOTICE.md).
