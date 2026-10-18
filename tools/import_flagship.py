#!/usr/bin/env python3
"""Import the base-game normal-difficulty Rebel flagship stages (local archive only).

Reads data/bosses.xml BOSS_1..3_NORMAL (layouts boss_N_easy), their artillery
weapons from data/blueprints.xml and the hull art. Writes generated/flagship_data.h
(geometry/systems/weapons, used by game code) and generated/flagship_art.h
(90x65 RGB555 thumbnails, used by the renderer only).
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image
from import_ftl import PkgArchive, xml_root, layout
from convert_ship_art import c_bitmap
from import_player_ships import ship_mounts

STAGES = ('BOSS_1_NORMAL', 'BOSS_2_NORMAL', 'BOSS_3_NORMAL')
CANVAS = (90, 65)
SURGE_LASER = 'LASER_HEAVY_1'


def weapon_record(base, name):
    w = base.find(".//weaponBlueprint[@name='%s']" % name)
    if w is None:
        raise ValueError('Missing flagship weapon ' + name)
    rec = dict(id=name, type=w.findtext('type'), cooldown_ms=round(float(w.findtext('cooldown')) * 1000))
    for key, tag in [('damage', 'damage'), ('shots', 'shots'), ('shield_piercing', 'sp'), ('ion', 'ion'),
                     ('fire_chance', 'fireChance'), ('breach_chance', 'breachChance'), ('beam_length', 'length'),
                     ('missiles', 'missiles')]:
        rec[key] = int(w.findtext(tag, '0'))
    if rec['type'] == 'BEAM':
        rec['shots'] = 1
    return rec


def import_flagship(archive_path, output):
    archive_path, output = Path(archive_path).resolve(), Path(output).resolve()
    archive = PkgArchive(archive_path)
    sources = {}

    def original(name):
        payload = archive.read(name)
        sources[name] = dict(path=name, size=len(payload), sha256=hashlib.sha256(payload).hexdigest())
        return payload

    bosses = xml_root(original('data/bosses.xml'))
    base = xml_root(original('data/blueprints.xml'))
    weapons, weapon_index, stages = [], {}, []
    art = ['/* Original local FTL flagship art. Do not redistribute. */', '#ifndef FLAGSHIP_ART_H', '#define FLAGSHIP_ART_H',
           '#include <stdint.h>', '#include "enemy_catalog_art.h"']
    for stage_id in STAGES:
        bp = bosses.find(".//shipBlueprint[@name='%s']" % stage_id)
        if bp is None:
            raise ValueError('Missing ' + stage_id)
        name = bp.get('layout')
        geo = layout(original('data/' + name + '.txt'))
        ship_xml = xml_root(original('data/' + name + '.xml'))
        img = ship_xml.find('.//img')
        systems = []
        for s in bp.find('systemList'):
            rec = dict(name=s.tag, room=int(s.get('room')), power=int(s.get('power')), weapon=-1)
            if s.tag == 'artillery':
                wid = s.get('weapon')
                if wid not in weapon_index:
                    weapon_index[wid] = len(weapons)
                    weapons.append(weapon_record(base, wid))
                rec['weapon'] = weapon_index[wid]
            systems.append(rec)
        room_ids = [r['id'] for r in geo['rooms']]
        if len(room_ids) > 32 or len(geo['doors']) > 48 or any(s['room'] not in room_ids for s in systems):
            raise ValueError('Flagship stage exceeds DS bounds: ' + stage_id)
        drones = [d.get('name') for d in bp.find('droneList')] if bp.find('droneList') is not None else []
        stage = dict(blueprint=stage_id, layout=name, hull=int(bp.find('health').get('amount')),
                     reactor=int(bp.find('maxPower').get('amount')), rooms=geo['rooms'], doors=geo['doors'],
                     systems=systems, drones=drones, boarding=bp.findtext('boardingAI', ''),
                     hull_bbox={k: int(img.get(k)) for k in ('x', 'y', 'w', 'h')})
        picture = Image.open(io.BytesIO(original('img/ships_glow/' + bp.get('img') + '_base.png'))).convert('RGBA')
        stage['art_source_size'] = picture.size
        stage['mounts'] = ship_mounts(ship_xml, picture.size, 'data/' + name + '.xml')
        picture.thumbnail(CANVAS, Image.Resampling.LANCZOS)
        x, y = (CANVAS[0] - picture.width) // 2, (CANVAS[1] - picture.height) // 2
        canvas = Image.new('RGBA', CANVAS)
        canvas.alpha_composite(picture, (x, y))
        stage['art_rect'] = dict(x=x, y=y, w=picture.width, h=picture.height)
        art += c_bitmap('imported_flagship_%d_art' % len(stages), canvas)
        canvas.save(output / ('imported_flagship_%d.png' % (len(stages) + 1)))
        stages.append(stage)
    surge = weapon_record(base, SURGE_LASER)
    h = ['/* Original local FTL flagship data (normal difficulty). Do not redistribute. */', '#ifndef FLAGSHIP_DATA_H',
         '#define FLAGSHIP_DATA_H', '#include "enemy_catalog.h"', '#define FLAGSHIP_STAGE_COUNT 3',
         '#define FLAGSHIP_WEAPON_COUNT %d' % len(weapons),
         'typedef struct { const char *name; int room,power,weapon; } FlagshipSystem;',
         'typedef struct { const char *id,*type; int cooldown_ms,damage,shots,shield_piercing,ion,fire_chance,breach_chance,beam_length,missiles; } FlagshipWeapon;',
         'typedef struct { const char *blueprint; int hull,reactor,room_count,door_count,system_count; const ImportedEnemyRoom *rooms; const ImportedEnemyDoor *doors; const FlagshipSystem *systems; int hull_x,hull_y,hull_w,hull_h,art_source_width,art_source_height; } FlagshipStage;']
    wf = ('id', 'type', 'cooldown_ms', 'damage', 'shots', 'shield_piercing', 'ion', 'fire_chance', 'breach_chance', 'beam_length', 'missiles')
    h.append('static const FlagshipWeapon flagship_weapons[FLAGSHIP_WEAPON_COUNT] = {')
    h += ['    {' + ','.join(json.dumps(w[k]) for k in wf) + '},' for w in weapons]
    h.append('};')
    h.append('/* Power-surge barrage blueprint (LASER_HEAVY_1); the original hard-codes 1 damage. */')
    h.append('static const FlagshipWeapon flagship_surge_laser = {' + ','.join(json.dumps(surge[k]) for k in wf) + '};')
    for i, st in enumerate(stages):
        h.append('static const ImportedEnemyRoom flagship_%d_rooms[] = {' % i)
        h += ['    {%d,%d,%d,%d,%d},' % (r['id'], r['x'], r['y'], r['w'], r['h']) for r in st['rooms']]
        h.append('};')
        h.append('static const ImportedEnemyDoor flagship_%d_doors[] = {' % i)
        h += ['    {%d,%d,%d,%d,%d},' % (d['x'], d['y'], d['room_a'], d['room_b'], d['vertical']) for d in st['doors']]
        h.append('};')
        h.append('static const FlagshipSystem flagship_%d_systems[] = {' % i)
        h += ['    {%s,%d,%d,%d},' % (json.dumps(s['name']), s['room'], s['power'], s['weapon']) for s in st['systems']]
        h.append('};')
    h.append('static const FlagshipStage flagship_stages[FLAGSHIP_STAGE_COUNT] = {')
    for i, st in enumerate(stages):
        b = st['hull_bbox']
        h.append('    {%s,%d,%d,%d,%d,%d,flagship_%d_rooms,flagship_%d_doors,flagship_%d_systems,%d,%d,%d,%d,%d,%d},' % (
            json.dumps(st['blueprint']), st['hull'], st['reactor'], len(st['rooms']), len(st['doors']), len(st['systems']),
            i, i, i, b['x'], b['y'], b['w'], b['h'], st['art_source_size'][0], st['art_source_size'][1]))
    h.append('};')
    h.append('/* V68 original weapon mounts per stage (hull-image pixels; weapons 0-3, artillery 4+i). */')
    h.append('static const ImportedEnemyMounts flagship_mounts[FLAGSHIP_STAGE_COUNT] = {')
    h += ['    {%d,{%s}},' % (len(st['mounts']), ','.join('{%d,%d}' % tuple(m) for m in st['mounts'])) for st in stages]
    h += ['};', '#endif', '']
    (output / 'flagship_data.h').write_text('\n'.join(h), encoding='utf-8')
    art.append('static const uint16_t *const imported_flagship_art[3] = { imported_flagship_0_art, imported_flagship_1_art, imported_flagship_2_art };')
    art.append('static const ImportedEnemyArtRect imported_flagship_art_rect[3] = {')
    art += ['    {%d,%d,%d,%d},' % (s['art_rect']['x'], s['art_rect']['y'], s['art_rect']['w'], s['art_rect']['h']) for s in stages]
    art += ['};', '#endif', '']
    (output / 'flagship_art.h').write_text('\n'.join(art), encoding='utf-8')
    (output / 'flagship_data.json').write_text(json.dumps(dict(stages=stages, weapons=weapons, surge_laser=surge,
        sources=list(sources.values()),
        note='Normal-difficulty base-game blueprints. Drone lists, boarding AI and surge parameters are recorded; runtime support is staged (see docs/V42_FLAGSHIP.md).'),
        indent=2), encoding='utf-8')
    print('Imported %d flagship stages, %d artillery weapons.' % (len(stages), len(weapons)))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path, nargs='?')
    ap.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'generated')
    args = ap.parse_args()
    archive = args.archive or Path(json.loads((args.output / 'manifest.json').read_text())['archive'])
    import_flagship(archive, args.output)


if __name__ == '__main__':
    main()
