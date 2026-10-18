#!/usr/bin/env python3
"""Audit installed non-DLC blueprint weapon definitions; never changes playable pools."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
from import_ftl import PkgArchive, xml_root

SELECTED = ('MISSILES_2_PLAYER','LASER_BURST_3','LASER_BURST_1','LASER_HEAVY_1','ION_1','BEAM_2','BOMB_1')
ENEMY_DEPENDENCIES = ('DRONE_LASER',)
FIELDS = ('type','damage','sysDamage','persDamage','ion','shots','power','cooldown','sp','fireChance','breachChance','stunChance','missiles','hullBust','lockdown','length','speed','cost','rarity')

def audit(archive_path, output):
    archive_path, output = Path(archive_path).resolve(), Path(output).resolve()
    if output == archive_path.parent or archive_path.is_relative_to(output):
        raise ValueError('Output must be separate from installed archive')
    archive = PkgArchive(archive_path)
    raw = archive.read('data/blueprints.xml')
    root = xml_root(raw)
    strings = {e.get('name'):e.text for e in xml_root(archive.read('data/text_blueprints.xml')).iter('text')}
    entries=[]
    references=[]
    for weapon in root.findall('.//weaponBlueprint'):
        weapon_id=weapon.get('name')
        if not weapon_id:
            references.append((weapon.text or '').strip())
            continue
        title=weapon.find('title')
        name=(title.text or strings.get(title.get('id'))) if title is not None else None
        fields={key:weapon.findtext(key) for key in FIELDS}
        def number(key):return float(fields[key]) if fields[key] is not None else 0
        family='ion' if fields['type']=='LASER' and number('ion') else {'LASER':'laser','MISSILES':'missile','BEAM':'beam','BOMB':'bomb'}.get(fields['type'],'unknown')
        mechanics=[family+'_delivery','equipment_and_effect_validation']
        if number('shots')>1:mechanics.append('multi_shot_queue')
        if number('sp'):mechanics.append('shield_piercing')
        if number('ion'):mechanics.append('ion_lockout')
        if number('fireChance'):mechanics.append('ignition')
        if number('breachChance'):mechanics.append('breach_generation')
        if fields['sysDamage'] is not None:mechanics.append('explicit_system_damage')
        if fields['persDamage'] is not None:mechanics.append('crew_healing' if number('persDamage')<0 else 'explicit_crew_damage')
        if number('hullBust'):mechanics.append('empty_room_hull_multiplier')
        if number('lockdown'):mechanics.append('weapon_lockdown')
        if weapon_id.startswith('ARTILLERY_'):mechanics.append('artillery_controller')
        if weapon_id.startswith('DRONE_'):mechanics.append('drone_controller')
        status='selected_player_catalog' if weapon_id in SELECTED else 'selected_enemy_dependency' if weapon_id in ENEMY_DEPENDENCIES else 'not_enabled'
        entries.append(dict(id=weapon_id,title=name,family=family,raw_fields=fields,
                            status=status,catalog_index=SELECTED.index(weapon_id) if weapon_id in SELECTED else None,
                            required_mechanics=mechanics,
                            historical_base_game_eligibility='unreviewed; file origin alone is insufficient',
                            player_obtainability='unreviewed; definition or rarity is not proof',
                            excluded_or_unreviewed_properties=['stunChance'] if number('stunChance') else []))
    ids=[row['id'] for row in entries]
    if len(ids)!=len(set(ids)):raise ValueError('Duplicate named weapon definition')
    if any(w not in ids for w in SELECTED+ENEMY_DEPENDENCIES):raise ValueError('Selected weapon missing')
    catalog_path=output/'weapon_catalog.json'
    if catalog_path.exists():
        imported=[w['id'] for w in json.loads(catalog_path.read_text(encoding='utf-8'))['weapons']]
        if imported[:len(SELECTED)]!=list(SELECTED):raise ValueError('Audit selection and generated runtime catalog disagree')
    result=dict(source=dict(archive=str(archive_path),file='data/blueprints.xml',sha256=hashlib.sha256(raw).hexdigest()),
                scope='Named definitions in installed data/blueprints.xml only, excluding unnamed weapon references. This file can contain post-AE revisions and internal weapons; neither historical base eligibility nor player availability is inferred.',
                counts=dict(named_definitions=len(entries),unnamed_reference_occurrences=len(references),by_status=dict(Counter(r['status'] for r in entries)),by_family=dict(Counter(r['family'] for r in entries))),
                semantics='Mechanic tags come from raw fields/prefixes and express requirements to assess, not proof the mechanism is correct or the weapon should be enabled. Runtime support is explicitly selected; others remain out of the playable pool.',
                references=references,weapons=entries)
    output.mkdir(parents=True,exist_ok=True)
    (output/'weapon_coverage.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result['counts'],sort_keys=True))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive',type=Path,nargs='?')
    parser.add_argument('--output',type=Path,default=Path(__file__).resolve().parents[1]/'generated')
    args=parser.parse_args()
    archive=args.archive or Path(json.loads((args.output/'manifest.json').read_text(encoding='utf-8'))['archive'])
    audit(archive,args.output)
