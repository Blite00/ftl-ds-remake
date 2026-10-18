import json
from pathlib import Path
import sys
import unittest
import xml.etree.ElementTree as ET
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from import_enemy_weapons import payload,header
XML='<weaponBlueprint name="MISSILES_2"><type>MISSILES</type><title>Artemis</title><damage>2</damage><shots>1</shots><missiles>1</missiles><sp>5</sp><cooldown>10</cooldown><power>2</power><stunChance>1</stunChance></weaponBlueprint>'
class EnemyWeaponImportTest(unittest.TestCase):
 def test_payload_and_override_presence(self):
  row=payload(ET.fromstring(XML),{});self.assertEqual((row['cooldown_ms'],row['power'],row['stun_chance']),(10000,2,1));self.assertEqual(row['has_system_damage'],0)
  row=payload(ET.fromstring(XML.replace('</weaponBlueprint>','<sysDamage>0</sysDamage><persDamage>2</persDamage></weaponBlueprint>')),{});self.assertEqual((row['has_system_damage'],row['system_damage'],row['has_personnel_damage'],row['personnel_damage']),(1,0,1,2))
 def test_fail_closed(self):
  variants=[XML.replace('<damage>2</damage>',''),XML.replace('<damage>2</damage>','<damage>2</damage><damage>3</damage>'),XML.replace('<power>2','<power>9'),XML.replace('MISSILES_2','MISSILES_2_PLAYER'),XML.replace('<cooldown>10','<cooldown>NaN'),XML.replace('<cooldown>10','<cooldown>0'),XML.replace('<type>MISSILES','<type>BEAM'),XML.replace('</weaponBlueprint>','<lockdown>1</lockdown></weaponBlueprint>'),XML.replace('<sp>5</sp>','<sp><x/></sp>'),XML.replace('<stunChance>1','<stunChance>11')]
  for xml in variants:
   with self.subTest(xml=xml),self.assertRaises(ValueError):payload(ET.fromstring(xml),{})
 def test_local_header_is_reproducible_and_enemy_only(self):
  root=Path(__file__).resolve().parents[1];data=json.loads((root/'generated/enemy_weapon_variants.json').read_text(encoding='utf-8'))
  self.assertEqual((root/'generated/enemy_weapon_variants.h').read_text(encoding='utf-8'),header(data));self.assertEqual([v['id'] for v in data['variants']],['MISSILES_2']);self.assertEqual(len(data['sources']),2)
if __name__=='__main__':unittest.main()
