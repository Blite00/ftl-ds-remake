#!/usr/bin/env python3
"""Cross-platform host test runner (same checks as tools/check.ps1).

Compiles each tests/*.c smoke test with the host C compiler against source/ and
the locally converted generated/ headers, then runs it in a disposable folder.
Run tools/setup_assets.py first so generated/ exists.

  python tools/check.py            # quick checks
  python tools/check.py --full     # also the long scripted campaign runs
  python tools/check.py NAME ...   # only the named checks
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PS1 = ROOT / 'tools' / 'check.ps1'


def load_checks():
    text = PS1.read_text(encoding='utf-8')
    long_runs = re.findall(r"'([a-z0-9_]+)'", re.search(r'\$longRuns = @\(([^)]*)\)', text).group(1))
    checks = []
    for m in re.finditer(r"@\{ Name='([^']+)'; Sources=@\(([^)]*)\)([^}]*)\}", text):
        checks.append(dict(name=m.group(1), sources=re.findall(r"'([^']+)'", m.group(2)),
                           log='Log=$true' in m.group(3), optional='Optional=$true' in m.group(3)))
    return checks, set(long_runs)


def sources_for(check):
    s = list(check['sources'])
    extra = []
    if 'game' in s:
        extra += ['fire', 'achievements', 'boarding', 'breach', 'subsystems', 'species', 'beam', 'drones']
    if 'campaign' in s:
        extra += ['outcome', 'rewards']
    if 'render' in s and 'menu' not in s:
        extra += ['menu']
    if 'render' in s:
        extra += ['setup', 'shot_path', 'player_art', 'hangar', 'profile']
        if 'tutorial' not in s:
            extra += ['tutorial']
    return [ROOT / 'source' / (n + '.c') for n in s + extra]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('names', nargs='*')
    ap.add_argument('--full', action='store_true')
    ap.add_argument('--cc', default=os.environ.get('CC') or shutil.which('gcc') or shutil.which('cc'))
    args = ap.parse_args()
    if not (ROOT / 'generated' / 'event_catalog.h').is_file():
        sys.exit('generated/ is missing: run  python tools/setup_assets.py --no-build  first.')
    if not args.cc:
        sys.exit('No host C compiler found (gcc or cc). Set CC or pass --cc.')
    for py in ('enemy_weapon_import_test.py', 'event_compile_test.py'):
        if subprocess.call([sys.executable, str(ROOT / 'tests' / py)]) != 0:
            sys.exit('Python importer test failed: ' + py)
    checks, long_runs = load_checks()
    if args.names:
        checks = [c for c in checks if c['name'] in args.names]
    run_dir = Path(tempfile.mkdtemp(prefix='check-', dir=str(ROOT / 'build') if (ROOT / 'build').is_dir() else None))
    (run_dir / 'tests').mkdir()
    passed = 0
    for c in checks:
        if not args.full and not args.names and c['name'] in long_runs:
            continue
        test = ROOT / 'tests' / (c['name'] + '.c')
        if not test.is_file():
            if c['optional']:
                print('SKIP (not present):', c['name'])
                continue
            sys.exit('Required test missing: %s' % test)
        exe = run_dir / (c['name'] + ('.exe' if os.name == 'nt' else ''))
        cmd = [args.cc, '-std=c99', '-Wall', '-Wextra', '-Werror', '-I' + str(ROOT / 'source'),
               '-I' + str(ROOT / 'generated'), str(test)] + [str(p) for p in sources_for(c)] + ['-o', str(exe), '-lm']
        if subprocess.call(cmd) != 0:
            sys.exit('Compilation failed: ' + c['name'])
        run = [str(exe)] + ([str(run_dir / 'logging-smoke.log')] if c['log'] else [])
        if subprocess.call(run, cwd=str(run_dir)) != 0:
            sys.exit('Test failed: ' + c['name'])
        passed += 1
        print('PASS:', c['name'], flush=True)
    print('Passed %d host checks. Disposable artifacts: %s' % (passed, run_dir))


if __name__ == '__main__':
    main()
