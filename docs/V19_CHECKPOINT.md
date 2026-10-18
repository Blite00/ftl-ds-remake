# V19 checkpoint: Small Bomb and own-ship targeting

October 3, 2026. Software integration verified. Emulator and physical DS acceptance remain pending; the complete base-game goal is still active.

## Implemented

- Original Small Bomb (BOMB_1), stable catalog ID 6: 13-second charge, one power, one missile per activation, zero hull damage, two system damage, raw personnel override 2 and raw fire chance 1. Store offer 10 costs 45 scrap; all older catalog/offer indices remain unchanged.
- A distinct teleport/impact effect bypasses ordinary shields without consuming shield bubbles. Enemy-directed bombs use normal evasion. Fire, separate system and crew damage share existing room damage rules; crew from both factions in the room can be hit.
- HOME/ENEMY targeting through touch or controller Down. L/R selects a room on that ship; FIRE/A launches. HOME explicitly says that the attack damages your systems and crew. Weapon switching restores a valid target for the selected weapon and ship.
- Queue snapshots preserve bomb side, room and payload. Retargeting after firing cannot redirect the queued attack. Full effects pools retry without charging extra ammunition. Enemy bomb volleys use a bounded ammunition count; an outstanding volley blocks another recharge/charge expenditure. New encounters reset player bomb targeting to ENEMY.
- Map-only save format 11 preserves bomb equipment and the appended store stock; formats 2–10 migrate. Target side, aim, charge and autofire reset on load, consistent with the existing save policy. Full cargo, insufficient resources and repeated purchases preserve stock/resources when rejected.
- Persistent source weapon coverage register: 49 named definitions from the installed data/blueprints.xml, seven selected player entries, one selected enemy dependency and 41 disabled entries. Unnamed reference occurrences are tracked separately. Source-file location alone does not prove historical base-game eligibility or player availability. See WEAPON_COVERAGE.md and generated/weapon_coverage.json.

## Source facts and approximations

Small Bomb's explicit sysDamage and persDamage fields are both 2. Runtime converts one personnel unit to 15 crew HP, giving 30 direct crew damage. That conversion is a documented compatibility choice; the XML alone does not prove the HP unit. Other selected weapons retain their existing authored crew-damage policy. Raw fire chance 1 becomes the existing ten-percent ignition policy.

The teleport delay is twelve simulation ticks and presentation is procedural. Friendly self-targeted bombs bypass evasion; hostile bombs retain it. These timing, conversion and evasion policies are not claimed as recovered engine behavior. Public enemy-bomb scheduling works in focused tests, but this release does not add bomb-equipped enemies to random campaign pools. Older enemy missile behavior remains its existing approximation; bomb ammunition accounting must not be mistaken for complete enemy resource fidelity.

Only Small Bomb is added; healing, fire, breach, ion and other bomb variants remain outside the playable pool until their semantics are covered. Advanced Edition stun and Lanius remain excluded. The seven selected player weapons now provide initial examples of laser, missile, ion, beam and bomb families, not the full original catalog.

## Validation evidence

All 49 registered strict host checks passed on integrated source (`build/v19-checks.txt`). Focused tests cover source payloads, shield bypass, evasion, zero hull damage, both-faction friendly fire, immutable target-side/payload queues, pause, effect-pool saturation, player/enemy ammunition, encounter resets, UI weapon switching, visible hit/miss feedback and sensor-respecting framebuffer output.

The earned-bomb campaign buys Small Bomb for 45 scrap after ordinary upgrades, observes actual bomb impacts with system-health loss, completes six current sectors and performs 36 disk cold loads preserving resources, equipment/cargo and stock. Final hull 30, scrap 112. No resources, damage or victories are injected. Existing beam, ion, boarding, mixed-species, ordinary and resumed campaign checks also passed, including the existing 108-cold-load regression. These are evidence for the current six-sector subset, not the future full campaign.

Inspected actual renderer previews: bomb-target-v19.png, bomb-home-v19.png, bomb-impact-v19.png and store-bomb-v19.png. The HOME schematic and friendly-fire label are readable. Physical stylus/controller usability and audio remain unverified.

A save written by the actual archived V18 source migrated through the current loader/resave path: beam equipment, resources and old stock retained, new bomb stock initialized. Its backup matched the original SHA-256 exactly: `2daf00c9e32aefcd2e857923fee4a97259896080f3780e4ef0a001c865457235`. Evidence: build/v19-release-migration.json. Selected weapon headers/JSON and coverage data also regenerated identically; build/v19-import-check.json records hashes. Original archive reads were non-mutating.

ROM build passed. ARM9 static sections: 681188 bytes; image high-water: 681216 bytes. Game: 13360 bytes; Campaign: 14416 bytes. Runtime stack/heap and frame time remain unmeasured; the existing toolchain executable-stack metadata warning remains.

Release: releases/v19/ftl-ds-v19.nds, 475136 bytes, SHA-256 `116f4fec72c891cb652bfd875e7b83bc14b357463c8ea2f0265447b78d2636dc`.
Recoverable archive: checkpoints/v19-bomb-weapons.zip, with per-file manifest and SHA-256 sidecar. V18 archive verified before work; older releases preserved.

## Exact next steps

1. Complete equipment controls: unequip, reorder with consistent power priority, explicit cargo/installed sale selection and full-inventory handling. Support empty loadouts safely across UI, combat and saves where ship rules allow.
2. Expand supported original weapon/enemy configurations using the coverage register; review each definition's player availability and required behavior rather than enabling raw entries wholesale.
3. Add drone control, cloaking, augments and ship-specific systems, followed by the corresponding original playable ships.
4. Build larger eight-sector campaigns, visible advancing fleet, sector choices, original event support and final-sector/base defense.
5. Finish crew/species progression coverage, Crystal acquisition, animation sets, hangar/unlocks, tutorial/audio/lid support and the full release gates in FULL_GAME_CHECKLIST.md.
6. Emulator input/audio/DLDI and physical DS input, frame time, memory high-water and SD durability remain pending. Continue independent implementation meanwhile.
