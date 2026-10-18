#!/usr/bin/env python3
"""FTL DS one-step setup: find your own copy of FTL, convert its assets, build the ROM.

This repository ships no FTL art, audio, text or data. Everything the DS build
needs is read from a copy of FTL: Faster Than Light that you own (Steam, GOG,
Humble or Epic; base game data, version 1.6.x) and converted on your machine.
The converted files land in ignored folders (generated/, nitrofiles/) and in
the ROM you build. Keep them to yourself: do not commit, upload or share them.

  python tools/setup_assets.py                    # auto-detect FTL, import, build
  python tools/setup_assets.py --ftl "D:/Games/FTL Faster Than Light"
  python tools/setup_assets.py --steam-download YOUR_STEAM_LOGIN
  python tools/setup_assets.py --no-build         # only convert assets
  python tools/setup_assets.py --find             # only show where FTL was found

The archive is opened read-only and never modified.
"""
import argparse
import hashlib
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / 'tools'
GENERATED = ROOT / 'generated'
STEAM_APP_ID = '212680'          # FTL: Faster Than Light on Steam.
STEAM_DIR_NAME = 'FTL Faster Than Light'

# Archives this project has been tested with. Others are tried, with a warning.
KNOWN_ARCHIVES = {
    'd7c4744f91f96303c0342b30c2dcb63843050e0f489426200c683d02fb8abc7d': 'FTL 1.6.x (Steam, Windows)',
}

# Entries every importer depends on; a copy missing any of these is not usable.
REQUIRED_ENTRIES = [
    'data/blueprints.xml', 'data/text_blueprints.xml', 'data/autoBlueprints.xml',
    'data/kestral.txt', 'data/kestral.xml', 'data/text_tutorial.xml',
    'img/ship/kestral_base.png', 'img/ship/kestral_floor.png',
    'img/window_base.png', 'audio/music/bp_MUS_TitleScreen.ogg',
]

# Import order matters: later steps read catalogs written by earlier ones.
STEPS = [
    ('Kestrel and base data', ['import_ftl.py', '{archive}']),
    ('Enemy weapon variants', ['import_enemy_weapons.py', '{archive}']),
    ('Weapons and augments', ['import_weapons.py', '--archive', '{archive}']),
    ('First enemy encounter', ['import_enemy.py']),
    ('Drones', ['drone_coverage.py', '{archive}']),
    ('Player ships (hangar)', ['import_player_ships.py', '{archive}']),
    ('Achievements', ['import_achievements.py', '{archive}']),
    ('Tutorial text', ['import_tutorial.py', '{archive}']),
    ('Events', ['event_compile.py', '{archive}']),
    ('Sector events', ['import_events.py']),
    ('Interface art', ['import_ui_art.py']),
    ('Ship art', ['convert_ship_art.py']),
    ('Rebel flagship', ['import_flagship.py', '{archive}']),
    ('System icons', ['import_system_icons.py', '--archive', '{archive}']),
    ('Star map art', ['import_map_art.py', '--archive', '{archive}']),
    ('Window frames', ['import_ui_frames.py', '--archive', '{archive}']),
    ('Music and sound', ['import_audio.py', '{archive}', '--ffmpeg', '{ffmpeg}']),
    ('Launcher icon', ['make_launcher_icon.py']),
]

# Headers the DS build includes; all must exist after import.
GENERATED_HEADERS = [
    'ftl_data.h', 'ship_art.h', 'combat_art.h', 'interior_art.h', 'enemy_data.h', 'enemy_catalog.h',
    'enemy_weapon_variants.h', 'weapon_properties.h', 'weapon_catalog.h', 'augment_catalog.h',
    'drone_catalog.h', 'player_ships.h', 'achievement_catalog.h', 'tutorial_text.h', 'species_art.h',
    'enemy_catalog_art.h', 'sector_events.h', 'event_catalog.h', 'ui_art.h', 'audio_assets.h',
    'flagship_data.h', 'flagship_art.h', 'system_icons.h', 'map_art.h', 'ftl_ui_art.h',
    'launcher_icon.gif',
]


def say(msg):
    print(msg, flush=True)


def fail(msg):
    say('\nERROR: ' + msg)
    sys.exit(1)


