#!/usr/bin/env python3
"""Import the 18 base-game player ship layouts (9 cruisers, Type A and B).

Writes generated/player_ships.h (layout, systems, starting crew/weapons/drones/
augments, names and unlock text) and one art pack per layout under
nitrofiles/ships/ (tactical hull, combat and hangar art, room furnishings), so
only the chosen ship's art is in RAM. Type C layouts and the Lanius cruiser are
Advanced Edition and excluded, as are Advanced Edition systems (clone bay,
battery, mind control, hacking). Requires Pillow. Output stays local; do not
redistribute.

Ship class ids follow the original order (unlockShip ids): 0 Kestrel, 1 Stealth,
2 Mantis, 3 Engi, 4 Federation, 5 Slug, 6 Rock, 7 Zoltan, 8 Crystal.
Layout index = class * 2 + (0 for Type A, 1 for Type B).
"""
import argparse
import hashlib
import io
import json
import struct
from pathlib import Path
from PIL import Image
from import_ftl import PkgArchive, xml_root, layout

CLASSES = [('Kestrel', 'PLAYER_SHIP_HARD', 'KESTREL'), ('Stealth', 'PLAYER_SHIP_STEALTH', 'STEALTH'),
           ('Mantis', 'PLAYER_SHIP_MANTIS', 'MANTIS'), ('Engi', 'PLAYER_SHIP_CIRCLE', 'CIRCLE'),
           ('Federation', 'PLAYER_SHIP_FED', 'FED'), ('Slug', 'PLAYER_SHIP_JELLY', 'JELLY'),
           ('Rock', 'PLAYER_SHIP_ROCK', 'ROCK'), ('Zoltan', 'PLAYER_SHIP_ENERGY', 'ENERGY'),
           ('Crystal', 'PLAYER_SHIP_CRYSTAL', 'CRYSTAL')]
# text_blueprints keys differ from blueprint names for a few ships.
TEXT_KEYS = {'PLAYER_SHIP_HARD': 'PLAYER_SHIP_KESTREL', 'PLAYER_SHIP_HARD_2': 'PLAYER_SHIP_KESTREL_2',
             'PLAYER_SHIP_CIRCLE_2': 'PLAYER_SHIP_ENGI_2'}
# Base-game Type A unlock rules (the installed text, which also serves as the hint).
UNLOCK_A = {'PLAYER_SHIP_CIRCLE': 'Reach sector 5 with the Kestrel.'}
AE_SYSTEMS = {'clonebay', 'battery', 'mind', 'hacking'}
SYSTEM_CODES = ['shields', 'engines', 'weapons', 'oxygen', 'medbay', 'pilot', 'sensors', 'doors',
                'teleporter', 'drones', 'cloaking', 'artillery']
SPECIES = {'human': 0, 'engi': 1, 'mantis': 2, 'rock': 3, 'slug': 4, 'energy': 5, 'crystal': 6}
# Starting weapons missing from the shared catalog, mapped to their closest
# catalog weapon until the unique version exists.
WEAPON_FALLBACK = {}
TACTICAL_SIZE = (256, 88)
TACTICAL_Y = 23


def geometry(rooms):
    """Mirror of render.c geometry() for the player ship (tile and origin)."""
    mx = min(r['x'] for r in rooms); my = min(r['y'] for r in rooms)
    xx = max(r['x'] + r['w'] for r in rooms); yy = max(r['y'] + r['h'] for r in rooms)
    height, t = 84, 14
    if (xx - mx) * t > 232:
        t = 232 // (xx - mx)
    if (yy - my) * t > height:
        t = height // (yy - my)
    t = max(t, 1)
    ox = (256 - (xx - mx) * t) // 2 - mx * t
    oy = 25 + (height - (yy - my) * t) // 2 - my * t
    return t, ox, oy


def ship_mounts(ship_xml, size, source, limit=8):
    """Original <weaponMounts> x,y (hull-image pixels). Raises on missing/invalid data."""
    node = ship_xml.find('.//weaponMounts')
    mounts = []
    for m in (node.findall('mount') if node is not None else []):
        try:
            x, y = int(m.get('x')), int(m.get('y'))
        except (TypeError, ValueError):
            raise ValueError('Malformed weapon mount in ' + source)
        if not (0 <= x <= size[0] and 0 <= y <= size[1]):
            raise ValueError('Weapon mount outside the hull image in ' + source)
        mounts.append((x, y))
    if not mounts:
        raise ValueError('Missing weaponMounts in ' + source)
    return mounts[:limit]


