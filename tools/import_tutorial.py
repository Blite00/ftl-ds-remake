#!/usr/bin/env python3
"""Import the original FTL tutorial prompts for the DS tutorial (source/tutorial.c).

Reads data/text_tutorial.xml from the owner's archive (read-only, through
import_ftl.PkgArchive) and writes generated/tutorial_text.h. Each DS prompt names
the original text id it comes from. Wording is kept, except for explicit,
checked adaptations listed below (DS controls instead of mouse/iPad gestures,
DS screen locations, DS rule differences and trims so a prompt fits the
instruction panel: TUTORIAL_TEXT_COLS columns x TUTORIAL_TEXT_LINES lines).
Every adaptation is an exact (old, new) replacement that must match once, so a
changed source file fails loudly instead of silently drifting. Original "\\n\\n"
paragraph breaks become one DS line break. Output stays local (not redistributed).
"""
import argparse
import hashlib
import html
import json
import re
from pathlib import Path
from import_ftl import PkgArchive

COLS = 39   # 6x8 font across the 248-pixel top-screen panel.
LINES = 6
FONT = set('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 \n+-/:.%!>,?\'"();=[]&_*<')

# (DS key, original id, adaptations as (sha256 prefix of original span, span
# length, DS replacement)). Order is the DS step order; tutorial.c maps
# its steps to these TT_* indices.
STEPS = [
    ('SHIP_INTRO_1', 'tutorial_SHIP_INTRO_1', [
        ('96bbbc69c5d606e6ba30', 35, 'the focus of the game view on the touch screen.'),
    ]),
    ('SHIP_INTRO_2', 'tutorial_SHIP_INTRO_2', [
        ('0aeb0abdf5990bf49500', 46, 'The icons and bars at the bottom of the touch screen'),
        ('afb47b8557e6a7ac511e', 26, '(P used/total, top right).'),
    ]),
    ('SHIP_INTRO_3', 'tutorial_SHIP_INTRO_3', [
        ('dc1a29504558c388aac2', 33, "DOORS and SENSORS are your ship's subsystems; tap DOORS to see their rooms (DOR, SNS)."),
    ]),
    ('SHIP_INTRO_5', 'tutorial_SHIP_INTRO_5', [
        ('a20af2138f61707921cc', 71, 'Hull, oxygen, power (P) and missiles (M) are at the top of the touch screen; shields (SH) are on the top screen.'),
    ]),
    ('POWER_SYSTEM_1', 'tutorial_POWER_SYSTEM_1', []),
    ('POWER_SYSTEM_2', 'tutorial_POWER_SYSTEM_2_PC', [
        ('6f5c6c9833cb56f0fa69', 36, 'TAP + beside the engine system (ENG)'),
        ('5e3baadf98849f4aa583', 22, 'TAP - to depower'),
    ]),
    ('DAMAGE_SYSTEM_1', 'tutorial_DAMAGE_SYSTEM_1', []),
    ('DAMAGE_SYSTEM_2', 'tutorial_DAMAGE_SYSTEM_2_IPAD', [
        ('3ac90a8cbf26b4d0cb20', 83, 'Select a crewmember by TAPPING their portrait below the ship (or with L/R).'),
    ]),
    ('DAMAGE_SYSTEM_3', 'tutorial_DAMAGE_SYSTEM_3_IPAD', [
        ('cc316d64453ea019b3f7', 10, 'TAP a room (or D-PAD, then A)'),
    ]),
    ('DAMAGE_SYSTEM_4', 'tutorial_DAMAGE_SYSTEM_4', []),
    # DS rule: repaired systems do not repower themselves (game.c asks the player).
    ('DAMAGE_SYSTEM_5', 'tutorial_DAMAGE_SYSTEM_5', [
        ('7f0c52b6f329b66c5a86', 100, 'On the DS, repaired systems stay unpowered: give your ENGINES their power back.'),
    ]),
    # The DS has no manning silhouette above the power bars.
    ('DAMAGE_SYSTEM_5_5', 'tutorial_DAMAGE_SYSTEM_5_5', [
        ('9fe8dc0514edee9718ca', 67, ''),
    ]),
    # The iPad pair (open the door control, then a door) merged into one DS prompt.
    ('FIRE_1', 'tutorial_FIRE_IPAD_1_IPAD', [
        ('7aee6475436bc25d6544', 45, 'You could send crew to put it out'),
        ('a1e2a36e1ce4b91406ef', 72, 'TAP DOORS, then TAP a gold airlock door to open it (or START twice, L/R and A).'),
    ]),
    # DS floors redden (not pink) as oxygen drains.
    ('FIRE_END', 'tutorial_FIRE_END', [
        ('65ee2d9dda1b8e16b67b', 15, 'Rooms turn red'),
        ('ba46e85fbfc761dd5080', 68, 'Then close the airlock: your oxygen system refills the ship.'),
    ]),
    ('DAMAGE_SYSTEM_6', 'tutorial_DAMAGE_SYSTEM_6', []),
    ('DAMAGE_SYSTEM_7', 'tutorial_DAMAGE_SYSTEM_7_IPAD', [
        ('cc5b1c0872a07786a220', 53, 'Without fuel you will be stranded.'),
        ('6ed37ba80ba967cc92a8', 32, 'Tap JUMP (or press A) to continue.'),
    ]),
    ('STAR_MAP_1', 'tutorial_STAR_MAP_1_IPAD', [
        ('ef2248f903cd357f26f7', 49, ''),
        ('0303209002960818022e', 40, 'Select an adjacent location (TAP or D-PAD) and tap JUMP (A)'),
    ]),
    ('COMBAT_0', 'tutorial_COMBAT_0_IPAD', [
        ('83052d6705d665119362', 20, 'Tap PAUSE (or press X)'),
    ]),
    # DS-authored shields step: keeps the original note from COMBAT_3 and asks the
    # player to power the shields (the original Kestrel starts with them powered).
    ('SHIELDS', 'tutorial_COMBAT_3', [
        ('e1e29ff5447ef3e437ec', 125, 'The enemy is charging its weapon, but your shields have no power! Power your SHIELDS fully with + (or START).'),
    ]),
    ('COMBAT_1', 'tutorial_COMBAT_1_IPAD', [
        ('89f42001e7086e7987ca', 60, ''),
        ('758fa757dd6c795a0484', 53, 'TAP AIM (Y), choose your laser W2 and TAP POWER ON (or START).'),
        ('82d65546287fee43d779', 30, 'POWER OFF depowers it.'),
    ]),
    ('COMBAT_2', 'tutorial_COMBAT_2_IPAD', [
        ('bb87e93bf70ea61a197e', 86, 'Target a room in the enemy ship by TAPPING it (or D-PAD), then tap FIRE (A).'),
        ('0f9a6f73a9442f02c417', 34, '(PAUSE button or X)'),
    ]),
    ('COMBAT_3', 'tutorial_COMBAT_3', [
        ('8190b0651df39db78710', 12, "the enemy's"),
        ('95cc8a0d254ce086caa6', 8, "it can't"),
        ('b190a47ff29cbcd5135e', 76, ''),
    ]),
    # No inventory steps on the DS: the Kestrel already carries its Artemis.
    ('COMBAT_4', 'tutorial_COMBAT_4', [
        ('60866b113c40ef44dfc3', 67, 'Power your Artemis (W1) too.'),
        ('2284263918638ff1ed4e', 39, 'Shooting it uses your missiles'),
    ]),
    ('COMBAT_4_5', 'tutorial_COMBAT_4_5', [
        ('6f9ae903a713c88e20cf', 30, 'As enemy systems'),
        ('8c5d075d8d526b148792', 114, '. Attacking a RED system does no more system damage, but still damages'),
    ]),
    ('COMBAT_4_6', 'tutorial_COMBAT_4_6_PC', [
        ('fe81244479493f27b790', 140, ''),
        ('30bf2c469be25e75b23a', 11, 'the enemy!'),
    ]),
    ('DONE_1', 'tutorial_DONE_1_IPAD', [
        ('bc325370bed53cc4458d', 20, 'Tap CONTINUE (A) to quit'),
    ]),
]


