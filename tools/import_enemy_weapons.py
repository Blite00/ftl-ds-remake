#!/usr/bin/env python3
"""Import reviewed enemy-only payloads without changing the player catalog."""
import argparse
from decimal import Decimal, InvalidOperation
import hashlib
import json
from pathlib import Path
from import_ftl import PkgArchive, xml_root

SOURCE = "data/blueprints.xml"
TEXT_SOURCE = "data/text_blueprints.xml"
SELECTED = ("MISSILES_2",)
FIELDS = ("damage", "shots", "missiles", "sp", "ion", "sysDamage", "persDamage", "fireChance", "breachChance", "stunChance", "power")
COSMETIC = {"tip", "title", "short", "desc", "tooltip", "cost", "bp", "rarity", "image", "launchSounds", "hitShipSounds", "hitShieldSounds", "missSounds", "weaponArt", "iconImage", "speed"}

def scalar(e, name, required=False):
    entries=e.findall(name)
    if len(entries)>1 or (required and len(entries)!=1):raise ValueError("missing/duplicate "+name)
    if not entries:return None
    field=entries[0]
    if field.attrib or len(field):raise ValueError("nested/attributed "+name)
    return (field.text or "").strip()

def integer(e,name,low=0,high=30,required=False):
    value=scalar(e,name,required)
    if value is None:return 0
    try:result=int(value)
    except ValueError:raise ValueError("invalid "+name)
    if str(result)!=value or not low<=result<=high:raise ValueError("out of bounds "+name)
    return result

def payload(e,strings):
    if e.tag!="weaponBlueprint" or set(e.attrib)!={"name"} or e.get("name") not in SELECTED:raise ValueError("unreviewed enemy variant")
    for child in e:
        if child.tag not in set(FIELDS)|COSMETIC|{"type","cooldown"}:raise ValueError("unsupported field "+child.tag)
    if scalar(e,"type",True)!="MISSILES":raise ValueError("unsupported weapon family")
    try:cooldown=Decimal(scalar(e,"cooldown",True))*1000
    except (InvalidOperation,TypeError):raise ValueError("invalid cooldown")
    if not cooldown.is_finite() or cooldown!=cooldown.to_integral_value() or not 1<=cooldown<=300000:raise ValueError("out of bounds cooldown")
    titles=e.findall("title")
    if len(titles)!=1 or len(titles[0]) or set(titles[0].attrib)-{"id"}:raise ValueError("invalid title")
    title=titles[0];name=strings.get(title.get("id")) if title.get("id") else title.text
    if not name or not name.strip():raise ValueError("missing title string")
    return dict(id=e.get("name"),name=name.strip(),type="MISSILES",cooldown_ms=int(cooldown),
        power=integer(e,"power",1,8,True),damage=integer(e,"damage",0,30,True),shots=integer(e,"shots",1,8,True),
        missiles=integer(e,"missiles",1,8,True),shield_piercing=integer(e,"sp",0,10,True),ion=integer(e,"ion",0,8),
        has_system_damage=int(e.find("sysDamage") is not None),system_damage=integer(e,"sysDamage",0,30),
        has_personnel_damage=int(e.find("persDamage") is not None),personnel_damage=integer(e,"persDamage",-20,20),
        fire_chance=integer(e,"fireChance",0,10),breach_chance=integer(e,"breachChance",0,10),stun_chance=integer(e,"stunChance",0,10))

def build(archive):
    raw=archive.read(SOURCE);texts=archive.read(TEXT_SOURCE)
    strings={e.get("name"):e.text for e in xml_root(texts).iter("text")}
    rows=[]
    for ident in SELECTED:
        matches=[e for e in xml_root(raw).iter("weaponBlueprint") if e.get("name")==ident]
        if len(matches)!=1:raise ValueError("missing/duplicate enemy variant "+ident)
        rows.append(payload(matches[0],strings))
    return dict(schema=1,variants=rows,sources=[dict(path=path,sha256=hashlib.sha256(data).hexdigest(),size=len(data)) for path,data in ((SOURCE,raw),(TEXT_SOURCE,texts))],
        adaptations=["Enemy-only data; no player catalog/store/loot expansion", "Raw fire/breach chance converted by existing DS ten-percent steps", "Absent system damage defaults to hull damage; absent personnel damage uses existing DS hull-times-five rule", "Installed stunChance retained as metadata; Advanced Edition stun effects remain deferred", "Source weapon-list membership does not establish original pairing/spawn weights"])

def header(data):
    row=data["variants"][0]
    return "\n".join(["/* Original local source payload. Do not redistribute. */", "#ifndef ENEMY_WEAPON_VARIANTS_H", "#define ENEMY_WEAPON_VARIANTS_H",
      "typedef struct { const char *id,*name,*type; int cooldown_ms,power,damage,shots,missiles,shield_piercing,ion,has_system_damage,system_damage,has_personnel_damage,personnel_damage,fire_chance,breach_chance,stun_chance; } ImportedEnemyWeaponVariant;",
      "static const ImportedEnemyWeaponVariant imported_enemy_artemis = {"+",".join(json.dumps(v) for v in row.values())+"};", "#endif", ""])

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("archive",type=Path);parser.add_argument("--output",type=Path,default=Path(__file__).resolve().parents[1]/"generated");args=parser.parse_args()
    archive=args.archive.resolve();output=args.output.resolve()
    if output.is_relative_to(archive.parent) or archive.is_relative_to(output):raise ValueError("output must be outside installation")
    data=build(PkgArchive(archive));output.mkdir(parents=True,exist_ok=True)
    (output/"enemy_weapon_variants.json").write_text(json.dumps(data,indent=2)+"\n",encoding="utf-8")
    (output/"enemy_weapon_variants.h").write_text(header(data),encoding="utf-8")
    print("Imported enemy-only MISSILES_2: 10s, power2, damage2, finite missile ammunition")

if __name__=="__main__":main()
