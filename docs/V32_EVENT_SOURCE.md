# V32 original Rebel encounter source closure

## Scope and provenance

The installed FTL archive was read without modification. Generated original text and data remain local. `tools/event_compile.py` now includes the complete named `REBEL` root at index 11 after the eleven existing roots; historical root indices are preserved. The quest stage `ENGI_UNLOCK_2FAKE` remains excluded. A compiler-supported closure is eligible only through the runtime policy that supports its mechanics.

Schema 6 appends `escape_chance` to `ImportedEventShip`; all prior fields retain their order. Chance is an integer from 0 through 10000 representing the exact XML decimal probability. Missing chance is explicitly interpreted as 10000. An absent escape uses chance 0 and node -1. Invalid probabilities, unsupported effects, malformed references, cycles, incomplete branches, replacement ships and resource-bearing surrender refusals still reject the entire closure.

## Original structure retained

The source `REBEL` root has ten introduction variants and the exact `SHIPS_REBEL` pool: REBEL_FAT and REBEL_SKINNY. Its surrender has chance 0.5 and hull thresholds 2 through 3. Its escape has chance 0.5 and hull thresholds 3 through 4, without an explicit timer or gotaway callback.

The loaded surrender tree retains 23 text entries with 14 distinct texts; repeated entries preserve source weighting. Every variant offers the source accept/refuse choices. Acceptance executes hostile=false and RANDOM stuff. Refusal is an empty event with unchanged hostility, not an explicit hostile=true. The runtime must therefore resume on a terminal unchanged-hostility refusal without recreating the enemy.

The destroyed callback retains two MED standard alternatives. The dead-crew callback retains all nine weighted alternatives in order:

1. MED standard, three entries.
2. HIGH standard, two entries.
3. HIGH fuel, two entries.
4. One unspecified-species recruit and LOW scrap_only.
5. LOW weapon.

Existing pending weapon/recruit offers retain overflow and replacement decisions. No source alternatives are dropped to avoid full cargo or crew.

## Explicit DS interpretations

XML reward family names do not provide executable reward mathematics. The following are authored, inclusive DS economy ranges, not original-game balance claims:

| Reward | DS resources |
| --- | --- |
| RANDOM stuff | Independently 1–4 fuel, 1–4 missiles, 1–3 drone parts; no scrap |
| HIGH fuel | 5–8 fuel and 20–30 scrap |
| LOW scrap_only | 10–20 scrap |
| LOW weapon | Uniform choice among the seven supported player weapons |
| Unspecified recruit | Uniform choice among six normal supported species |

The new resource families compile into the existing resource effects. The `rewards.c` object-offer implementation requires no change. RANDOM stuff does not recover source rarity or correlated quantity rolls; it is deliberately a supply-only offer distinct from destruction scrap.

Missing escape timer uses 40 seconds of operational engine/pilot time. An absent gotaway callback grants no pursuit effect and no salvage. The data establishes neither this timer nor executable trigger ordering; these are runtime policies, documented separately with their tests. Loaded escape warnings are accepted only when they close to a single text and current-ship hostile=true, with no choices, random branches, resources, pursuit, or replacement ship.

## Compiler evidence

All 42 event compiler tests pass. New tests cover reward ranges, invalid families, exact decimal escape probabilities, default timer and absent gotaway, all 23 surrender variants, both source hulls, ten introductions and all nine weighted dead-crew rewards. Earlier unsupported probability and missing-callback rejection fixtures have been replaced by malformed-probability and orphan-callback fixtures because those two semantics are now implemented explicitly.

The generated catalog has 12 reviewed roots, 193 nodes, 93 choices and five ship descriptors. Coverage still audits 351 named definitions; 339 remain excluded. The header reproduces exactly from catalog JSON. A read-only installed-archive rebuild reproduces the JSON, coverage and header exactly. Runtime behavior remains a separate acceptance check; physical Nintendo DS validation remains pending.