def adapt(key, s, digest, size, new):
    """Replace the one span of s whose SHA-256 prefix is digest.

    Adaptation anchors are stored as hashes (not original text) so this
    repository contains none of FTL's tutorial wording. The span must match
    exactly once, so an unexpected archive version fails loudly."""
    hits = [i for i in range(len(s) - size + 1)
            if hashlib.sha256(s[i:i + size].encode()).hexdigest().startswith(digest)]
    if len(hits) != 1:
        raise ValueError('%s: adaptation %s matched %d times' % (key, digest, len(hits)))
    return s[:hits[0]] + new + s[hits[0] + size:]


def normalize(raw):
    s = html.unescape(raw).replace('\\n', '\n')
    s = re.sub(r'[ \t]*\n[ \t]*', '\n', s)
    s = re.sub(r'\n+', '\n', s)   # Paragraph break -> one DS line break.
    return s.strip()


def wrap(s):
    """Mirror of tut_wrap() in source/tutorial_view.inc."""
    lines = []
    for para in s.split('\n'):
        while para:
            if len(para) <= COLS:
                cut = len(para)
            else:
                cut = COLS
                while cut > 0 and para[cut] != ' ':
                    cut -= 1
                if not cut:
                    cut = COLS
            lines.append(para[:cut])
            para = para[cut:].lstrip(' ')
    return lines