# ---------------------------------------------------------------- finding FTL
def archive_in(folder):
    """ftl.dat inside an FTL install folder (Windows, Linux or macOS layout)."""
    folder = Path(folder)
    for rel in ('ftl.dat', 'data/ftl.dat', 'FTL.app/Contents/Resources/ftl.dat', 'Contents/Resources/ftl.dat'):
        if (folder / rel).is_file():
            return folder / rel
    return None


def steam_roots():
    roots = []
    if os.name == 'nt':
        try:
            import winreg
            for hive, key in ((winreg.HKEY_CURRENT_USER, r'Software\Valve\Steam'),
                              (winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\WOW6432Node\Valve\Steam'),
                              (winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\Valve\Steam')):
                for value in ('SteamPath', 'InstallPath'):
                    try:
                        with winreg.OpenKey(hive, key) as k:
                            roots.append(Path(winreg.QueryValueEx(k, value)[0]))
                    except OSError:
                        pass
        except ImportError:
            pass
        for env in ('ProgramFiles(x86)', 'ProgramFiles'):
            if os.environ.get(env):
                roots.append(Path(os.environ[env]) / 'Steam')
    else:
        home = Path.home()
        roots += [home / '.steam/steam', home / '.local/share/Steam', home / '.var/app/com.valvesoftware.Steam/.local/share/Steam',
                  home / 'Library/Application Support/Steam']
    libraries = []
    for root in roots:
        if not root.is_dir():
            continue
        libraries.append(root)
        vdf = root / 'steamapps' / 'libraryfolders.vdf'
        if vdf.is_file():
            for path in re.findall(r'"path"\s+"([^"]+)"', vdf.read_text(encoding='utf-8', errors='replace')):
                libraries.append(Path(path.replace('\\\\', '\\')))
    seen, unique = set(), []
    for lib in libraries:
        key = str(lib).lower()
        if key not in seen:
            seen.add(key)
            unique.append(lib)
    return unique


def candidate_folders():
    for lib in steam_roots():
        yield lib / 'steamapps' / 'common' / STEAM_DIR_NAME
    home = Path.home()
    if os.name == 'nt':
        drives = [d + ':/' for d in 'CDEFGH' if Path(d + ':/').exists()]
        for d in drives:
            for rel in ('GOG Games/FTL Advanced Edition', 'GOG Games/FTL', 'GOG Galaxy/Games/FTL Advanced Edition',
                        'Program Files (x86)/GOG Galaxy/Games/FTL Advanced Edition', 'Program Files (x86)/GOG.com/FTL',
                        'Program Files/Epic Games/FTL', 'Program Files/Epic Games/FTLFasterThanLight',
                        'Program Files (x86)/FTL', 'Program Files/FTL', 'Games/FTL', 'FTL'):
                yield Path(d) / rel
    elif platform.system() == 'Darwin':
        yield Path('/Applications/FTL.app')
        yield home / 'Applications/FTL.app'
        yield home / 'Library/Application Support/GOG.com/Galaxy/Applications/FTL.app'
    else:
        yield home / 'GOG Games/FTL Advanced Edition/game'
        yield home / 'GOG Games/FTL/game'
        yield home / 'Games/ftl-advanced-edition/drive_c/GOG Games/FTL Advanced Edition'
        yield home / 'FTL'


def find_archives():
    found = []
    for folder in candidate_folders():
        try:
            archive = archive_in(folder)
        except OSError:
            archive = None
        if archive and archive.resolve() not in [a.resolve() for a in found]:
            found.append(archive)
    return found


def steam_download(user):
    """Download your own FTL through SteamCMD (requires owning it on that account)."""
    steamcmd = shutil.which('steamcmd') or shutil.which('steamcmd.exe') or shutil.which('steamcmd.sh')
    if not steamcmd:
        fail('SteamCMD was not found. Install it (https://developer.valvesoftware.com/wiki/SteamCMD) and make '
             'sure "steamcmd" is on your PATH, or install FTL with the normal Steam client and rerun without '
             '--steam-download.')
    target = ROOT / '.ftl-install'
    target.mkdir(exist_ok=True)
    say('Downloading FTL with SteamCMD into %s.\nSteam will ask for your password and Steam Guard code; '
        'they go straight to Steam, never to this script.' % target)
    cmd = [steamcmd, '+@sSteamCmdForcePlatformType', 'windows', '+force_install_dir', str(target),
           '+login', user, '+app_update', STEAM_APP_ID, 'validate', '+quit']
    if subprocess.call(cmd) != 0 or not archive_in(target):
        fail('SteamCMD did not produce ftl.dat. Check that this Steam account owns FTL: Faster Than Light.')
    return archive_in(target)


# ---------------------------------------------------------------- verification
def verify(archive):
    sys.path.insert(0, str(TOOLS))
    from import_ftl import PkgArchive
    try:
        pkg = PkgArchive(archive)
    except (ValueError, OSError) as e:
        fail('%s is not a usable FTL 1.6 data archive (%s).\nFTL 1.5 and older (data.dat + resource.dat) are '
             'not supported; update the game to 1.6.x through Steam/GOG.' % (archive, e))
    missing = [e for e in REQUIRED_ENTRIES if e not in pkg.entries]
    if missing:
        fail('%s is missing expected FTL files: %s' % (archive, ', '.join(missing)))
    digest = hashlib.sha256()
    with open(archive, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            digest.update(block)
    sha = digest.hexdigest()
    label = KNOWN_ARCHIVES.get(sha)
    if label:
        say('Verified %s (%d entries): %s' % (archive, len(pkg.entries), label))
    else:
        say('Found %s (%d entries), sha256 %s.\nThis exact version has not been tested; continuing. If an import '
            'step fails, please open an issue with this hash.' % (archive, len(pkg.entries), sha))
    return sha


# ---------------------------------------------------------------- import + build
def find_ffmpeg(explicit):
    ffmpeg = explicit or shutil.which('ffmpeg')
    if not ffmpeg:
        hint = ('winget install Gyan.FFmpeg' if os.name == 'nt' else
                'brew install ffmpeg' if platform.system() == 'Darwin' else 'sudo apt install ffmpeg')
        fail('ffmpeg is needed to convert the music and sound effects. Install it (for example: %s), '
             'or pass --ffmpeg PATH.' % hint)
    return ffmpeg


def check_python():
    if sys.version_info < (3, 9):
        fail('Python 3.9 or newer is required.')
    try:
        import PIL  # noqa: F401
    except ImportError:
        fail('Pillow is required: run  %s -m pip install --user Pillow' % Path(sys.executable).name)


def run_imports(archive, ffmpeg):
    GENERATED.mkdir(exist_ok=True)
    total = len(STEPS)
    for i, (label, cmd) in enumerate(STEPS, 1):
        args = [a.format(archive=str(archive), ffmpeg=ffmpeg or '') for a in cmd]
        say('[%2d/%d] %s' % (i, total, label))
        result = subprocess.run([sys.executable, str(TOOLS / args[0])] + args[1:], cwd=str(ROOT),
                                capture_output=True, text=True)
        if result.returncode != 0:
            sys.stdout.write(result.stdout)
            sys.stderr.write(result.stderr)
            fail('Import step "%s" failed (see output above).' % label)
    missing = [h for h in GENERATED_HEADERS if not (GENERATED / h).is_file()]
    if missing:
        fail('Import finished but these generated files are missing: ' + ', '.join(missing))
    say('Assets converted into generated/ and nitrofiles/ (both are git-ignored).')


def build_rom():
    rom = ROOT / 'ftl-ds.nds'
    blocksds = os.environ.get('BLOCKSDS')
    wonderful = Path('/opt/wonderful/thirdparty/blocksds/core')
    msys_bash = None
    if os.name == 'nt':
        # Wonderful/BlocksDS on Windows lives inside an MSYS2 install (portable .tools/ or C:/msys64).
        for msys in (ROOT / '.tools' / 'msys64', Path(os.environ.get('MSYS2_ROOT', 'C:/msys64'))):
            if (msys / 'usr/bin/bash.exe').is_file() and (msys / 'opt/wonderful/bin/wf-env').is_file():
                msys_bash = msys / 'usr/bin/bash.exe'
                break
    if msys_bash:
        say('Building with BlocksDS (%s) ...' % msys_bash.parents[2])
        env = dict(os.environ, MSYSTEM='UCRT64', CHERE_INVOKING='1')
        code = subprocess.call([str(msys_bash), '-lc', 'source /opt/wonderful/bin/wf-env && make -j2'], cwd=str(ROOT), env=env)
    elif (blocksds and Path(blocksds).is_dir()) or wonderful.is_dir():
        say('Building with BlocksDS ...')
        env = dict(os.environ)
        if Path('/opt/wonderful/bin/wf-env').is_file() and shutil.which('bash'):
            code = subprocess.call(['bash', '-lc', 'source /opt/wonderful/bin/wf-env && make -j2'], cwd=str(ROOT), env=env)
        else:
            code = subprocess.call(['make', '-j2'], cwd=str(ROOT), env=env)
    elif shutil.which('docker') and subprocess.call(['docker', 'info'], stdout=subprocess.DEVNULL,
                                                    stderr=subprocess.DEVNULL) == 0:
        say('Building inside the BlocksDS Docker image (first run downloads it) ...')
        code = subprocess.call(['docker', 'run', '--rm', '-v', '%s:/work' % ROOT, '-w', '/work',
                                'skylyrac/blocksds:slim-latest', 'make', '-j2'])
    else:
        if shutil.which('docker'):
            say('\nDocker is installed but not running. Start Docker (Docker Desktop on Windows/macOS), then run\n'
                'this again with --build-only.')
            return False
        say('\nAssets are ready, but no Nintendo DS toolchain was found, so the ROM was not built.\n'
            'Install BlocksDS (https://blocksds.skylyrac.net/docs/setup/) or Docker, then run this again\n'
            'with --build-only. See README.md.')
        return False
    if code != 0 or not rom.is_file():
        fail('The DS build failed. See the compiler output above.')
    say('\nDone: %s\nCopy it to your flashcart or open it in melonDS. It contains converted FTL assets: '
        'keep it for yourself and do not share it.' % rom)
    return True


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--ftl', type=Path, help='path to ftl.dat or to the FTL install folder')
    ap.add_argument('--steam-download', metavar='STEAM_LOGIN',
                    help='download your own copy with SteamCMD (the account must own FTL)')
    ap.add_argument('--ffmpeg', help='path to ffmpeg (default: found on PATH)')
    ap.add_argument('--no-build', action='store_true', help='convert assets only; do not build the ROM')
    ap.add_argument('--build-only', action='store_true', help='skip importing; build from existing generated/')
    ap.add_argument('--find', action='store_true', help='list FTL copies found on this computer and exit')
    args = ap.parse_args()

    if args.find:
        found = find_archives()
        say('\n'.join(str(a) for a in found) if found else 'No FTL install found automatically. Use --ftl PATH.')
        return
    if args.build_only:
        missing = [h for h in GENERATED_HEADERS if not (GENERATED / h).is_file()]
        if missing:
            fail('Converted assets are missing (%s). Run without --build-only first.' % ', '.join(missing[:4]))
        build_rom()
        return

    check_python()
    if args.steam_download:
        archive = steam_download(args.steam_download)
    elif args.ftl:
        p = args.ftl.expanduser()
        archive = p if p.is_file() else archive_in(p)
        if not archive:
            fail('No ftl.dat found at %s.' % p)
    else:
        found = find_archives()
        if not found:
            fail('Could not find FTL automatically. Install FTL: Faster Than Light (Steam, GOG, Humble or Epic) '
                 'and run again, or point to it:\n  python tools/setup_assets.py --ftl "PATH/TO/FTL"\n'
                 'or download your own copy with SteamCMD:\n  python tools/setup_assets.py --steam-download STEAM_LOGIN')
        archive = found[0]
        if len(found) > 1:
            say('Several FTL copies found; using the first (choose another with --ftl):\n  ' + '\n  '.join(map(str, found)))
    archive = archive.resolve()
    if ROOT in archive.parents and '.ftl-install' not in archive.parts:
        fail('Keep FTL outside this repository folder so it can never be committed by accident.')
    verify(archive)
    ffmpeg = find_ffmpeg(args.ffmpeg)
    run_imports(archive, ffmpeg)
    if not args.no_build:
        build_rom()


if __name__ == '__main__':
    main()
