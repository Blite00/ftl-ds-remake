#!/usr/bin/env python3
"""Compile closed, explicitly enabled original event trees from a read-only FTL archive.

Generated original text/data stays local. Unknown effects, requirements, attributes,
references and cycles fail closed; no branch is discarded to make a root compile.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from import_ftl import PkgArchive, xml_root

SELECTED = ("REFUEL_STATION", "REPAIR_STATION", "SELL_MISSILES_STATION",
            "SELL_DRONES_STATION", "FUEL_FOR_DRONE", "NO_FUEL_REFUGEE_DAMAGED", "AUTO_REFUEL_STATION", "SQUAT_REFUEL_STATION", "AUTO_WARNING", "FIND_WEAPON", "AUTO_CIVILIAN", "REBEL")
# File names mean base-source containers, not verified historical pre-AE contents.
BASE_FILES = tuple("data/" + n + ".xml" for n in ("events", "events_engi",
 "events_zoltan", "events_mantis", "events_fuel", "events_crystal", "events_rebel",
 "events_slug", "events_pirate", "events_nebula", "events_rock", "events_boss", "newEvents"))
KINDS = {"fuel": 0, "scrap": 1, "missiles": 2, "drones": 3}
SPECIES = {"human": 0, "engi": 1, "mantis": 2, "rock": 3, "slug": 4, "energy": 5, "crystal": 6}
# This is an explicit DS payout approximation, NOT recovered engine reward math.
AUTO_REWARDS = {("fuel", "MED"): [(0, 3, 6), (1, 15, 25)],
                ("weapon", "LOW"): [(5, 0, 6)],
                ("fuel", "HIGH"): [(0, 5, 8), (1, 20, 30)],
                ("scrap_only", "LOW"): [(1, 10, 20)],
                ("stuff", "RANDOM"): [(0, 1, 4), (2, 1, 4), (3, 1, 3)]}
SHIP_FILES = ("data/events_ships.xml", "data/autoBlueprints.xml", "data/dlcPirateBlueprints.xml")
ENEMY_BLUEPRINTS = {"AUTO_BASIC": "ENEMY_SCOUT", "AUTO_ASSAULT": "ENEMY_ASSAULT",
                    "REBEL_FAT": "ENEMY_REBEL_FAT", "REBEL_SKINNY": "ENEMY_REBEL"}
# V53: every imported hull (generated/enemy_catalog.json order) is an event ship;
# indices 4.. are encoded ENEMY_SHIP_BASE+index for the generic ENEMY_SHIP setup.
_catalog = json.loads((Path(__file__).resolve().parents[1] / "generated" / "enemy_catalog.json").read_text())
for _i, _ship in enumerate(_catalog["ships"]):
    if _i >= 4:
        ENEMY_BLUEPRINTS[_ship["blueprint"]] = "ENEMY_SHIP_BASE+%d" % _i
_weapons = [w["id"] for w in json.loads((Path(__file__).resolve().parents[1] / "generated" / "weapon_catalog.json").read_text())["weapons"]]
WEAPON_IDS = {w: i for i, w in enumerate(_weapons) if i != 7}
# V55 augments (generated/augment_catalog.json order = enum GameAugment).
AUGMENT_IDS = {a["id"]: i for i, a in enumerate(json.loads((Path(__file__).resolve().parents[1] / "generated" / "augment_catalog.json").read_text())["augments"])}
# V56 drones (generated/drone_catalog.json order = DS drone catalog id).
DRONE_IDS = {d["id"]: i for i, d in enumerate(json.loads((Path(__file__).resolve().parents[1] / "generated" / "drone_catalog.json").read_text())["drones"])}
# Requirement kinds: systems 0..4 main, 5 sensors, 6 doors, 7 teleporter, 8 drones, 9 pilot, 10 cloaking.
REQ_SYSTEMS = {"shields": 0, "engines": 1, "weapons": 2, "oxygen": 3, "medbay": 4, "sensors": 5, "doors": 6,
               "teleporter": 7, "drones": 8, "pilot": 9, "cloaking": 10}
# Blue options the base-game DS player can never meet (yet): Advanced Edition
# systems/crew, plus augments and drones not built yet. The choice stays visible
# and disabled, exactly like an unmet original requirement.
# Advanced Edition-only blue options are dropped from base-game events.
AE_REQS = {"hacking", "mind", "clonebay", "anaerobic", "BACKUP_DNA"}
REQ_NEVER = {"LIFE_SCANNER", "FLEET_DISTRACTION"}
# V62 status effects: system power limits for the beacon. Advanced Edition systems
# (hacking, clone bay, mind control, battery) are no-ops in the base game.
STATUS_TYPES = {"limit": 0, "divide": 1, "loss": 2, "clear": 3}
STATUS_SYSTEMS = {"shields": 0, "engines": 1, "engine": 1, "weapons": 2, "oxygen": 3, "medbay": 4, "sensors": 6, "doors": 7}
STATUS_IGNORED = {"hacking", "clonebay", "mind", "battery", "pilot"}
# V62 homeworld specials from sector_data.xml (<event name min=1 max=1/> in the
# *_HOME sectors): the starts of the original ship-unlock chains, mapped to the DS
# sector type whose lists stand in for that people (pool category 7).
HOME_SPECIALS = {"ENGI_UNLOCK_1": 2, "ROCK_UNLOCK1": 2, "MANTIS_NAMED_THIEF": 4,
                 "NEBULA_SLUG_FIGHT_UNLOCK": 0, "ZOLTAN_PEACE_QUEST": 1,
                 # V63: ROCK_HOME's Crystal beacon, whose blue option ends in <secretSector/>.
                 "ROCK_CRYSTAL_BEACON": 2,
                 # V66: REBEL_SECTOR_MINIBOSS's flagship under construction (Federation unlock).
                 # Appended last so earlier specials keep their saved bits.
                 "FLAGSHIP_CONSTRUCTION": 3}
# V63 hidden Crystal homeworld (sector_data.xml CRYSTAL_HOME) = DS sector type 6
# (SECTOR_SECRET, after SECTOR_FINAL). Its lists compile after the homeworld
# specials so every earlier root index (saved seen bits) stays put. Category 8 is
# the sector's start beacon (START_BEACON_CRYSTAL, which places the CRYSTAL_UNLOCK
# quest). STORE_CRYSTAL is left to the DS fixed store beacons.
SECRET_SECTOR = 6
SECRET_POOLS = {0: ["HOSTILE_CRYSTAL", "BOARDERS_CRYSTAL"], 1: ["NEUTRAL_CRYSTAL"], 4: ["ITEMS_CRYSTAL"],
                5: ["NOTHING_CRYSTAL"], 8: ["START_BEACON_CRYSTAL"]}
IE_SECRET_SECTOR = 18
# V60 combat environments (event <environment type>). PDS is Advanced Edition.
ENVIRONMENTS = {"asteroid": 1, "sun": 2, "nebula": 3, "storm": 4}
# V53 sector pools from data/sector_data.xml, mapped to the DS sector types.
# Categories: 0 hostile (fight beacons), 1 neutral, 2 distress, 3 quests, 4 items, 5 empty.
POOLS = {
    1: {0: ["HOSTILE_CIVILIAN", "HOSTILE1", "HOSTILE_ZOLTAN"], 1: ["NEUTRAL_CIVILIAN", "NEUTRAL_ZOLTAN"], 2: ["DISTRESS_BEACON", "DISTRESS_BEACON_ZOLTAN"], 3: ["QUESTS", "QUESTS_ZOLTAN"], 4: ["ITEMS", "ITEM_ZOLTAN"], 5: ["NOTHING", "NOTHING_ZOLTAN"]},
    2: {0: ["HOSTILE_ENGI", "HOSTILE_ROCK"], 1: ["NEUTRAL_ENGI", "NEUTRAL_ROCK"], 2: ["DISTRESS_BEACON_ENGI", "DISTRESS_BEACON_ROCK"], 3: ["QUESTS_ENGI", "QUESTS_ROCK"], 4: ["ITEMS_ENGI", "ITEMS"], 5: ["NOTHING_ENGI", "NOTHING_ROCK"]},
    3: {0: ["HOSTILE_REBEL"], 1: ["NEUTRAL_REBEL"], 2: ["DISTRESS_BEACON_REBEL"], 3: ["QUESTS_REBEL"], 4: ["ITEMS"], 5: ["NOTHING_REBEL"]},
    4: {0: ["HOSTILE_PIRATE", "HOSTILE_MANTIS"], 1: ["NEUTRAL_PIRATE", "NEUTRAL_MANTIS"], 2: ["DISTRESS_BEACON_PIRATE", "DISTRESS_BEACON_MANTIS"], 3: ["QUESTS_PIRATE"], 4: ["ITEMS"], 5: ["NOTHING_PIRATE", "NOTHING_MANTIS"]},
    0: {0: ["NEBULA_HOSTILE", "NEBULA_HOSTILE_SLUG", "HOSTILE_SLUG"], 1: ["NEBULA_NEUTRAL", "NEBULA_NEUTRAL_SLUG"], 2: ["DISTRESS_BEACON", "DISTRESS_BEACON_SLUG"], 3: [], 4: ["ITEMS"], 5: ["NEBULA_EMPTY", "NOTHING_SLUG"]},
}
for level, low, high in (("LOW", 10, 20), ("MED", 20, 30), ("HIGH", 30, 45)):
    AUTO_REWARDS[("standard", level)] = [(0, 1, 3), (2, 1, 3), (1, low, high)]
# V53: the remaining autoReward kinds, all authored DS economy (not recovered math).
for level, k in (("LOW", 0), ("MED", 1), ("HIGH", 2)):
    scrap = ((8, 15), (15, 25), (25, 40))[k]
    AUTO_REWARDS.setdefault(("scrap_only", level), [(1, (10, 20, 35)[k], (20, 35, 50)[k])])
    AUTO_REWARDS.setdefault(("fuel", level), [(0, (1, 3, 5)[k], (3, 6, 8)[k]), (1,) + scrap])
    AUTO_REWARDS.setdefault(("fuel_only", level), [(0, (1, 3, 5)[k], (3, 5, 7)[k])])
    AUTO_REWARDS.setdefault(("missiles", level), [(2, (1, 3, 5)[k], (3, 5, 8)[k]), (1,) + scrap])
    AUTO_REWARDS.setdefault(("droneparts", level), [(3, (1, 2, 4)[k], (2, 4, 6)[k]), (1,) + scrap])
    # V56: drone rewards give a random drone schematic plus scrap.
    AUTO_REWARDS.setdefault(("drone", level), [(10, -1, -1), (1,) + scrap])
    AUTO_REWARDS.setdefault(("stuff", level), [(0, 1, (2, 3, 4)[k]), (2, 1, (2, 3, 4)[k]), (3, 0, (1, 2, 3)[k]), (1,) + scrap])
    AUTO_REWARDS.setdefault(("weapon", level), [(5, -1, -1), (1,) + scrap])
for level, k in (("LOW", 0), ("MED", 1), ("HIGH", 2)):
    AUTO_REWARDS.setdefault(("augment", level), [(9, -1, -1), (1, (8, 15, 25)[k], (15, 25, 40)[k])])
for kind in ("standard", "scrap_only", "fuel", "fuel_only", "missiles", "droneparts", "drone", "stuff", "weapon", "augment"):
    AUTO_REWARDS.setdefault((kind, "RANDOM"), AUTO_REWARDS[(kind, "MED")])
    AUTO_REWARDS.setdefault((kind, "MEDIUM"), AUTO_REWARDS[(kind, "MED")])
MAX_DEPTH, MAX_NODES, MAX_CHOICES, MAX_TEXT, MAX_LABEL = 16, 6000, 6, 1600, 228
# V62: text rows per choice box by number of choices (campaign_view.inc choice_rows).
CHOICE_ROWS = {1: 6, 2: 6, 3: 4, 4: 3, 5: 3, 6: 2}
MAX_TEXT_ALTERNATIVES = 32

class Unsupported(ValueError):
    pass

def attrs(e, allowed):
    bad = set(e.attrib) - set(allowed)
    if bad:
        raise Unsupported("unsupported %s attributes: %s" % (e.tag, ",".join(sorted(bad))))

def integer(value, low=-999, high=999):
    try:
        n = int(value)
    except (TypeError, ValueError):
        raise Unsupported("invalid integer: " + str(value))
    if not low <= n <= high:
        raise Unsupported("integer out of bounds: " + str(n))
    return n

def probability(value):
    if not re.fullmatch(r"(?:0(?:\.[0-9]{1,4})?|1(?:\.0{1,4})?)", value):
        raise Unsupported("invalid probability chance")
    whole, _, fraction = value.partition(".")
    return int(whole) * 10000 + int((fraction + "0000")[:4])


def wrapped_rows(text):
    """Match campaign_view.inc event_row at38columns, including long words."""
    rows = 0
    while text:
        n, last = 0, -1
        while n < len(text) and text[n] != "\n" and n < 38:
            if text[n] == " ":
                last = n
            n += 1
        if n == 38 and n < len(text) and text[n] != "\n" and last > 0:
            n = last
        text = text[n:].lstrip(" \n\r")
        rows += 1
    return rows

class Compiler:
    def __init__(self, definitions, strings):
        self.definitions, self.strings = definitions, strings
        self.nodes, self.choices, self.effects, self.alternatives = [], [], [], []
        self.sources = set()
        self.ships, self.enemies, self.ship_blueprints = [], [], []
        self.ship_indices = {}
        self.req_lists, self.req_list_ids = {}, []
        self.crew_names = []
        self.quest_targets = {}  # V63: compiled quest node -> original event name

    def resolve(self, tag, name):
        key = (tag, name)
        if key not in self.definitions:
            raise Unsupported("missing %s reference: %s" % key)
        source, element = self.definitions[key]
        self.sources.add(source)
        return element

    def text(self, e):
        if e is None:
            return ""
        attrs(e, {"id", "planet", "back"})
        if len(e):
            raise Unsupported("nested text element")
        if "id" in e.attrib:
            if (e.text or "").strip():
                raise Unsupported("mixed referenced and inline text")
            if e.attrib["id"] not in self.strings:
                raise Unsupported("missing text: " + e.attrib["id"])
            s = self.strings[e.attrib["id"]]
        else:
            s = e.text or ""
        # Renderer uses ASCII. Preserve wording with explicit punctuation normalization.
        s = s.translate(str.maketrans({"‘": "'", "’": "'", "“": '\"', "”": '\"', "–": "-", "—": "--", "…": "...", " ": " "}))
        s = re.sub(r"\s+", " ", s).strip()
        if not s:
            raise Unsupported("empty text element")
        if any(ord(c) > 127 for c in s):
            raise Unsupported("unsupported text glyph")
        if len(s) > MAX_TEXT:
            raise Unsupported("text too long")
        return s

    def surrender_paths(self, index, accepted=False, effects=False, depth=0):
        """Validate every continuation: refusal cannot silently grant rewards."""
        if depth >= MAX_DEPTH:
            raise Unsupported("surrender continuation depth exceeds16")
        node = self.nodes[index]
        if node["ship"] >= 0:
            raise Unsupported("replacement ship in surrender")
        if accepted and node["hostile"] == 1:
            raise Unsupported("surrender reopens accepted combat")
        accepted = accepted or node["hostile"] == 0
        effects = effects or bool(node["effect_count"] or node["pursuit"])
        targets = self.alternatives[node["first_alt"]:node["first_alt"] + node["alt_count"]]
        targets += [c["next"] for c in self.choices[node["first_choice"]:node["first_choice"] + node["choice_count"]]]
        if targets:
            found = False
            for target in targets:
                found = self.surrender_paths(target, accepted, effects, depth + 1) or found
            return found
        if effects and not accepted:
            raise Unsupported("resource-bearing surrender refusal")
        return accepted

    def compile_ship(self, name, path):
        if "ship:" + name in path:
            raise Unsupported("cyclic ship reference: " + name)
        if name in self.ship_indices:
            return self.ship_indices[name]
        e = self.resolve("ship", name)
        attrs(e, {"name", "auto_blueprint"})
        if any(c.tag not in {"destroyed", "deadCrew", "escape", "gotaway", "surrender", "crew"} for c in e):
            raise Unsupported("unsupported ship outcome: " + name)
        if len(e.findall("destroyed")) != 1 or len(e.findall("deadCrew")) > 1:
            raise Unsupported("missing or duplicate ship outcomes: " + name)
        escape = e.find("escape")
        if len(e.findall("escape")) > 1 or len(e.findall("gotaway")) > 1 or (escape is None and e.find("gotaway") is not None):
            raise Unsupported("missing or duplicate escape/gotaway pair")
        escape_timer = escape_min = escape_max = escape_chance = 0
        escape_node = -1
        if escape is not None:
            attrs(escape, {"timer", "min", "max", "chance", "load"})
            # The XML does not define an implicit timer. Forty seconds is a
            # documented DS adaptation, not a recovered executable constant.
            escape_timer = integer(escape.get("timer", "40"), 1, 3000)
            escape_chance = probability(escape.get("chance", "1"))
            escape_min = integer(escape.get("min"), 1, 1000)
            escape_max = integer(escape.get("max"), 1, 1000)
            if escape_min > escape_max:
                raise Unsupported("inverted escape hull range")
            wrapper = copy.deepcopy(escape)
            wrapper.tag = "event"
            wrapper.attrib = {"load": escape.get("load")} if "load" in escape.attrib else {}
            escape_node = self.compile(wrapper, path + ("ship:" + name,), True)
            warning = self.nodes[escape_node]
            if (not warning["text"] or warning["ship"] != -1 or warning["hostile"] != 1 or
                warning["effect_count"] or warning["pursuit"] or warning["choice_count"] or warning["alt_count"]):
                raise Unsupported("unsupported escape warning closure")
        surrender_min = surrender_max = surrender_chance = 0
        surrender_node = -1
        if len(e.findall("surrender")) > 1:
            raise Unsupported("duplicate surrender outcome")
        surrender = e.find("surrender")
        if surrender is not None:
            attrs(surrender, {"min", "max", "chance", "load"})
            surrender_min = integer(surrender.get("min"), 0, 1000)
            surrender_max = integer(surrender.get("max"), 0, 1000)
            if surrender_min > surrender_max:
                raise Unsupported("inverted surrender hull range")
            surrender_chance = probability(surrender.get("chance", "1"))
            wrapper = copy.deepcopy(surrender)
            wrapper.tag = "event"
            wrapper.attrib = {"load": surrender.get("load")} if "load" in surrender.attrib else {}
            surrender_node = self.compile(wrapper, path + ("ship:" + name,), True)
            if not self.surrender_paths(surrender_node):
                raise Unsupported("surrender has no explicit ceasefire continuation")
        # V61: crew species mix (<crew><crewMember type prop/></crew>), up to three species.
        crew_mix = []
        crews = e.findall("crew")
        if len(crews) > 1:
            raise Unsupported("duplicate crew mix")
        if crews:
            attrs(crews[0], set())
            for member in crews[0]:
                attrs(member, {"type", "prop"})
                if member.tag != "crewMember" or member.get("type") not in SPECIES or len(member):
                    raise Unsupported("unsupported crew mix member")
                crew_mix.append((SPECIES[member.get("type")], max(0, min(100, round(float(member.get("prop", "0")) * 100)))))
            if not 1 <= len(crew_mix) <= 3 or sum(p for _, p in crew_mix) <= 0:
                raise Unsupported("unsupported crew mix size")
        crew_mix += [(-1, 0)] * (3 - len(crew_mix))
        blueprint_name = e.get("auto_blueprint")
        names = []
        if ("blueprintList", blueprint_name) not in self.definitions and blueprint_name in ENEMY_BLUEPRINTS:
            listing = []
            names.append(blueprint_name)
        else:
            listing = self.resolve("blueprintList", blueprint_name)
            attrs(listing, {"name"})
        for child in listing:
            attrs(child, set())
            if child.tag != "name" or len(child):
                raise Unsupported("invalid ship blueprint list")
            blueprint = (child.text or "").strip()
            if blueprint not in ENEMY_BLUEPRINTS:
                raise Unsupported("unsupported ship blueprint: " + blueprint)
            names.append(blueprint)
        if not 1 <= len(names) <= 16:
            raise Unsupported("ship blueprint pool size")
        outcomes = {}
        for tag in ("destroyed", "deadCrew", "gotaway"):
            child = e.find(tag)
            if child is None:
                outcomes[tag] = -1
                continue
            attrs(child, {"load"})
            wrapper = copy.deepcopy(child)
            wrapper.tag = "event"
            outcomes[tag] = self.compile(wrapper, path + ("ship:" + name,), False)
        first = len(self.enemies)
        self.enemies.extend(ENEMY_BLUEPRINTS[n] for n in names)
        self.ship_blueprints.extend(names)
        index = len(self.ships)
        self.ships.append(dict(id=name, first_enemy=first, enemy_count=len(names), crew0=crew_mix[0][0], crew0_percent=crew_mix[0][1],
                               crew1=crew_mix[1][0], crew1_percent=crew_mix[1][1], crew2=crew_mix[2][0], crew2_percent=crew_mix[2][1],
                              destroyed=outcomes["destroyed"], dead_crew=outcomes["deadCrew"], escape_timer=escape_timer, escape_min=escape_min, escape_max=escape_max, escape_node=escape_node, gotaway=outcomes["gotaway"], surrender_min=surrender_min, surrender_max=surrender_max, surrender_chance=surrender_chance, surrender_node=surrender_node, escape_chance=escape_chance))
        self.ship_indices[name] = index
        return index

    def compile(self, e, path=(), ship_context=False):
        if len(path) >= MAX_DEPTH:
            raise Unsupported("reference/nesting depth exceeds16")
        if len(self.nodes) >= MAX_NODES:
            raise Unsupported("compiled node limit exceeded")
        attrs(e, {"name", "unique", "load"})
        if e.get("unique", "false") not in ("true", "false"):
            raise Unsupported("invalid unique flag")
        if e.get("load"):
            if len(e) or (e.text or "").strip() or set(e.attrib) != {"load"}:
                raise Unsupported("mixed loaded event")
            name = e.attrib["load"]
            if name in path:
                raise Unsupported("cyclic event reference: " + name)
            tag = "event" if ("event", name) in self.definitions else "eventList"
            return self.compile(self.resolve(tag, name), path + (name,), ship_context)
        if e.tag == "eventList":
            attrs(e, {"name"})
            if not 1 <= len(e) <= 16 or any(c.tag != "event" for c in e):
                raise Unsupported("invalid event list")
            targets = [self.compile(c, path + ("list",), ship_context) for c in e]
            return self.random_node(targets)
        if e.tag != "event":
            raise Unsupported("unexpected node: " + e.tag)
        texts = e.findall("text")
        if len(texts) > 1:
            raise Unsupported("multiple node texts")
        t = texts[0] if texts else None
        if t is not None and t.get("load"):
            attrs(t, {"load"})
            if len(t) or (t.text or "").strip():
                raise Unsupported("mixed loaded text")
            name = t.attrib["load"]
            if "text:" + name in path:
                raise Unsupported("cyclic text list")
            listing = self.resolve("textList", name)
            attrs(listing, {"name"})
            if not 1 <= len(listing) <= MAX_TEXT_ALTERNATIVES or any(x.tag != "text" for x in listing):
                raise Unsupported("invalid text list")
            targets = []
            for alternative in listing:
                clone = copy.deepcopy(e)
                clone.remove(clone.find("text"))
                clone.insert(0, copy.deepcopy(alternative))
                targets.append(self.compile(clone, path + ("text:" + name,), ship_context))
            return self.random_node(targets)
        children = list(e)
        allowed = {"text", "choice", "item_modify", "damage", "autoReward", "distressBeacon", "ship", "img", "modifyPursuit", "crewMember",
                   "fleet", "reveal_map", "weapon", "augment", "drone", "unlockShip", "environment", "quest",
                   "store", "removeCrew", "boarders", "status", "secretSector"}
        for c in children:
            if c.tag not in allowed:
                raise Unsupported("unsupported effect/tag: " + c.tag)
        choices = e.findall("choice")
        # V60 beacon hazards (combat environments). PDS batteries are Advanced Edition.
        env = 0
        env_elements = e.findall("environment")
        if len(env_elements) > 1:
            raise Unsupported("multiple environments")
        if env_elements:
            attrs(env_elements[0], {"type", "target"})
            kind = env_elements[0].get("type")
            if kind not in ENVIRONMENTS:
                raise Unsupported("unsupported environment: " + str(kind))
            if env_elements[0].get("target") is not None:
                raise Unsupported("targeted environment")
            env = ENVIRONMENTS[kind]
        ship, hostile = -1, -1
        ship_elements = e.findall("ship")
        if len(ship_elements) > 1:
            raise Unsupported("multiple ship actions")
        if ship_elements:
            action = ship_elements[0]
            attrs(action, {"load", "hostile"})
            if len(action) or (action.text or "").strip():
                raise Unsupported("nested ship action")
            if action.get("hostile") not in ("true", "false"):
                raise Unsupported("ship action requires explicit hostility")
            hostile = int(action.get("hostile") == "true")
            if action.get("load"):
                if ship_context:
                    raise Unsupported("replacement ship in active context")
                ship = self.compile_ship(action.get("load"), path)
                ship_context = True
            elif not ship_context:
                raise Unsupported("ship hostility without loaded ship context")
        pursuit = 0
        pursuit_elements = e.findall("modifyPursuit")
        if len(pursuit_elements) > 1:
            raise Unsupported("duplicate pursuit effect")
        if pursuit_elements:
            effect = pursuit_elements[0]
            attrs(effect, {"amount"})
            if len(effect) or (effect.text or "").strip():
                raise Unsupported("nested pursuit effect")
            pursuit = integer(effect.get("amount"), -3, 10)
            if not pursuit:
                raise Unsupported("zero pursuit effect")
        node_effects = []
        for c in children:
            if c.tag == "img":
                attrs(c, {"planet", "back"})
                if len(c) or (c.text or "").strip():
                    raise Unsupported("nested decorative image")
            elif c.tag == "item_modify":
                attrs(c, set())
                for item in c:
                    if item.tag != "item":
                        raise Unsupported("unsupported item child")
                    attrs(item, {"type", "min", "max"})
                    if len(item):
                        raise Unsupported("nested item effects")
                    typ = item.get("type")
                    if typ not in KINDS:
                        raise Unsupported("unsupported resource: " + str(typ))
                    low, high = integer(item.get("min")), integer(item.get("max"))
                    if low > high or low < 0 < high:
                        raise Unsupported("invalid resource range")
                    node_effects.append((KINDS[typ], low, high))
            elif c.tag == "damage":
                attrs(c, {"amount", "system", "effect"})
                if len(c):
                    raise Unsupported("nested damage effects")
                amount = integer(c.get("amount"), -30, 30)
                node_effects.append((4, -amount, -amount))
                # V53: system damage and room hazards. Target 0..4 main systems,
                # 5 random system, 6 random room (hazard only).
                targets = {"shields": 0, "engines": 1, "weapons": 2, "oxygen": 3, "medbay": 4, "random": 5, "room": 6}
                system = c.get("system")
                if system is not None:
                    if system not in targets:
                        raise Unsupported("unsupported damage system: " + system)
                    if system != "room" and amount > 0:
                        node_effects.append((7, amount, targets[system]))
                    effect = c.get("effect")
                    if effect is not None:
                        hazards = {"fire": 1, "breach": 2, "random": 3}
                        if effect not in hazards:
                            raise Unsupported("unsupported damage effect: " + effect)
                        node_effects.append((8, hazards[effect], targets[system]))
                elif c.get("effect") is not None:
                    raise Unsupported("damage effect without system")
            elif c.tag == "crewMember" and (c.get("id") or c.get("all_skills")):
                # V62 named crew (original story characters): species, name and skill level.
                attrs(c, {"amount", "class", "id", "all_skills"})
                if len(c) or integer(c.get("amount", "1"), 1, 1) != 1 or c.get("class") not in SPECIES:
                    raise Unsupported("unsupported named crew")
                name = (self.strings.get(c.get("id")) or c.get("id") or "").strip() if c.get("id") else ""
                if len(name) > 15:
                    raise Unsupported("named crew name too long")
                if name not in self.crew_names:
                    self.crew_names.append(name)
                skills = integer(c.get("all_skills", "0"), 0, 2)
                node_effects.append((17, SPECIES[c.get("class")] + 10 * skills, self.crew_names.index(name)))
            elif c.tag == "crewMember":
                attrs(c, {"amount", "class"})
                if len(c) or (c.text or "").strip():
                    raise Unsupported("nested crew reward")
                if integer(c.get("amount"), 1, 1) != 1:
                    raise Unsupported("unsupported crew reward amount")
                species = c.get("class")
                if species is not None and species not in SPECIES:
                    raise Unsupported("unsupported crew reward species: " + species)
                low, high = (0, 5) if species is None else (SPECIES[species], SPECIES[species])
                node_effects.append((6, low, high))
            elif c.tag == "autoReward":
                attrs(c, {"level"})
                if len(c):
                    raise Unsupported("nested automatic reward effects")
                key = ((c.text or "").strip(), c.get("level", "").upper())
                if key not in AUTO_REWARDS:
                    raise Unsupported("unsupported automatic reward: " + str(key))
                node_effects.extend(AUTO_REWARDS[key])
            elif c.tag in ("fleet", "reveal_map"):
                # Decorative background fleet; the DS map is always fully revealed.
                attrs(c, {"type"} if c.tag == "fleet" else set())
                if len(c):
                    raise Unsupported("nested " + c.tag)
            elif c.tag == "augment":
                attrs(c, {"name"})
                if len(c):
                    raise Unsupported("nested augment reward")
                name = c.get("name")
                if name == "RANDOM":
                    node_effects.append((9, -1, -1))
                elif name in AUGMENT_IDS:
                    node_effects.append((9, AUGMENT_IDS[name], AUGMENT_IDS[name]))
                else:
                    raise Unsupported("unsupported augment reward: " + str(name))
            elif c.tag == "store":
                # V62: the beacon becomes a store, opened after the event.
                attrs(c, set())
                if len(c) or (c.text or "").strip():
                    raise Unsupported("nested store")
                node_effects.append((13, 0, 0))
            elif c.tag == "removeCrew":
                # V62: a crewmember leaves (the base game has no clone bay to bring them back).
                attrs(c, set())
                if any(x.tag not in ("clone", "text") for x in c) or len(c.findall("clone")) > 1 or len(c.findall("text")) > 1:
                    raise Unsupported("malformed removeCrew")
                node_effects.append((14, 0, 0))
            elif c.tag == "boarders":
                # V62: intruders beam aboard when this event's fight starts.
                attrs(c, {"min", "max", "class"})
                if len(c):
                    raise Unsupported("nested boarders")
                species = c.get("class", "random")
                if species != "random" and species not in SPECIES:
                    raise Unsupported("unsupported boarder species: " + species)
                code = 7 if species == "random" else SPECIES[species]
                low, high = integer(c.get("min"), 1, 8), integer(c.get("max"), 1, 8)
                if low > high:
                    raise Unsupported("inverted boarder count")
                node_effects.append((15, low + 100 * code, high + 100 * code))
            elif c.tag == "status":
                attrs(c, {"type", "target", "system", "amount"})
                if len(c):
                    raise Unsupported("nested status")
                kind, target, system = c.get("type"), c.get("target"), c.get("system")
                if kind not in STATUS_TYPES or target not in ("player", "enemy"):
                    raise Unsupported("unsupported status: %s %s" % (kind, target))
                if system in STATUS_IGNORED:
                    continue
                if system not in STATUS_SYSTEMS:
                    raise Unsupported("unsupported status system: " + str(system))
                amount = integer(c.get("amount", "0"), 0, 100)
                if kind == "divide" and amount < 1:
                    raise Unsupported("status divide by zero")
                node_effects.append((16, STATUS_TYPES[kind] * 1000 + (target == "enemy") * 100 + STATUS_SYSTEMS[system], amount))
            elif c.tag == "quest":
                # V61: a quest beacon for the target event, placed on an unvisited beacon.
                attrs(c, {"event"})
                if len(c) or not c.get("event"):
                    raise Unsupported("malformed quest")
                target = self.compile(self.resolve("event" if ("event", c.get("event")) in self.definitions else "eventList", c.get("event")), path + ("quest:" + c.get("event"),), False)
                self.quest_targets[target] = c.get("event")
                node_effects.append((12, target, target))
            elif c.tag == "unlockShip":
                # V57: original hangar unlock (ship class id 0-8, Type A layout).
                attrs(c, {"id"})
                if len(c):
                    raise Unsupported("nested unlockShip")
                ship = integer(c.get("id"), 0, 8)
                node_effects.append((11, ship, ship))
            elif c.tag == "drone":
                attrs(c, {"name"})
                if len(c):
                    raise Unsupported("nested drone reward")
                name = c.get("name")
                if name == "RANDOM":
                    node_effects.append((10, -1, -1))
                elif name in DRONE_IDS:
                    node_effects.append((10, DRONE_IDS[name], DRONE_IDS[name]))
                else:
                    raise Unsupported("unsupported drone reward: " + str(name))
            elif c.tag == "weapon":
                attrs(c, {"name"})
                if len(c):
                    raise Unsupported("nested weapon reward")
                name = c.get("name")
                if name == "RANDOM":
                    node_effects.append((5, -1, -1))
                elif name in WEAPON_IDS:
                    node_effects.append((5, WEAPON_IDS[name], WEAPON_IDS[name]))
                else:
                    raise Unsupported("unsupported weapon reward: " + str(name))
            elif c.tag == "secretSector":
                # V63: jump to the hidden Crystal sector once this event is closed.
                attrs(c, set())
                if len(c) or (c.text or "").strip():
                    raise Unsupported("nested secretSector")
                node_effects.append((IE_SECRET_SECTOR, 0, 0))
            elif c.tag == "distressBeacon":
                attrs(c, set())
                if len(c):
                    raise Unsupported("nested distress effects")
        if any(k == IE_SECRET_SECTOR for k, _, _ in node_effects) and (
                choices or ship_elements or ship_context or sum(k == IE_SECRET_SECTOR for k, _, _ in node_effects) > 1):
            raise Unsupported("secretSector must end a closed event")
        if sum(k == 12 for k, _, _ in node_effects) > 1:
            raise Unsupported("multiple quests in one event")
        if sum(k in (5, 6, 17) for k, _, _ in node_effects) > 1:
            raise Unsupported("multiple pending object rewards in immediate event")
        if len(node_effects) > 8:
            raise Unsupported("effect limit exceeds8")
        compiled_choices = []
        choices = [c for c in choices if c.get("req") not in AE_REQS]
        if len(choices) > MAX_CHOICES:
            raise Unsupported("choice limit exceeds6")
        for c in choices:
            attrs(c, {"hidden", "req", "lvl"})
            if c.get("hidden", "false") not in ("true", "false"):
                raise Unsupported("invalid hidden flag")
            if len(c.findall("text")) != 1 or len(c.findall("event")) != 1 or len(c) != 2:
                raise Unsupported("malformed choice")
            label = self.text(c.find("text"))
            if len(label) > MAX_LABEL:
                raise Unsupported("choice label too long")
            req = c.get("req")
            kind = value = 0
            level = integer(c.get("lvl", "1"), 1, 8)
            if req is not None:
                if req in SPECIES:
                    kind, value = 1, SPECIES[req]
                elif req in REQ_SYSTEMS:
                    kind, value = 2, REQ_SYSTEMS[req]
                elif req in WEAPON_IDS:
                    kind, value = 3, WEAPON_IDS[req]
                elif req in AUGMENT_IDS:
                    kind, value = 6, AUGMENT_IDS[req]
                elif req in DRONE_IDS:
                    kind, value = 7, DRONE_IDS[req]
                elif req in REQ_NEVER:
                    kind, value = 4, 0
                elif ("blueprintList", req) in self.definitions:
                    members = [(n.text or "").strip() for n in self.resolve("blueprintList", req)]
                    ids = sorted({WEAPON_IDS[m] for m in members if m in WEAPON_IDS})
                    drone_ids = sorted({DRONE_IDS[m] for m in members if m in DRONE_IDS})
                    if ids and drone_ids:
                        raise Unsupported("mixed weapon/drone requirement list: " + req)
                    if drone_ids:
                        ids = drone_ids
                    if not ids:
                        raise Unsupported("unsupported requirement list: " + req)
                    if req not in self.req_lists:
                        self.req_lists[req] = len(self.req_list_ids)
                        self.req_list_ids.append(ids)
                    kind, value = (8 if drone_ids else 5), self.req_lists[req]
                else:
                    raise Unsupported("unsupported requirement: " + req)
            target = self.compile(c.find("event"), path + ("choice",), ship_context)
            # Rows available per button: 4 or 5 choices fit three text rows, six fit two.
            rows = CHOICE_ROWS[len(choices)]
            kinds = self.effect_kinds(target)
            hint_rows = (len(kinds) * 17 + 37) // 38 if kinds else 0  # "FUEL +1..+3 / " per resource
            if wrapped_rows(label) + hint_rows > rows:
                raise Unsupported("choice label exceeds wrapped row limit")
            compiled_choices.append(dict(text=label, next=target, req_kind=kind, req_value=value, req_level=level))
        if len(self.nodes) >= MAX_NODES:
            raise Unsupported("compiled node limit exceeded")
        first_choice, first_effect = len(self.choices), len(self.effects)
        self.choices.extend(compiled_choices)
        self.effects.extend(dict(kind=k, min=lo, max=hi) for k, lo, hi in node_effects)
        node = dict(text=self.text(t), first_choice=first_choice, choice_count=len(compiled_choices),
                    first_effect=first_effect, effect_count=len(node_effects), first_alt=0, alt_count=0, ship=ship, hostile=hostile, pursuit=pursuit, env=env)
        self.nodes.append(node)
        return len(self.nodes) - 1

    def entry_env(self, index, depth=0):
        """Environment set on arrival (the node or any of its random alternatives)."""
        node = self.nodes[index]
        if node.get("env"):
            return node["env"]
        if depth < MAX_DEPTH:
            for a in self.alternatives[node["first_alt"]:node["first_alt"] + node["alt_count"]]:
                found = self.entry_env(a, depth + 1)
                if found:
                    return found
        return 0

    def immediate_effects(self, index):
        node = self.nodes[index]
        if node["effect_count"] or node["pursuit"]:
            return True
        return any(self.immediate_effects(a) for a in
                   self.alternatives[node["first_alt"]:node["first_alt"] + node["alt_count"]])

    def effect_kinds(self, index, depth=0):
        """Resource kinds (0..4) any immediate branch of this node can change."""
        node = self.nodes[index]
        kinds = {e["kind"] for e in self.effects[node["first_effect"]:node["first_effect"] + node["effect_count"]] if e["kind"] <= 4}
        if depth < MAX_DEPTH:
            for a in self.alternatives[node["first_alt"]:node["first_alt"] + node["alt_count"]]:
                kinds |= self.effect_kinds(a, depth + 1)
        return kinds

    def random_node(self, targets):
        if len(self.nodes) >= MAX_NODES:
            raise Unsupported("compiled node limit exceeded")
        first = len(self.alternatives)
        self.alternatives.extend(targets)
        self.nodes.append(dict(text="", first_choice=0, choice_count=0, first_effect=0,
                               effect_count=0, first_alt=first, alt_count=len(targets), ship=-1, hostile=-1, pursuit=0, env=0))
        return len(self.nodes) - 1

def read_archive(archive):
    definitions, strings, sources = {}, {}, []
    names = list(BASE_FILES) + list(SHIP_FILES) + ["data/text_events.xml", "data/text_misc.xml", "data/text_tutorial.xml"]
    for name in names:
        data = archive.read(name)
        sources.append(dict(path=name, sha256=hashlib.sha256(data).hexdigest(), size=len(data)))
        root = xml_root(data)
        if name in BASE_FILES or name in SHIP_FILES:
            for e in root.iter():
                if e.tag in {"event", "eventList", "textList", "ship", "blueprintList"} and "name" in e.attrib:
                    key = (e.tag, e.attrib["name"])
                    if key in definitions:
                        # newEvents.xml repeats some base definitions; the base-file copy wins.
                        if name in ("data/newEvents.xml", "data/dlcPirateBlueprints.xml"):
                            continue
                        raise Unsupported("duplicate definition: " + str(key))
                    definitions[key] = (name, e)
        else:
            for e in root.iter("text"):
                if "name" in e.attrib:
                    key, value = e.attrib["name"], e.text or ""
                    if key in strings and strings[key] != value:
                        raise Unsupported("conflicting text: " + key)
                    strings[key] = value
    return definitions, strings, sources

def build(archive):
    definitions, strings, sources = read_archive(archive)
    catalog = Compiler(definitions, strings)
    roots = []
    for name in SELECTED:
        e = catalog.resolve("event", name)
        node = catalog.compile(e, (name,))
        roots.append(dict(id=name, node=node, unique=int(e.get("unique") == "true"),
                          distress=int(e.find("distressBeacon") is not None),
                          fuel_only=int(name.startswith("NO_FUEL_"))))
    # V53: original beacon lists from sector_data.xml, compiled member by member.
    # A member that uses an unsupported feature is left out of the pool (fail
    # closed per event, with rollback), never partially compiled.
    pools, pooled = [], {}
    root_ids = {r["id"]: i for i, r in enumerate(roots)}
    def add_list(sector_type, category, list_name):
        tag = "eventList" if ("eventList", list_name) in definitions else "event"
        if (tag, list_name) not in definitions:
            return
        listing = definitions[(tag, list_name)][1]
        members = list(listing) if tag == "eventList" else [listing]
        for index, member in enumerate(members):
            if member.tag != "event":
                continue
            name = member.get("load") or ("%s#%d" % (list_name, index) if tag == "eventList" else list_name)
            if name not in root_ids:
                element = catalog.resolve("event", member.get("load")) if member.get("load") and ("event", member.get("load")) in definitions else member
                if member.get("load") and ("event", member.get("load")) not in definitions:
                    continue
                state = (len(catalog.nodes), len(catalog.choices), len(catalog.effects), len(catalog.alternatives),
                         len(catalog.ships), len(catalog.enemies), len(catalog.ship_blueprints), dict(catalog.ship_indices),
                         dict(catalog.req_lists), len(catalog.req_list_ids), set(catalog.sources))
                try:
                    node = catalog.compile(element, (name,))
                    if catalog.nodes[node]["text"] == "" and catalog.nodes[node]["alt_count"] == 0:
                        raise Unsupported("empty event")
                except Unsupported as error:
                    (n0, c0, e0, a0, s0, en0, b0, si, rl, rli, src) = state
                    del catalog.nodes[n0:], catalog.choices[c0:], catalog.effects[e0:], catalog.alternatives[a0:]
                    del catalog.ships[s0:], catalog.enemies[en0:], catalog.ship_blueprints[b0:], catalog.req_list_ids[rli:]
                    catalog.ship_indices, catalog.req_lists, catalog.sources = si, rl, src
                    pooled.setdefault(name, str(error))
                    continue
                root_ids[name] = len(roots)
                roots.append(dict(id=name[:63], node=node, unique=int(element.get("unique") == "true"),
                                  distress=int(element.find("distressBeacon") is not None), fuel_only=0))
                pooled[name] = None
            pools.append(dict(sector=sector_type, category=category, root=root_ids[name]))
            # V60: events that put the ship in a hazard also serve the map's hazard beacons (category 6).
            if catalog.entry_env(roots[root_ids[name]]["node"]) and dict(sector=sector_type, category=6, root=root_ids[name]) not in pools:
                pools.append(dict(sector=sector_type, category=6, root=root_ids[name]))
    for sector_type in sorted(POOLS):
        for category in sorted(POOLS[sector_type]):
            for list_name in POOLS[sector_type][category]:
                add_list(sector_type, category, list_name)
    # V62 homeworld specials (pool category 7), compiled with the same rollback.
    for name, sector_type in HOME_SPECIALS.items():
        if ("event", name) not in definitions:
            continue
        if name not in root_ids:
            state = (len(catalog.nodes), len(catalog.choices), len(catalog.effects), len(catalog.alternatives),
                     len(catalog.ships), len(catalog.enemies), len(catalog.ship_blueprints), dict(catalog.ship_indices),
                     dict(catalog.req_lists), len(catalog.req_list_ids), set(catalog.sources))
            try:
                node = catalog.compile(catalog.resolve("event", name), (name,))
            except Unsupported as error:
                (n0, c0, e0, a0, s0, en0, b0, si, rl, rli, src) = state
                del catalog.nodes[n0:], catalog.choices[c0:], catalog.effects[e0:], catalog.alternatives[a0:]
                del catalog.ships[s0:], catalog.enemies[en0:], catalog.ship_blueprints[b0:], catalog.req_list_ids[rli:]
                catalog.ship_indices, catalog.req_lists, catalog.sources = si, rl, src
                pooled.setdefault(name, str(error))
                continue
            root_ids[name] = len(roots)
            roots.append(dict(id=name, node=node, unique=1, distress=0, fuel_only=0))
            pooled[name] = None
        pools.append(dict(sector=sector_type, category=7, root=root_ids[name]))
    # V63 hidden Crystal homeworld lists (sector type 6), appended last.
    for category in sorted(SECRET_POOLS):
        for list_name in SECRET_POOLS[category]:
            add_list(SECRET_SECTOR, category, list_name)
    # V63: events reached only as quests of compiled events (e.g. CRYSTAL_UNLOCK)
    # count as enabled. Only targets of surviving quest effects are kept.
    quest_names = {catalog.quest_targets[e["min"]] for e in catalog.effects
                   if e["kind"] == 12 and e["min"] in catalog.quest_targets}
    coverage = []
    for (tag, name), (source, element) in sorted(definitions.items()):
        if tag != "event":
            continue
        trial = Compiler(definitions, strings)
        reason = None
        try:
            trial.compile(element, (name,))
        except Unsupported as error:
            reason = str(error)
        pooled_ok = name in SELECTED or (name in pooled and pooled[name] is None)
        accepted = pooled_ok or name in quest_names
        if name in SELECTED and reason:
            raise AssertionError("enabled root rejected: " + name)
        if name in pooled and pooled[name]:
            reason = pooled[name]
        coverage.append(dict(id=name, source=source, status="enabled" if accepted else "excluded",
                             reason=("complete reviewed closure" if name in SELECTED else "original sector beacon list member" if pooled_ok else "quest target of an enabled event") if accepted else reason or "not referenced by a supported sector beacon list",
                             eligibility="reviewed base-game encounter" if accepted else "base-source container; historical/AE eligibility not established"))
    result = dict(schema=7, crew_names=catalog.crew_names, pools=pools, req_lists=catalog.req_list_ids, ships=catalog.ships, enemies=catalog.enemies, ship_blueprints=catalog.ship_blueprints, roots=roots, nodes=catalog.nodes, choices=catalog.choices,
                  effects=catalog.effects, alternatives=catalog.alternatives,
                  sources=sources, selected=list(SELECTED), max_depth=MAX_DEPTH,
                  max_text=max(len(n["text"]) for n in catalog.nodes),
                  max_choice_text=max(len(c["text"]) for c in catalog.choices),
                  adaptations=["Missing escape timer uses40seconds; missing gotaway grants no reward or pursuit; DS interpretations", "HIGH fuel grants5-8fuel and20-30scrap; LOW scrap_only10-20scrap; RANDOM stuff independently grants1-4fuel,1-4missiles,1-3droneparts without scrap; authored DS economy", "LOW weapon selects uniformly from seven supported catalog entries; DS approximation, including starter-only rarity-zero entries", "Unspecified crew amount1 selects uniformly from six normal supported species; Crystal only through explicit class; DS approximation", "Enemy escape hull threshold latches charging; timer interpreted as seconds of operational engine/pilot time; DS approximation, not recovered engine behavior", "ASCII punctuation and whitespace normalization", "Decorative text-list planet/background attributes omitted", "Uniform bounded random lists and inclusive resource ranges",
                    "Decorative img planet/background attributes omitted", "Standard rewards use fuel1-3, missiles1-3 and scrap LOW10-20/MED20-30/HIGH30-45; authored DS economy", "Automatic MED fuel reward mapped to 3-6 fuel and 15-25 scrap; authored DS economy", "Resource affordability uses worst-case debit before committing a choice", "All choices stay visible; unmet requirements disabled"])
    return result, dict(schema=1, scope="All named event definitions in12 installed base-source containers; not proof of pre-AE eligibility or complete historical event coverage",
                        source_files=sources, total=len(coverage), enabled=sum(1 for x in coverage if x["status"] == "enabled"), roots=len(roots), entries=coverage)

def header(catalog):
    out = ["/* Original content compiled locally from owner's read-only FTL archive. Do not redistribute. */",
      "#ifndef FTL_EVENT_CATALOG_H", "#define FTL_EVENT_CATALOG_H",
      "enum { IE_FUEL=0, IE_SCRAP, IE_MISSILES, IE_DRONEPARTS, IE_HULL, IE_WEAPON, IE_CREW, IE_SYSDAMAGE, IE_SHIPHAZARD, IE_AUGMENT, IE_DRONE, IE_UNLOCK, IE_QUEST, IE_STORE, IE_REMOVECREW, IE_BOARDERS, IE_STATUS, IE_NAMED_CREW, IE_SECRET_SECTOR };",
      "enum { IE_REQ_NONE=0, IE_REQ_SPECIES, IE_REQ_SYSTEM, IE_REQ_WEAPON, IE_REQ_NEVER, IE_REQ_WEAPON_ANY, IE_REQ_AUGMENT, IE_REQ_DRONE, IE_REQ_DRONE_ANY };",
      "#define ENEMY_SHIP_BASE 100",
      "/* V61: identifies this compiled catalog in saves (quest beacons hold node indices). */",
      "#define IMPORTED_EVENT_CATALOG_HASH 0x%08xu" % (int(hashlib.sha256(json.dumps([catalog["nodes"], catalog["effects"], catalog["choices"]], sort_keys=True).encode()).hexdigest()[:8], 16)),
      "typedef struct { int sector,category,root; } ImportedEventPool;",
      "typedef struct { int kind,min,max; } ImportedEventEffect;",
      "typedef struct { const char *text; int next,req_kind,req_value,req_level; } ImportedEventChoice;",
      "typedef struct { const char *text; int first_choice,choice_count,first_effect,effect_count,first_alt,alt_count,ship,hostile,pursuit,env; } ImportedEventNode;",
      "enum { IE_ENV_NONE=0, IE_ENV_ASTEROID, IE_ENV_SUN, IE_ENV_NEBULA, IE_ENV_STORM };",
      "typedef struct { const char *id; int first_enemy,enemy_count,crew0,crew0_percent,crew1,crew1_percent,crew2,crew2_percent,destroyed,dead_crew,escape_timer,escape_min,escape_max,escape_node,gotaway,surrender_min,surrender_max,surrender_chance,surrender_node,escape_chance; } ImportedEventShip;",
      "typedef struct { const char *id; int node,unique,distress,fuel_only; } ImportedEventRoot;"]
    for singular, plural, typ in (("NODE","nodes","ImportedEventNode"),("CHOICE","choices","ImportedEventChoice"),
                                 ("EFFECT","effects","ImportedEventEffect"),("ROOT","roots","ImportedEventRoot"),("SHIP","ships","ImportedEventShip")):
        rows = catalog[plural]
        out.append("#define IMPORTED_EVENT_%s_COUNT %d" % (singular, len(rows)))
        out.append("static const %s imported_event_%s[] = {" % (typ, plural))
        for row in rows:
            out.append("    {" + ",".join(json.dumps(v, ensure_ascii=True) for v in row.values()) + "},")
        out.append("};")
    out.append("#define IMPORTED_EVENT_ALT_COUNT %d" % len(catalog["alternatives"]))
    out.append("static const int imported_event_alternatives[] = {" + ",".join(map(str,catalog["alternatives"])) + "};")
    names = catalog["crew_names"] or [""]
    out.append("#define IMPORTED_EVENT_CREW_NAME_COUNT %d" % len(names))
    out.append("static const char *const imported_event_crew_names[] = {" + ",".join(json.dumps(n) for n in names) + "};")
    out.append("#define IMPORTED_EVENT_POOL_COUNT %d" % len(catalog["pools"]))
    out.append("static const ImportedEventPool imported_event_pools[] = {" + ",".join("{%d,%d,%d}" % (r["sector"], r["category"], r["root"]) for r in catalog["pools"]) + "};")
    lists = catalog["req_lists"] or [[-1]]
    width = max(len(l) for l in lists) + 1
    out.append("#define IMPORTED_EVENT_REQ_LIST_WIDTH %d" % width)
    out.append("static const int imported_event_req_lists[][IMPORTED_EVENT_REQ_LIST_WIDTH] = {" + ",".join("{" + ",".join(map(str, l + [-1] * (width - len(l)))) + "}" for l in lists) + "};")
    out.append("#define IMPORTED_EVENT_ENEMY_COUNT %d" % len(catalog["enemies"]))
    out.append("static const int imported_event_enemies[] = {" + ",".join(catalog["enemies"]) + "};")
    out.append("#endif\n")
    return "\n".join(out)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("archive",type=Path)
    p.add_argument("--output",type=Path,default=Path(__file__).resolve().parents[1]/"generated")
    args=p.parse_args()
    out=args.output.resolve()
    if args.archive.resolve().is_relative_to(out) or out.is_relative_to(args.archive.resolve().parent):
        raise ValueError("output must remain outside installation directory")
    catalog, coverage=build(PkgArchive(args.archive))
    out.mkdir(parents=True,exist_ok=True)
    (out/"event_catalog.json").write_text(json.dumps(catalog,indent=2)+"\n",encoding="utf-8")
    (out/"event_coverage.json").write_text(json.dumps(coverage,indent=2)+"\n",encoding="utf-8")
    (out/"event_catalog.h").write_text(header(catalog),encoding="utf-8")
    print(json.dumps({"enabled":len(catalog["roots"]),"nodes":len(catalog["nodes"]),"choices":len(catalog["choices"]),"coverage":coverage["total"],"max_text":catalog["max_text"],"max_choice_text":catalog["max_choice_text"]}))

if __name__ == "__main__":
    main()