def c_string(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n') + '"'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path, nargs='?')
    ap.add_argument('--show', action='store_true', help='print the wrapped DS prompts')
    args = ap.parse_args()
    output = Path(__file__).resolve().parents[1] / 'generated'
    archive = PkgArchive(args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive']))
    raw = archive.read('data/text_tutorial.xml')
    texts = {k: normalize(v) for k, v in re.findall(r'<text name="([^"]*)"[^>]*>(.*?)</text>', raw.decode('utf-8'), re.S)}
    rows = []
    for key, source, changes in STEPS:
        if source not in texts:
            raise ValueError('Missing tutorial text ' + source)
        s = texts[source]
        for digest, size, new in changes:
            s = adapt(key, s, digest, size, new)
        bad = sorted(set(s) - FONT)
        if bad:
            raise ValueError('%s: characters outside the DS font: %r' % (key, bad))
        lines = wrap(s)
        if len(lines) > LINES:
            raise ValueError('%s: %d lines (max %d):\n%s' % (key, len(lines), LINES, '\n'.join(lines)))
        if args.show:
            print('--- %s (%s, %d lines)' % (key, source, len(lines)))
            print('\n'.join(lines))
        rows.append((key, source, s, bool(changes)))
    header = ['/* Generated by tools/import_tutorial.py from the owner\'s FTL archive',
              ' * (data/text_tutorial.xml sha256 %s). Do not redistribute.' % hashlib.sha256(raw).hexdigest(),
              ' * Prompts keep the original wording; adapted ones (DS controls, screens,',
              ' * rules or length) are marked and listed in the importer. */',
              '#ifndef FTL_DS_TUTORIAL_TEXT_H', '#define FTL_DS_TUTORIAL_TEXT_H',
              '#define TUTORIAL_TEXT_COLS %d' % COLS, '#define TUTORIAL_TEXT_LINES %d' % LINES,
              '#define TUTORIAL_TEXT_COUNT %d' % len(rows),
              'enum TutorialTextId { ' + ', '.join('TT_' + key for key, _, _, _ in rows) + ' };',
              'typedef struct { const char *id,*text; int adapted; } TutorialText;',
              'static const TutorialText tutorial_prompts[TUTORIAL_TEXT_COUNT] = {']
    header += ['    {%s,%s,%d},' % (c_string(src), c_string(s), adapted) for _, src, s, adapted in rows]
    header += ['};', '#endif', '']
    (output / 'tutorial_text.h').write_text('\n'.join(header), encoding='utf-8', newline='\n')
    print('Imported %d tutorial prompts' % len(rows))


if __name__ == '__main__':
    main()
