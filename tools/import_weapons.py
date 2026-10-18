#!/usr/bin/env python3
"""Import the base-game FTL weapon list (data/blueprints.xml) for the DS catalog.

Indices 0..6 are the historical V20 catalog and must never move (saves store
them). Index 7 is the enemy-only Artemis variant (GAME_ENEMY_ARTEMIS). New
weapons follow. Advanced Edition-only weapons (flak, chain/charge guns, stun)
live in dlcBlueprints.xml and are excluded from this base-game build.
Writes generated/weapon_catalog.h, weapon_properties.h and weapon_catalog.json.
"""
import argparse
import hashlib
import json
from pathlib import Path
from import_ftl import PkgArchive, xml_root

# (blueprint id, DS short name of at most 8 characters, enemy-only)
WEAPONS = [
    ('MISSILES_2_PLAYER', 'ARTEMIS', 0), ('LASER_BURST_3', 'BURST2', 0), ('LASER_BURST_1', 'BASIC', 0),
    ('LASER_HEAVY_1', 'HEAVY', 0), ('ION_1', 'ION', 0), ('BEAM_2', 'HALBERD', 0), ('BOMB_1', 'BOMB', 0),
    ('MISSILES_2', 'ARTEMIS', 1),
    ('LASER_BURST_2', 'DUAL', 0), ('LASER_BURST_2_A', 'BURST1', 0), ('LASER_BURST_5', 'BURST3', 0),
    ('LASER_HEAVY_2', 'HEAVY2', 0), ('LASER_HULL_1', 'HULL1', 0), ('LASER_HULL_2', 'HULL2', 0),
    ('ION_2', 'HVYION', 0), ('ION_4', 'ION2', 0),
    ('MISSILES_1', 'LETO', 0), ('MISSILES_3', 'HERMES', 0), ('MISSILES_BURST', 'PEGASUS', 0),
    ('MISSILES_BREACH', 'BREACH', 0), ('MISSILES_HULL', 'SMASHER', 0),
    ('BEAM_1', 'MINIBEAM', 0), ('BEAM_LONG', 'PIKE', 0), ('BEAM_3', 'GLAIVE', 0), ('BEAM_FIRE', 'FIREBEAM', 0),
    ('BEAM_BIO', 'ANTIBIO', 0), ('BEAM_HULL', 'HULLBEAM', 0),
    ('BOMB_BREACH_1', 'BRBOMB1', 0), ('BOMB_BREACH_2', 'BRBOMB2', 0), ('BOMB_FIRE', 'FIREBOMB', 0),
    ('BOMB_ION', 'IONBOMB', 0), ('BOMB_HEAL', 'HEALBOMB', 0), ('BOMB_LOCK', 'LOCKBOMB', 0),
    ('CRYSTAL_BURST_1', 'CRYSTAL1', 0), ('CRYSTAL_BURST_2', 'CRYSTAL2', 0),
    ('CRYSTAL_HEAVY_1', 'HVYCRYS1', 0), ('CRYSTAL_HEAVY_2', 'HVYCRYS2', 0),
    # V58: Rock B's unique laser and the Federation Artillery Beam (system-mounted, never in a slot).
    ('LASER_HEAVY_1_SP', 'HEAVYSP', 0), ('ARTILLERY_FED', 'ARTILLRY', 1),
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path)
    args = parser.parse_args()
    output = Path(__file__).resolve().parents[1] / 'generated'
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    archive = PkgArchive(archive_path)
    payload = archive.read('data/blueprints.xml')
    base = xml_root(payload)
    strings = {n.get('name'): n.text for n in xml_root(archive.read('data/text_blueprints.xml')).iter('text')}
    rows = []
    for wid, short, enemy_only in WEAPONS:
        w = base.find(".//weaponBlueprint[@name='%s']" % wid)
        if w is None:
            raise ValueError('Missing base weapon ' + wid)
        title = w.find('title')
        name = title.text if title is not None and title.text else strings.get(title.get('id') if title is not None else '', wid)
        num = lambda tag, d='0': int(float(w.findtext(tag, d) or d))
        rows.append(dict(id=wid, name=name, short=short, type=w.findtext('type'),
                         cooldown_ms=round(float(w.findtext('cooldown')) * 1000), power=num('power'), damage=num('damage'),
                         shots=num('shots', '1') if w.findtext('type') != 'BEAM' else 0, missiles=num('missiles'),
                         shield_piercing=num('sp'), ion=num('ion'), cost=num('cost'), speed=num('speed'),
                         fire_chance=num('fireChance'), breach_chance=num('breachChance'), beam_length=num('length'),
                         rarity=num('rarity'), hull_bust=num('hullBust'), lockdown=num('lockdown'),
                         system_damage=num('sysDamage'), personnel_damage=num('persDamage'),
                         has_system_damage=int(w.find('sysDamage') is not None),
                         has_personnel_damage=int(w.find('persDamage') is not None), enemy_only=enemy_only))
    count = len(rows)
    fields = ('id', 'name', 'short', 'type', 'cooldown_ms', 'power', 'damage', 'shots', 'missiles', 'shield_piercing', 'ion',
              'cost', 'speed', 'fire_chance', 'breach_chance', 'beam_length', 'rarity', 'hull_bust', 'lockdown', 'enemy_only')
    header = ['/* Original local base-game weapon data (data/blueprints.xml); raw chance values. Do not redistribute. */',
              '#ifndef WEAPON_CATALOG_H', '#define WEAPON_CATALOG_H', '#define IMPORTED_WEAPON_COUNT %d' % count,
              'typedef struct { const char *id,*name,*short_name,*type; int cooldown_ms,power,damage,shots,missiles,shield_piercing,ion,cost,speed,fire_chance,breach_chance,beam_length,rarity,hull_bust,lockdown,enemy_only; } ImportedWeapon;',
              'static const ImportedWeapon imported_weapon_catalog[IMPORTED_WEAPON_COUNT] = {']
    header += ['    {' + ','.join(json.dumps(r[k]) for k in fields) + '},' for r in rows]
    header += ['};', '#endif', '']
    (output / 'weapon_catalog.h').write_text('\n'.join(header), encoding='utf-8')
    pfields = ('id', 'fire_chance', 'breach_chance', 'system_damage', 'personnel_damage', 'has_system_damage', 'has_personnel_damage')
    props = ['/* Original local XML fields. Raw chances; absent overrides have explicit presence flags. */',
             '#ifndef WEAPON_PROPERTIES_H', '#define WEAPON_PROPERTIES_H',
             'typedef struct { const char *id; int fire_chance,breach_chance,system_damage,personnel_damage,has_system_damage,has_personnel_damage; } ImportedWeaponProperties;',
             'static const ImportedWeaponProperties imported_weapon_properties[%d] = {' % count]
    props += ['    {' + ','.join(json.dumps(r[k]) for k in pfields) + '},' for r in rows]
    props += ['};', '#endif', '']
    (output / 'weapon_properties.h').write_text('\n'.join(props), encoding='utf-8')
    (output / 'weapon_catalog.json').write_text(json.dumps(dict(
        weapons=rows, source='data/blueprints.xml', sha256=hashlib.sha256(payload).hexdigest(),
        note='Stable indices 0..6 (V20), 7 enemy-only Artemis, base-game additions after. AE weapons excluded.'), indent=2), encoding='utf-8')
    # V55: base-game augments (blueprints.xml augBlueprint, stable order). AE-only
    # augments live in dlcBlueprints.xml and are excluded.
    augs = []
    for b in base.iter('augBlueprint'):
        title = b.find('title')
        name = title.text if title is not None and title.text else strings.get(title.get('id') if title is not None else '', b.get('name'))
        value = b.findtext('value')
        augs.append(dict(id=b.get('name'), name=name, cost=int(b.findtext('cost', '0')), rarity=int(b.findtext('rarity', '0')),
                         value=int(round(float(value) * 100)) if value not in (None, '') else 0,
                         stackable=int(b.findtext('stackable', 'false') == 'true')))
    afields = ('id', 'name', 'cost', 'rarity', 'value', 'stackable')
    aug = ['/* Original local base-game augments (data/blueprints.xml); value in percent. Do not redistribute. */',
           '#ifndef AUGMENT_CATALOG_H', '#define AUGMENT_CATALOG_H', '#define IMPORTED_AUGMENT_COUNT %d' % len(augs),
           'typedef struct { const char *id,*name; int cost,rarity,value,stackable; } ImportedAugment;',
           'static const ImportedAugment imported_augments[IMPORTED_AUGMENT_COUNT] = {']
    aug += ['    {' + ','.join(json.dumps(r[k]) for k in afields) + '},' for r in augs]
    aug += ['};', '#endif', '']
    (output / 'augment_catalog.h').write_text(chr(10).join(aug), encoding='utf-8')
    (output / 'augment_catalog.json').write_text(json.dumps(dict(augments=augs), indent=2), encoding='utf-8')
    print('Wrote %d weapons, %d augments' % (count, len(augs)))


if __name__ == '__main__':
    main()