def rgb555(image):
    return [0 if a < 128 else 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
            for r, g, b, a in image.get_flattened_data()]


def fit_canvas(image, size):
    image = image.copy()
    image.thumbnail(size, Image.Resampling.LANCZOS)
    canvas = Image.new('RGBA', size)
    canvas.alpha_composite(image, ((size[0] - image.width) // 2, (size[1] - image.height) // 2))
    return canvas


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path, nargs='?')
    args = ap.parse_args()
    root_dir = Path(__file__).resolve().parents[1]
    output = root_dir / 'generated'
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    archive = PkgArchive(archive_path)
    blueprints = xml_root(archive.read('data/blueprints.xml'))
    strings = {e.get('name'): e.text or '' for e in xml_root(archive.read('data/text_blueprints.xml')).iter('text')}
    weapons = [w['id'] for w in json.loads((output / 'weapon_catalog.json').read_text())['weapons']]
    drones = [d['id'] for d in json.loads((output / 'drone_catalog.json').read_text())['drones']]
    augments = [a['id'] for a in json.loads((output / 'augment_catalog.json').read_text())['augments']]
    pack_dir = root_dir / 'nitrofiles' / 'ships'
    pack_dir.mkdir(parents=True, exist_ok=True)
    meta, ships = [], []

    def text(node, key, fallback=''):
        el = node.find(key)
        if el is None:
            return fallback
        return (el.text or strings.get(el.get('id'), fallback) or fallback).strip()

    for cls, (cls_name, base, _) in enumerate(CLASSES):
        for variant in (0, 1):
            name = base if variant == 0 else base + '_2'
            bp = blueprints.find(".//shipBlueprint[@name='%s']" % name)
            if bp is None:
                raise ValueError('Missing blueprint ' + name)
            lay, img = bp.get('layout'), bp.get('img')
            payloads = {p: archive.read(p) for p in ('data/%s.txt' % lay, 'data/%s.xml' % lay,
                                                      'img/ship/%s_base.png' % img)}
            for p, data in payloads.items():
                meta.append({'source': p, 'sha256': hashlib.sha256(data).hexdigest()})
            grid = layout(payloads['data/%s.txt' % lay])
            key = TEXT_KEYS.get(name, name)
            title = strings.get('ship_%s_name' % key) or text(bp, 'name', name)
            klass = strings.get('ship_%s_class' % key) or text(bp, 'class', cls_name)
            unlock = UNLOCK_A.get(name) or (strings.get('ship_%s_unlock' % key) or text(bp, 'unlock', '')).strip()
            if variant == 1:
                unlock = 'Complete 2 of the 3 %s achievements.' % klass
            if cls == 0 and variant == 0:
                unlock = ''
            systems = []
            for s in bp.find('systemList'):
                if s.tag in AE_SYSTEMS:
                    continue
                if s.tag not in SYSTEM_CODES:
                    raise ValueError('Unknown system %s in %s' % (s.tag, name))
                systems.append(dict(code=SYSTEM_CODES.index(s.tag), room=int(s.get('room')), power=int(s.get('power')),
                                    start=int(s.get('start', 'true') == 'true'), img=s.get('img')))
            crew = []
            for c in bp.findall('crewCount'):
                crew += [SPECIES[c.get('class')]] * int(c.get('amount'))
            wl = bp.find('weaponList')
            weapon_ids = []
            for w in (wl if wl is not None else []):
                wid = WEAPON_FALLBACK.get(w.get('name'), w.get('name'))
                if wid not in weapons:
                    if w.get('name') == 'ARTILLERY_FED':
                        continue
                    raise ValueError('Unknown weapon %s in %s' % (wid, name))
                weapon_ids.append(weapons.index(wid))
            dl = bp.find('droneList')
            drone_ids = [drones.index(d.get('name')) for d in (dl if dl is not None else [])]
            aug_ids = [augments.index(a.get('name')) for a in bp.findall('aug')]
            ships.append(dict(blueprint=name, cls=cls, variant=variant, name=title, klass=klass, unlock=unlock,
                              hull=int(bp.find('health').get('amount')), reactor=int(bp.find('maxPower').get('amount')),
                              missiles=int(wl.get('missiles', '0')) if wl is not None else 0,
                              drone_parts=int(dl.get('drones', '0')) if dl is not None else 0,
                              weapon_slots=int(bp.findtext('weaponSlots', '4')), drone_slots=int(bp.findtext('droneSlots', '2')),
                              rooms=grid['rooms'], doors=grid['doors'], systems=systems, crew=crew,
                              weapons=weapon_ids, drones=drone_ids, augments=aug_ids))
            # Art pack.
            ship_xml = xml_root(payloads['data/%s.xml' % lay])
            bounds = ship_xml.find('.//img')
            hx, hy = int(bounds.get('x')), int(bounds.get('y'))
            hull = Image.open(io.BytesIO(payloads['img/ship/%s_base.png' % img])).convert('RGBA')
            layered = hull.copy()
            floor_off = ship_xml.find('.//offsets/floor')
            try:
                floor = Image.open(io.BytesIO(archive.read('img/ship/%s_floor.png' % img))).convert('RGBA')
                fx, fy = (int(floor_off.get('x')), int(floor_off.get('y'))) if floor_off is not None else (0, 0)
                layered.alpha_composite(floor, (fx, fy))
            except KeyError:
                pass
            # V68 weapon mounts (image pixels, relative to the hull image), used in
            # slot order like the original; fail closed on missing or out-of-hull data.
            mounts = ship_mounts(ship_xml, hull.size, 'data/%s.xml' % lay)
            mini = hull.copy()
            mini.thumbnail((110, 70), Image.Resampling.LANCZOS)
            ships[-1]['hull_art'] = dict(x=hx, y=hy, w=hull.width, h=hull.height, mounts=mounts,
                                         mini=[(110 - mini.width) // 2, (70 - mini.height) // 2, mini.width, mini.height])
            t, ox, oy = geometry(grid['rooms'])
            scale = t / 35
            tactical = Image.new('RGBA', TACTICAL_SIZE)
            tactical.alpha_composite(layered.resize((round(layered.width * scale), round(layered.height * scale)), Image.Resampling.LANCZOS),
                                     (round(ox + hx * scale), round(oy + hy * scale) - TACTICAL_Y))
            combat = fit_canvas(hull, (110, 70))
            hangar = hull.copy()
            hangar.thumbnail((240, 90), Image.Resampling.LANCZOS)
            hangar = fit_canvas(hangar, (140, 90))
            interiors = []
            for s in systems:
                if not s['img']:
                    continue
                try:
                    art = Image.open(io.BytesIO(archive.read('img/ship/interior/%s.png' % s['img']))).convert('RGBA')
                except KeyError:
                    continue
                art = art.resize((max(1, round(art.width * scale)), max(1, round(art.height * scale))), Image.Resampling.LANCZOS)
                interiors.append((s['code'], s['room'], art))
            blob = bytearray(b'SHP1')
            blob += struct.pack('<6H', TACTICAL_SIZE[0], TACTICAL_SIZE[1], 110, 70, 140, 90)
            for image in (tactical, combat, hangar):
                blob += struct.pack('<%dH' % (image.width * image.height), *rgb555(image))
            blob += struct.pack('<H', len(interiors))
            for code, room, art in interiors:
                blob += struct.pack('<4H', code, room, art.width, art.height)
                blob += struct.pack('<%dH' % (art.width * art.height), *rgb555(art))
            (pack_dir / ('%s.bin' % name.lower())).write_bytes(bytes(blob))
            tactical.save(output / ('player_ship_%s.png' % name.lower()))
            ships[-1]['pack_bytes'] = len(blob)

    header = ['/* Generated by tools/import_player_ships.py from the owner\'s FTL archive. Do not redistribute. */',
              '#ifndef FTL_DS_PLAYER_SHIPS_H', '#define FTL_DS_PLAYER_SHIPS_H',
              '#include "ftl_data.h"',
              '#define PLAYER_SHIP_COUNT %d' % len(ships),
              '#define PLAYER_SHIP_CLASSES %d' % len(CLASSES),
              '/* System codes: 0 shields 1 engines 2 weapons 3 oxygen 4 medbay 5 pilot 6 sensors 7 doors',
              ' * 8 teleporter 9 drones 10 cloaking 11 artillery. Species: 0 human 1 engi 2 mantis 3 rock 4 slug 5 zoltan 6 crystal. */',
              'typedef struct { int code,room,power,start; } PlayerShipSystem;',
              'typedef struct { const char *blueprint,*name,*klass,*unlock,*pack; int cls,variant,hull,reactor,missiles,drone_parts,weapon_slots,drone_slots;',
              '  int room_count,door_count,system_count,crew_count,weapon_count,drone_count,augment_count;',
              '  const FtlRoom *rooms; const FtlDoor *doors; const PlayerShipSystem *systems;',
              '  int crew[8],weapons[4],drones[3],augments[3]; } PlayerShipData;']
    for i, s in enumerate(ships):
        header.append('static const FtlRoom player_ship_rooms_%d[] = {%s};' % (i, ','.join('{%d,%d,%d,%d,%d}' % (r['id'], r['x'], r['y'], r['w'], r['h']) for r in s['rooms'])))
        header.append('static const FtlDoor player_ship_doors_%d[] = {%s};' % (i, ','.join('{%d,%d,%d,%d,%d}' % (d['x'], d['y'], d['room_a'], d['room_b'], d['vertical']) for d in s['doors'])))
        header.append('static const PlayerShipSystem player_ship_systems_%d[] = {%s};' % (i, ','.join('{%d,%d,%d,%d}' % (y['code'], y['room'], y['power'], y['start']) for y in s['systems'])))
    header.append('static const PlayerShipData player_ships[PLAYER_SHIP_COUNT] = {')

    def arr(values, n):
        values = list(values) + [-1] * (n - len(values))
        return '{' + ','.join(map(str, values[:n])) + '}'
    for i, s in enumerate(ships):
        header.append('  {%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,player_ship_rooms_%d,player_ship_doors_%d,player_ship_systems_%d,%s,%s,%s,%s},' % (
            json.dumps(s['blueprint']), json.dumps(s['name']), json.dumps(s['klass']), json.dumps(s['unlock']),
            json.dumps('ships/%s.bin' % s['blueprint'].lower()), s['cls'], s['variant'], s['hull'], s['reactor'], s['missiles'],
            s['drone_parts'], s['weapon_slots'], s['drone_slots'], len(s['rooms']), len(s['doors']), len(s['systems']),
            len(s['crew']), len(s['weapons']), len(s['drones']), len(s['augments']), i, i, i,
            arr(s['crew'], 8), arr(s['weapons'], 4), arr(s['drones'], 3), arr(s['augments'], 3)))
    header.append('};')
    header += ['/* V68 hull image placement (room-grid pixels, 35 per tile), its fitted 110x70 combat',
               ' * thumbnail rect and the original weapon mounts (hull-image pixels; slot i uses mount i). */',
               '#define PLAYER_SHIP_MAX_MOUNTS 8',
               'typedef struct { int hull_x,hull_y,hull_w,hull_h,mini_x,mini_y,mini_w,mini_h,mount_count; int mounts[PLAYER_SHIP_MAX_MOUNTS][2]; } PlayerShipHull;',
               'static const PlayerShipHull player_ship_hulls[PLAYER_SHIP_COUNT] = {']
    for s in ships:
        a = s['hull_art']
        pts = ','.join('{%d,%d}' % m for m in a['mounts'])
        header.append('  {%d,%d,%d,%d,%d,%d,%d,%d,%d,{%s}},' % (a['x'], a['y'], a['w'], a['h'], *a['mini'], len(a['mounts']), pts))
    header += ['};', '#endif', '']
    (output / 'player_ships.h').write_text('\n'.join(header), encoding='utf-8')
    (output / 'player_ships.json').write_text(json.dumps({'ships': ships, 'assets': meta}, indent=2), encoding='utf-8')
    print('Imported %d player ship layouts, %d art bytes' % (len(ships), sum(s['pack_bytes'] for s in ships)))


if __name__ == '__main__':
    main()
