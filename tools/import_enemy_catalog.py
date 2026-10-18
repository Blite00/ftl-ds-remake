#!/usr/bin/env python3
"""Import bounded base-game enemy catalog from owner's read-only archive."""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image
from import_ftl import PkgArchive, xml_root, layout
from convert_ship_art import c_bitmap
from import_species import import_species
from import_player_ships import ship_mounts

# Indices 0..3 are the historical V33 hulls (ENEMY_SCOUT/ASSAULT/REBEL/REBEL_FAT); keep them first.
# V52 adds the remaining base-game enemy ships and their base pirate variants
# (dlcPirateBlueprints.xml entries without _DLC use only base-game systems).
SHIP_IDS = ('AUTO_BASIC', 'AUTO_ASSAULT', 'REBEL_SKINNY', 'REBEL_FAT',
            'REBEL_SKINNY_ELITE', 'JELLY_BUTTON', 'JELLY_CROISSANT', 'JELLY_TRUFFLE', 'ROCK_SCOUT', 'ROCK_FIGHT', 'ROCK_ASSAULT',
            'MANTIS_SCOUT', 'MANTIS_FIGHTER', 'MANTIS_BOMBER', 'FED_SCOUT', 'FED_BOMBER', 'CIRCLE_SCOUT', 'CIRCLE_BOMBER',
            'ZOLTAN_FIGHTER', 'ZOLTAN_BOMBER', 'CRYSTAL_SCOUT', 'CRYSTAL_BOMBER',
            'REBEL_FAT_P', 'REBEL_SKINNY_P', 'JELLY_BUTTON_P', 'JELLY_CROISSANT_P', 'JELLY_TRUFFLE_P', 'ROCK_SCOUT_P', 'ROCK_FIGHT_P',
            'MANTIS_SCOUT_P', 'MANTIS_FIGHTER_P', 'MANTIS_BOMBER_P', 'ZOLTAN_FIGHTER_P', 'ZOLTAN_BOMBER_P', 'FED_SCOUT_P', 'FED_BOMBER_P',
            # V61: unique quest ships referenced by name from events.
            'ROCK_ASSAULT_ELITE', 'ZOLTAN_PEACE',
            # V66: the flagship under construction (FLAGSHIP_CONSTRUCTION, Federation unlock).
            'BOSS_SPECIAL')
CANVAS = (90, 65)


