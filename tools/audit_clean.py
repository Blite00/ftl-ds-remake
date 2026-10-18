#!/usr/bin/env python3
"""Check that the repository contains no FTL assets before you commit or push.

1. File rules (always): no ROMs, archives, images, audio, converted binaries,
   generated/ or nitrofiles/ content among the files git would publish.
2. Content check (with --ftl): every file is compared against your own ftl.dat.
   Any byte-identical archive file, or any sentence of 40+ characters from the
   game's text (events, blueprints, tutorial, achievements), fails the audit.

  python tools/audit_clean.py                  # file rules only (also run in CI)
  python tools/audit_clean.py --ftl PATH       # full check against your copy
  python tools/audit_clean.py --install-hook   # run file rules before every commit

Exit status 0 means clean.
"""
import argparse
import hashlib
import html
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN_DIRS = ('generated/', 'nitrofiles/', '.ftl-install/', 'releases/', 'checkpoints/', '.tools/')
FORBIDDEN_EXT = {'.nds', '.dat', '.png', '.gif', '.bmp', '.jpg', '.jpeg', '.ogg', '.wav', '.mp3', '.pcm',
                 '.bin', '.sav', '.zip', '.7z', '.xz', '.gz', '.elf', '.exe', '.dll', '.ttf', '.font'}
MIN_SENTENCE = 40


def repo_files():
    """Files git would publish (tracked + untracked, minus .gitignore); falls back to a walk."""
    try:
        out = subprocess.run(['git', 'ls-files', '-co', '--exclude-standard', '-z'], cwd=str(ROOT),
                             capture_output=True, check=True).stdout
        return sorted(p for p in out.decode('utf-8').split('\0') if p)
    except (OSError, subprocess.CalledProcessError):
        skip = ('.git/', 'build/') + FORBIDDEN_DIRS
        files = []
        for p in ROOT.rglob('*'):
            rel = p.relative_to(ROOT).as_posix()
            if p.is_file() and not rel.startswith(skip) and '__pycache__' not in rel and not rel.endswith('.nds'):
                files.append(rel)
        return sorted(files)


def file_rules(files):
    problems = []
    for rel in files:
        if rel.startswith(FORBIDDEN_DIRS):
            problems.append('%s: converted/private folder must stay local' % rel)
        elif Path(rel).suffix.lower() in FORBIDDEN_EXT:
            problems.append('%s: %s files are not allowed (could carry game assets)' % (rel, Path(rel).suffix))
    return problems


def normalize(s):
    return re.sub(r'\s+', ' ', html.unescape(s).replace('\\n', ' ')).strip().lower()


def sentences_from_archive(pkg):
    found = set()
    for name in pkg.entries:
        if not (name.endswith('.xml') or name.endswith('.txt')):
            continue
        text = pkg.read(name).decode('utf-8', errors='replace')
        for chunk in re.findall(r'>([^<>]{%d,})<' % MIN_SENTENCE, text) + re.findall(r'"([^"<>]{%d,})"' % MIN_SENTENCE, text):
            for part in re.split(r'(?<=[.!?])\s+|\\n', chunk):
                part = normalize(part)
                if len(part) >= MIN_SENTENCE and ' ' in part:
                    found.add(part)
    return found


def content_check(files, archive):
    sys.path.insert(0, str(ROOT / 'tools'))
    from import_ftl import PkgArchive
    pkg = PkgArchive(archive)
    hashes = {}
    for name, (_, size, unpacked, _) in pkg.entries.items():
        hashes.setdefault(unpacked, []).append(name)
    sentences = sentences_from_archive(pkg)
    print('Comparing %d files against %d archive entries and %d game sentences...' % (len(files), len(pkg.entries), len(sentences)))
    by_len = {}
    for s in sentences:
        by_len.setdefault(s[:24], []).append(s)
    problems = []
    for rel in files:
        path = ROOT / rel
        data = path.read_bytes()
        if len(data) in hashes:
            digest = hashlib.sha256(data).hexdigest()
            for name in hashes[len(data)]:
                if hashlib.sha256(pkg.read(name)).hexdigest() == digest:
                    problems.append('%s: identical to archive file %s' % (rel, name))
        text = normalize(data.decode('utf-8', errors='ignore'))
        hits = 0
        for i in range(len(text) - 23):
            for s in by_len.get(text[i:i + 24], ()):
                if text.startswith(s, i):
                    if hits < 3:
                        problems.append('%s: contains game text "%s..."' % (rel, s[:60]))
                    hits += 1
        if hits > 3:
            problems.append('%s: ...and %d more game-text matches' % (rel, hits - 3))
    return problems


def install_hook():
    hook = ROOT / '.git' / 'hooks' / 'pre-commit'
    if not hook.parent.is_dir():
        sys.exit('No .git folder: run "git init" first.')
    hook.write_text('#!/bin/sh\nexec python3 tools/audit_clean.py || exec python tools/audit_clean.py\n', newline='\n')
    hook.chmod(0o755)
    print('Installed', hook)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--ftl', type=Path, help='your ftl.dat, for the full content comparison')
    ap.add_argument('--install-hook', action='store_true')
    args = ap.parse_args()
    if args.install_hook:
        install_hook()
        return
    files = repo_files()
    problems = file_rules(files)
    if args.ftl:
        problems += content_check([f for f in files if (ROOT / f).is_file()], args.ftl)
    if problems:
        print('NOT CLEAN - %d problem(s):' % len(problems))
        print('\n'.join('  ' + p for p in problems))
        sys.exit(1)
    print('Clean: %d files checked%s.' % (len(files), ' against ' + str(args.ftl) if args.ftl else ' (file rules)'))


if __name__ == '__main__':
    main()
