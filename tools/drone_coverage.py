#!/usr/bin/env python3
"""Audit local original drone definitions without changing runtime content pools."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
from import_ftl import PkgArchive, xml_root

ENEMY_DEPENDENCIES = ('COMBAT_1',)
# V56: every base-game player drone (rarity > 0). Order = DS catalog id; 0 and 1
# keep their save ids. Anti-Combat, Fire Beam and Ion Intruder drones are
# Advanced Edition and stay excluded.
PLAYER_SELECTED = ('COMBAT_1', 'DEFENSE_1', 'COMBAT_2', 'DEFENSE_2', 'COMBAT_BEAM',
                   'SHIP_REPAIR', 'REPAIR', 'BATTLE', 'BOARDER')
KIND = {'COMBAT_1': 0, 'DEFENSE_1': 1, 'COMBAT_2': 0, 'DEFENSE_2': 1, 'COMBAT_BEAM': 2,
        'SHIP_REPAIR': 3, 'REPAIR': 4, 'BATTLE': 5, 'BOARDER': 6}
FIELDS = ('type', 'power', 'cooldown', 'dodge', 'speed', 'cost', 'bp', 'rarity',
          'level', 'target', 'weaponBlueprint', 'droneImage', 'image', 'iconImage')
REQUIREMENTS = {
    'COMBAT': ['orbital_movement', 'automatic_enemy_targeting', 'dependent_weapon_delivery'],
    'DEFENSE': ['orbital_movement', 'incoming_projectile_selection', 'interception',
                'target_and_level_semantics'],
    'SHIP_REPAIR': ['orbital_movement', 'finite_hull_repair_lifecycle'],
    'REPAIR': ['interior_drone_actor', 'pathfinding', 'system_repair', 'firefighting'],
    'BATTLE': ['interior_drone_actor', 'pathfinding', 'hostile_crew_combat'],
    'BOARDER': ['boarding_delivery', 'interception', 'breach_creation',
                'interior_drone_actor', 'hostile_crew_combat', 'system_sabotage'],
}


def raw_element(element):
    """Preserve attributes, absent children and repeated children without defaults."""
    return dict(tag=element.tag, attributes=dict(element.attrib),
                text=(element.text or '').strip(),
                children=[raw_element(child) for child in element])


def audit(archive_path, output):
    archive_path, output = Path(archive_path).resolve(), Path(output).resolve()
    installed = archive_path.parent
    if output == installed or output.is_relative_to(installed) or archive_path.is_relative_to(output):
        raise ValueError('Output must be separate from installed archive directory')
    archive = PkgArchive(archive_path)
    blueprint_raw = archive.read('data/blueprints.xml')
    strings_raw = archive.read('data/text_blueprints.xml')
    root = xml_root(blueprint_raw)
    strings = {e.get('name'): e.text for e in xml_root(strings_raw).iter('text')}
    weapons = {e.get('name'): e for e in root.findall('.//weaponBlueprint') if e.get('name')}
    entries, references = [], []
    for drone in root.findall('.//droneBlueprint'):
        drone_id = drone.get('name')
        if not drone_id:
            references.append((drone.text or '').strip())
            continue
        title = drone.find('title')
        name = (title.text or strings.get(title.get('id'))) if title is not None else None
        fields = {key: drone.findtext(key) for key in FIELDS}
        weapon_id = fields['weaponBlueprint']
        if weapon_id and weapon_id not in weapons:
            raise ValueError('Missing dependent weapon: ' + weapon_id)
        mechanics = ['drone_control_installation_and_power', 'drone_parts_and_deployment',
                     'equipment_management', 'save_and_encounter_lifecycle']
        mechanics += REQUIREMENTS.get(fields['type'], ['unrecognized_type_review'])
        entries.append(dict(
            id=drone_id, title=name, raw_fields=fields, raw_definition=raw_element(drone),
            dependent_weapon=raw_element(weapons[weapon_id]) if weapon_id else None,
            status='selected_player_and_enemy' if drone_id in PLAYER_SELECTED and drone_id in ENEMY_DEPENDENCIES else 'selected_player' if drone_id in PLAYER_SELECTED else 'selected_enemy_dependency' if drone_id in ENEMY_DEPENDENCIES else 'not_enabled',
            player_enabled=drone_id in PLAYER_SELECTED,
            required_mechanics=mechanics,
            historical_base_game_eligibility='unreviewed; file origin alone is insufficient',
            player_obtainability='Authored DS store package and schematic offer' if drone_id in PLAYER_SELECTED else 'unreviewed; definition or rarity is not proof'))
    ids = [row['id'] for row in entries]
    if len(ids) != len(set(ids)):
        raise ValueError('Duplicate named drone definition')
    if any(drone_id not in ids for drone_id in ENEMY_DEPENDENCIES):
        raise ValueError('Selected enemy drone missing')
    control = root.find(".//systemBlueprint[@name='drones']")
    if control is None:
        raise ValueError('Missing drone control system')
    result = dict(
        sources=[dict(archive=str(archive_path), file=name, sha256=hashlib.sha256(raw).hexdigest())
                 for name, raw in [('data/blueprints.xml', blueprint_raw),
                                   ('data/text_blueprints.xml', strings_raw)]],
        scope='Named drone definitions in installed data/blueprints.xml only. Internal definitions and post-AE revisions may occur here. No historical eligibility or player obtainability is inferred.',
        semantics='Raw text, attributes and child order are retained. Absent fields remain null. Cooldown, speed, dodge, target and level units/defaults need behavior verification; no engine timing is inferred. Requirement tags are review tasks, not implementation claims.',
        counts=dict(named_definitions=len(entries), unnamed_reference_occurrences=len(references),
                    by_status=dict(Counter(row['status'] for row in entries)),
                    by_type=dict(Counter(row['raw_fields']['type'] for row in entries))),
        drone_control=dict(raw_definition=raw_element(control),
                           raw_upgrade_cost_sequence=[n.text for n in control.findall('upgradeCost/level')],
                           note='Sequence preserved as written, without claiming index-to-level semantics or installation package contents.'),
        runtime_dependency_note='COMBAT_1 is the selected player store drone and Auto-Assault enemy dependency. Its ten-second firing interval, fixed projectile delivery and economy are authored; selection does not establish original orbit or vulnerability fidelity. DEFENSE_1 is selected for the player with authored one-second instantaneous missile interception; raw cooldown units are not inferred. This audit only records runtime selection.',
        references=references, drones=entries)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'drone_coverage.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    rows = {row['id']: row for row in entries}
    header = ['#ifndef FTL_DS_DRONE_CATALOG_H', '#define FTL_DS_DRONE_CATALOG_H',
              '/* kind: 0 combat, 1 defense, 2 beam, 3 hull repair, 4 system repair, 5 anti-personnel, 6 boarding. */',
              'typedef struct { const char *id,*name; int power,cost,rarity,kind,speed; } ImportedPlayerDrone;',
              'static const ImportedPlayerDrone imported_player_drones[%d] = {' % len(PLAYER_SELECTED)]
    for key in PLAYER_SELECTED:
        row = rows[key]
        header.append('    {' + ','.join([json.dumps(key), json.dumps(row['title']), str(int(row['raw_fields']['power'])), str(int(row['raw_fields']['cost'])),
                                     str(int(row['raw_fields']['rarity'])), str(KIND[key]), str(int(row['raw_fields']['speed'] or 0))]) + '},')
    header.insert(2, '#define IMPORTED_PLAYER_DRONE_COUNT %d' % len(PLAYER_SELECTED))
    (output / 'drone_catalog.json').write_text(json.dumps({'drones': [dict(id=k, name=rows[k]['title'], kind=KIND[k]) for k in PLAYER_SELECTED]}, indent=2), encoding='utf-8')
    header += ['};', '#endif', '']
    (output / 'drone_catalog.h').write_text('\n'.join(header), encoding='utf-8')
    print(json.dumps(result['counts'], sort_keys=True))
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path, nargs='?')
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'generated')
    args = parser.parse_args()
    archive = args.archive or Path(json.loads((args.output / 'manifest.json').read_text(encoding='utf-8'))['archive'])
    audit(archive, args.output)