def import_catalog(archive_path, output):
    archive_path, output = Path(archive_path).resolve(), Path(output).resolve()
    if output == archive_path.parent or archive_path.is_relative_to(output):
        raise ValueError('Output must be separate from installed archive')
    output.mkdir(parents=True, exist_ok=True)
    archive = PkgArchive(archive_path)
    sources = {}

    def original(name):
        payload = archive.read(name)
        destination = output / 'original' / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(payload)
        if destination.read_bytes() != payload:
            raise IOError('Extraction verification failed: ' + name)
        sources[name] = dict(path=name, size=len(payload), sha256=hashlib.sha256(payload).hexdigest())
        return payload

    autos = xml_root(original('data/autoBlueprints.xml'))
    pirates = xml_root(original('data/dlcPirateBlueprints.xml'))
    # V66: the half-built flagship (BOSS_SPECIAL) lives in blueprints.xml.
    mains = xml_root(original('data/blueprints.xml'))
    weapon_ids = [w['id'] for w in json.loads((output / 'weapon_catalog.json').read_text())['weapons']]
    base = xml_root(original('data/blueprints.xml'))
    strings = {n.get('name'): n.text for n in xml_root(original('data/text_blueprints.xml')).iter('text')}
    drone = base.find(".//droneBlueprint[@name='COMBAT_1']")
    laser = base.find(".//weaponBlueprint[@name='DRONE_LASER']")
    if drone is None or laser is None or drone.findtext('weaponBlueprint') != 'DRONE_LASER':
        raise ValueError('Expected original combat drone laser')
    drone_data = dict(id='COMBAT_1', name=strings[drone.find('title').get('id')],
                      type=drone.findtext('type'), power=int(drone.findtext('power')),
                      cooldown_raw=int(drone.findtext('cooldown')), speed=int(drone.findtext('speed')),
                      dodge=int(drone.findtext('dodge')), weapon='DRONE_LASER',
                      damage=int(laser.findtext('damage')), shots=int(laser.findtext('shots')),
                      fire_chance=int(laser.findtext('fireChance')), breach_chance=int(laser.findtext('breachChance')))
    rebel_weapon = base.find(".//weaponBlueprint[@name='LASER_BURST_1']")
    rebel_weapon_data = dict(id='LASER_BURST_1', name=strings[rebel_weapon.find('title').get('id')],
                            cooldown_ms=round(float(rebel_weapon.findtext('cooldown')) * 1000))
    for key, tag in [('damage','damage'),('shots','shots'),('power','power'),('missiles','missiles'),
                     ('shield_piercing','sp'),('fire_chance','fireChance'),('breach_chance','breachChance')]:
        rebel_weapon_data[key] = int(rebel_weapon.findtext(tag, '0'))
    # Preserve absent override fields separately from explicit zero. XML alone
    # does not establish engine units, probability ordering or inheritance.
    properties = []
    weapon_catalog = []
    for weapon_id in ('MISSILES_2_PLAYER', 'LASER_BURST_3', 'LASER_BURST_1', 'LASER_HEAVY_1', 'ION_1', 'BEAM_2', 'BOMB_1'):
        weapon = base.find(".//weaponBlueprint[@name='%s']" % weapon_id)
        if weapon is None:
            raise ValueError('Missing supported weapon properties: ' + weapon_id)
        title = weapon.find('title')
        record = dict(id=weapon_id, name=title.text or strings[title.get('id')], type=weapon.findtext('type'),
                      cooldown_ms=round(float(weapon.findtext('cooldown'))*1000))
        for key, tag in [('power','power'),('damage','damage'),('shots','shots'),('missiles','missiles'),
                         ('shield_piercing','sp'),('ion','ion'),('cost','cost'),('speed','speed'),
                         ('fire_chance','fireChance'),('breach_chance','breachChance'),('beam_length','length')]:
            record[key] = int(weapon.findtext(tag, '0'))
        # Omitted speed uses the engine default, not an asserted zero-speed rule.
        record['has_speed'] = int(weapon.find('speed') is not None)
        record['excluded_stun_chance'] = int(weapon.findtext('stunChance', '0'))
        weapon_catalog.append(record)
        properties.append(dict(id=weapon_id,
                               fire_chance=int(weapon.findtext('fireChance', '0')),
                               breach_chance=int(weapon.findtext('breachChance', '0')),
                               system_damage=int(weapon.findtext('sysDamage', '0')),
                               personnel_damage=int(weapon.findtext('persDamage', '0')),
                               has_system_damage=int(weapon.find('sysDamage') is not None),
                               has_personnel_damage=int(weapon.find('persDamage') is not None)))
    property_fields = ('id','fire_chance','breach_chance','system_damage','personnel_damage','has_system_damage','has_personnel_damage')
    property_header = ['/* Original local XML fields. Raw chances; absent overrides have explicit presence flags. */',
                       '#ifndef WEAPON_PROPERTIES_H', '#define WEAPON_PROPERTIES_H',
                       'typedef struct { const char *id; int fire_chance,breach_chance,system_damage,personnel_damage,has_system_damage,has_personnel_damage; } ImportedWeaponProperties;',
                       'static const ImportedWeaponProperties imported_weapon_properties[7] = {']
    property_header += ['    {' + ','.join(json.dumps(row[key]) for key in property_fields) + '},' for row in properties]
    property_header += ['};', '#endif', '']
    # V51: tools/import_weapons.py owns weapon_properties.h (full base-game list).
    catalog_fields = ('id','name','type','cooldown_ms','power','damage','shots','missiles','shield_piercing','ion','cost','speed','fire_chance','breach_chance','beam_length')
    catalog_header = ['/* Original local selected weapon data; raw chance values. */', '#ifndef WEAPON_CATALOG_H',
                      '#define WEAPON_CATALOG_H', '#define IMPORTED_WEAPON_COUNT 7',
                      'typedef struct { const char *id,*name,*type; int cooldown_ms,power,damage,shots,missiles,shield_piercing,ion,cost,speed,fire_chance,breach_chance,beam_length; } ImportedWeapon;',
                      'static const ImportedWeapon imported_weapon_catalog[IMPORTED_WEAPON_COUNT] = {']
    catalog_header += ['    {' + ','.join(json.dumps(row[key]) for key in catalog_fields) + '},' for row in weapon_catalog]
    catalog_header += ['};', '#endif', '']
    # V51: tools/import_weapons.py owns weapon_catalog.h.
    # V62: tools/import_weapons.py also owns weapon_catalog.json (39 weapons); this importer only reads it.
    ships = []
    art_header = ['/* Original local FTL assets. Do not redistribute. */', '#ifndef ENEMY_CATALOG_ART_H',
                  '#define ENEMY_CATALOG_ART_H', '#include <stdint.h>', '#define IMPORTED_ENEMY_ART_WIDTH 90',
                  '#define IMPORTED_ENEMY_ART_HEIGHT 65',
                  'typedef struct { int x,y,w,h; } ImportedEnemyArtRect;']
    for ship_id in SHIP_IDS:
        blueprint = autos.find(".//shipBlueprint[@name='%s']" % ship_id)
        if blueprint is None:
            blueprint = pirates.find(".//shipBlueprint[@name='%s']" % ship_id)
        if blueprint is None:
            blueprint = mains.find(".//shipBlueprint[@name='%s']" % ship_id)
        if blueprint is None:
            raise ValueError('Expected base-game ship: ' + ship_id)
        name = blueprint.get('layout')
        ship = layout(original('data/' + name + '.txt'))
        ship_xml = xml_root(original('data/' + name + '.xml'))
        img = ship_xml.find('.//img')
        image_path = 'img/ships_glow/' + blueprint.get('img') + '_base.png'
        try:
            archive.read(image_path)
        except KeyError:
            image_path = 'img/ship/' + blueprint.get('img') + '_base.png'  # Boss hulls have no glow variant.
        art = Image.open(io.BytesIO(original(image_path))).convert('RGBA')
        source_size = art.size
        mounts = ship_mounts(ship_xml, source_size, 'data/' + name + '.xml')
        klass = blueprint.find('class')
        ship.update(blueprint=ship_id, name=klass.text if klass.text else strings[klass.get('id')],
                    hull=int(blueprint.find('health').get('amount')),
                    reactor=int(blueprint.find('maxPower').get('amount')),
                    automated=int(blueprint.find('crewCount').get('amount')) == 0,
                    crew_count=int(blueprint.find('crewCount').get('amount')),
                    crew_max=int(blueprint.find('crewCount').get('max')),
                    crew_species=blueprint.find('crewCount').get('class') or 'human',
                    missiles=int(blueprint.find('weaponList').get('missiles', '0')) if blueprint.find('weaponList') is not None else 0,
                    hull_bbox={k: int(img.get(k)) for k in ('x', 'y', 'w', 'h')},
                    art_source=image_path, art_source_size=source_size, art_array='imported_' + blueprint.get('img') + '_art',
                    mounts=mounts)
        ship['systems'] = [dict(name=s.tag, room=int(s.get('room')), power=int(s.get('power')),
                                maximum=int(s.get('max', s.get('power'))), start=int(s.get('start', 'true') == 'true'))
                           for s in blueprint.find('systemList')]
        ship['required_capabilities'] = 1 if ship_id == 'AUTO_ASSAULT' else 2 if ship_id in ('REBEL_SKINNY', 'REBEL_FAT') else 0
        weapon_list = blueprint.find('weaponList')
        # Peaceful quest ships (Zoltan Peace) carry no weapon list.
        ship['weapon_pool'] = [n.text for n in autos.find(".//blueprintList[@name='%s']" % weapon_list.get('load')).findall('name')] if weapon_list is not None and weapon_list.get('load') else []
        # Pool members outside the DS catalog (e.g. the alias BOMB_BREACH) are skipped.
        ship['weapon_ids'] = [weapon_ids.index(w) if w != 'MISSILES_2' else 7 for w in ship['weapon_pool'] if w in weapon_ids]
        if ship_id in ('REBEL_SKINNY', 'REBEL_FAT'):
            if rebel_weapon_data['id'] not in ship['weapon_pool']:
                raise ValueError('Selected Rebel laser not in original pool')
            ship['fixed_weapon'] = rebel_weapon_data
            ship['selected_optional_systems'] = ['medbay']
            ship['excluded_optional_systems'] = ['teleporter'] if ship_id == 'REBEL_SKINNY' else ['doors']
            if ship_id == 'REBEL_FAT':
                ship['required_capabilities'] |= 1
                ship['runtime_allocation'] = dict(weapons=1, engines=2, shields=2, oxygen=1, drones=2, medbay=0)
                ship['runtime_note'] = 'Selected Defense Laser Mark I + Combat I from original pools; starting system capacities match source. Fixed loadout, optional medbay selection, sector hull scaling and drone timing are authored.'
        if blueprint.find('droneList') is not None and any(s['name'] == 'drones' for s in ship['systems']):
            ship['drone_pool'] = [n.text for n in autos.find(".//blueprintList[@name='%s']" % blueprint.find('droneList').get('load')).findall('name')]
            if ship_id in ('AUTO_ASSAULT', 'REBEL_FAT') and drone_data['id'] not in ship['drone_pool']:
                raise ValueError('Selected combat drone not in ship pool')
            ship['drone_parts'] = int(blueprint.find('droneList').get('drones', '0'))
        else:
            ship['drone_pool'], ship['drone_parts'] = [], 0
        if len(ship['rooms']) > 32 or len(ship['doors']) > 48:
            raise ValueError('Imported ship exceeds DS geometry bounds')
        room_ids = [room['id'] for room in ship['rooms']]
        if len(set(room_ids)) != len(room_ids) or any(s['room'] not in room_ids for s in ship['systems']):
            raise ValueError('Invalid room/system references')
        for door in ship['doors']:
            if any(door[key] != -1 and door[key] not in room_ids for key in ('room_a', 'room_b')):
                raise ValueError('Invalid door room reference')
        art.thumbnail(CANVAS, Image.Resampling.LANCZOS)
        x, y = (CANVAS[0] - art.width)//2, (CANVAS[1] - art.height)//2
        canvas = Image.new('RGBA', CANVAS)
        canvas.alpha_composite(art, (x, y))
        ship['art_rect'] = dict(x=x, y=y, w=art.width, h=art.height)
        # Ships sharing a hull image (Zoltan Peace / Zoltan Fighter) share one art array.
        if not any(other['art_array'] == ship['art_array'] for other in ships):
            art_header += c_bitmap(ship['art_array'], canvas)
        canvas.save(output / ('imported_' + blueprint.get('img') + '.png'))
        ships.append(ship)

    header = ['/* Original local FTL data. Do not redistribute. */', '#ifndef ENEMY_CATALOG_H', '#define ENEMY_CATALOG_H',
              '#define IMPORTED_ENEMY_COUNT %d' % len(SHIP_IDS), '#define ENEMY_CAP_DRONES 1', '#define ENEMY_CAP_CREW 2',
              'typedef struct { int id,x,y,w,h; } ImportedEnemyRoom;',
              'typedef struct { int x,y,room_a,room_b,vertical; } ImportedEnemyDoor;',
              'typedef struct { const char *name; int room,power,maximum,start; } ImportedEnemySystem;',
              'typedef struct { const char *blueprint,*name; int hull,reactor,automated,missiles,room_count,door_count,system_count,required_capabilities; const ImportedEnemyRoom *rooms; const ImportedEnemyDoor *doors; const ImportedEnemySystem *systems; int hull_x,hull_y,hull_w,hull_h,art_source_width,art_source_height,drone_parts,crew_count,crew_max; const char *crew_species; const int *weapon_ids; int weapon_count; } ImportedEnemy;',
              'typedef struct { const char *id,*name; int power,cooldown_raw,speed,dodge,damage,shots,fire_chance,breach_chance; } ImportedEnemyDrone;']
    header.append('typedef struct { const char *id,*name; int cooldown_ms,damage,shots,power,missiles,shield_piercing,fire_chance,breach_chance; } ImportedEnemyWeapon;')
    for i, ship in enumerate(ships):
        for key, typename, fields in [('rooms','ImportedEnemyRoom',('id','x','y','w','h')),
                                       ('doors','ImportedEnemyDoor',('x','y','room_a','room_b','vertical')),
                                       ('systems','ImportedEnemySystem',('name','room','power','maximum','start'))]:
            header.append('static const %s imported_enemy_%d_%s[] = {' % (typename,i,key))
            if not ship[key]:
                header.append('    {0}, /* Empty list; count is zero. */')
            header.extend('    {' + ','.join(json.dumps(row[k]) for k in fields) + '},' for row in ship[key])
            header.append('};')
        header.append('static const int imported_enemy_%d_weapons[] = {%s};' % (i, ','.join(str(w) for w in ship['weapon_ids']) or '0'))
    header.append('static const ImportedEnemy imported_enemies[IMPORTED_ENEMY_COUNT] = {')
    for i, ship in enumerate(ships):
        values = [json.dumps(int(ship[k]) if isinstance(ship[k], bool) else ship[k]) for k in ('blueprint','name','hull','reactor','automated','missiles')]
        values += [str(len(ship[k])) for k in ('rooms','doors','systems')]
        values += [str(ship['required_capabilities'])]
        values += ['imported_enemy_%d_%s' % (i,k) for k in ('rooms','doors','systems')]
        values += [str(ship['hull_bbox'][k]) for k in ('x','y','w','h')]
        values += [str(n) for n in ship['art_source_size']]
        values += [str(ship[k]) for k in ('drone_parts','crew_count','crew_max')]
        values += [json.dumps(ship['crew_species']), 'imported_enemy_%d_weapons' % i, str(len(ship['weapon_ids']))]
        header.append('    {' + ','.join(values) + '},')
    header += ['};', '/* V68 original weapon mounts (hull-image pixels; slot i uses mount i, artillery 4+i). */',
               '#define IMPORTED_ENEMY_MAX_MOUNTS 8',
               'typedef struct { int count; int xy[IMPORTED_ENEMY_MAX_MOUNTS][2]; } ImportedEnemyMounts;',
               'static const ImportedEnemyMounts imported_enemy_mounts[IMPORTED_ENEMY_COUNT] = {']
    header += ['    {%d,{%s}},' % (len(ship['mounts']), ','.join('{%d,%d}' % tuple(m) for m in ship['mounts'])) for ship in ships]
    header += ['};', 'static const ImportedEnemyDrone imported_combat_drone = {' + ','.join(json.dumps(drone_data[k]) for k in ('id','name','power','cooldown_raw','speed','dodge','damage','shots','fire_chance','breach_chance')) + '};', '#endif', '']
    header.insert(-2, 'static const ImportedEnemyWeapon imported_rebel_weapon = {' + ','.join(json.dumps(rebel_weapon_data[k]) for k in ('id','name','cooldown_ms','damage','shots','power','missiles','shield_piercing','fire_chance','breach_chance')) + '};')
    art_header += ['static const uint16_t *const imported_enemy_art[%d] = { %s };' % (len(ships), ', '.join(ship['art_array'] for ship in ships)),
                   'static const ImportedEnemyArtRect imported_enemy_art_rect[%d] = {' % len(ships)]
    art_header += ['    {' + ','.join(str(ship['art_rect'][k]) for k in ('x','y','w','h')) + '},' for ship in ships]
    art_header += ['};', '#endif', '']
    (output/'enemy_catalog.h').write_text('\n'.join(header),encoding='utf-8')
    (output/'enemy_catalog_art.h').write_text('\n'.join(art_header),encoding='utf-8')
    catalog = dict(ships=ships, combat_drone=drone_data, rebel_weapon=rebel_weapon_data, weapon_properties=properties, sources=list(sources.values()),
                   conversion=dict(canvas=CANVAS,filter='Lanczos',format='RGB555 bit15 opaque; zero transparent',alpha_threshold=96),
                   note='Original data only. Runtime scaling, fixed equipment subset and drone timings are demake choices. Required capabilities must be implemented before enabling a hull. Optional cloak remains unselected. Raw cooldown is not a verified firing period.')
    (output/'enemy_catalog.json').write_text(json.dumps(catalog,indent=2),encoding='utf-8')
    import_species(archive_path, output)
    from weapon_coverage import audit
    audit(archive_path, output)
    print('Imported %d base-game hulls, %d bytes of RGB555 art; Auto-Assault requires combat drones.' % (len(ships),len(ships)*CANVAS[0]*CANVAS[1]*2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path, nargs='?')
    parser.add_argument('--output',type=Path,default=Path(__file__).resolve().parents[1]/'generated')
    args = parser.parse_args()
    archive = args.archive or Path(json.loads((args.output/'manifest.json').read_text())['archive'])
    import_catalog(archive,args.output)


if __name__ == '__main__':
    main()
