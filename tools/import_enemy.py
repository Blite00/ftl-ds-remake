#!/usr/bin/env python3
"""Import an original Auto-Scout and a fixed basic-laser encounter from local FTL.

The archive remains read-only. Generated original game data stays local.
Enemy scaling and weapon randomization are deliberately not reconstructed here.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
from import_ftl import PkgArchive, xml_root, layout
from import_enemy_catalog import import_catalog


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path, nargs='?')
    ap.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'generated')
    args = ap.parse_args()
    output = args.output.resolve()
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    if output == archive_path.resolve().parent or archive_path.resolve().is_relative_to(output):
        raise ValueError('Output must be separate from installed archive')
    archive = PkgArchive(archive_path)
    payloads = {}
    manifest = []
    for name in ('data/autoBlueprints.xml', 'data/auto_basic.txt', 'data/auto_basic.xml',
                 'img/ships_glow/auto_basic_base.png'):
        payload = archive.read(name)
        payloads[name] = payload
        dest = output / 'original' / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(payload)
        manifest.append(dict(path=name, size=len(payload), sha256=hashlib.sha256(payload).hexdigest()))
    auto_root = xml_root(payloads['data/autoBlueprints.xml'])
    ship = auto_root.find(".//shipBlueprint[@name='AUTO_BASIC']")
    if ship is None or ship.get('layout') != 'auto_basic':
        raise ValueError('Original Auto-Scout blueprint missing')
    strings = {e.get('name'): e.text for e in xml_root(archive.read('data/text_blueprints.xml')).iter('text')}
    data = layout(payloads['data/auto_basic.txt'])
    data.update(blueprint='AUTO_BASIC', name=strings[ship.find('class').get('id')],
                hull=int(ship.find('health').get('amount')),
                reactor=int(ship.find('maxPower').get('amount')),
                crew_count=int(ship.find('crewCount').get('amount')),
                automated=int(ship.find('crewCount').get('amount')) == 0,
                missiles=int(ship.find('weaponList').get('missiles')))
    data['systems'] = [dict(name=s.tag, room=int(s.get('room')), power=int(s.get('power')),
                            maximum=int(s.get('max', s.get('power'))),
                            start=int(s.get('start', 'true') == 'true')) for s in ship.find('systemList')]
    pool_name = ship.find('weaponList').get('load')
    pool = auto_root.find(".//blueprintList[@name='%s']" % pool_name)
    data['weapon_pool'] = dict(name=pool_name, entries=[n.text for n in pool.findall('name')])
    weapon_id = 'LASER_BURST_1'
    if weapon_id not in data['weapon_pool']['entries']:
        raise ValueError('Chosen basic laser is not in original enemy weapon pool')
    weapon = xml_root(archive.read('data/blueprints.xml')).find(".//weaponBlueprint[@name='%s']" % weapon_id)
    title = weapon.find('title')
    item = dict(id=weapon_id, name=title.text or strings[title.get('id')],
                cooldown_ms=round(float(weapon.findtext('cooldown')) * 1000))
    for key, tag in [('damage','damage'),('shots','shots'),('power','power'),('missiles','missiles'),('shield_piercing','sp')]:
        item[key] = int(weapon.findtext(tag, '0'))
    data['weapons'] = [item]
    img = xml_root(payloads['data/auto_basic.xml']).find('.//img')
    data['hull_bbox'] = {k:int(img.get(k)) for k in ('x','y','w','h')}
    png = payloads['img/ships_glow/auto_basic_base.png']
    if png[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('Enemy hull is not PNG')
    png_width, png_height = struct.unpack('>II', png[16:24])
    data['art'] = dict(source='img/ships_glow/auto_basic_base.png', width=png_width, height=png_height,
                       converted_array='combat_enemy', converted_canvas=[90,65])
    data['grid_bounds'] = dict(x=min(r['x'] for r in data['rooms']), y=min(r['y'] for r in data['rooms']),
                              w=max(r['x']+r['w'] for r in data['rooms'])-min(r['x'] for r in data['rooms']),
                              h=max(r['y']+r['h'] for r in data['rooms'])-min(r['y'] for r in data['rooms']))
    data['selection_note'] = ('Fixed early encounter uses blueprint base hull/power, systems whose start is not false, '
                              'and one original Basic Laser selected from WEAPONS_AUTO. Original sector/difficulty '
                              'scaling, optional-system selection and weapon randomization are not implemented. '
                              'The source specifies base hull 6 with no hull min/max attribute; system maxima are upgrade ceilings.')
    data['source_files'] = manifest
    header = ['/* Original local FTL data. Do not redistribute. Fixed unscaled Auto-Scout encounter. */',
              '#ifndef ENEMY_DATA_H', '#define ENEMY_DATA_H',
              'typedef struct { int id,x,y,w,h; } EnemyRoom;',
              'typedef struct { int x,y,room_a,room_b,vertical; } EnemyDoor;',
              'typedef struct { const char *name; int room,power,start; } EnemySystem;',
              'typedef struct { const char *id,*name; int cooldown_ms,damage,shots,power,missiles,shield_piercing; } EnemyWeapon;',
              'static const char enemy_name[] = %s;' % json.dumps(data['name'])]
    for key in ('hull','reactor','crew_count','automated','missiles'):
        header.append('#define ENEMY_%s %d' % (key.upper(), int(data[key])))
    for key,value in data['grid_bounds'].items():
        header.append('#define ENEMY_GRID_%s %d' % (key.upper(),value))
    for key,value in data['hull_bbox'].items():
        header.append('#define ENEMY_HULL_%s %d' % (key.upper(),value))
    header += ['#define ENEMY_ART_WIDTH %d' % png_width, '#define ENEMY_ART_HEIGHT %d' % png_height]
    for key,typ,fields in [('rooms','EnemyRoom',('id','x','y','w','h')),
                            ('doors','EnemyDoor',('x','y','room_a','room_b','vertical')),
                            ('systems','EnemySystem',('name','room','power','start')),
                            ('weapons','EnemyWeapon',('id','name','cooldown_ms','damage','shots','power','missiles','shield_piercing'))]:
        header.append('#define ENEMY_%s_COUNT %d' % (key[:-1].upper(),len(data[key])))
        header.append('static const %s enemy_%s[] = {' % (typ,key))
        header.extend('    {'+','.join(json.dumps(item[f]) for f in fields)+'},' for item in data[key])
        header.append('};')
    header.append('#endif\n')
    (output / 'enemy_data.h').write_text('\n'.join(header),encoding='utf-8')
    (output / 'enemy_data.json').write_text(json.dumps(data,indent=2),encoding='utf-8')
    import_catalog(archive_path, output)
    print('Imported Auto-Scout: %d rooms, %d doors, hull %d, crew %d; fixed %s' %
          (len(data['rooms']),len(data['doors']),data['hull'],data['crew_count'],item['name']))


if __name__ == '__main__':
    main()
