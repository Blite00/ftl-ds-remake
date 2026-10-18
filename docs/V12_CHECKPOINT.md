# V12 checkpoint: named crew and living enemies

October 3, 2026. The full base-game DS goal remains active and incomplete.

## Delivered

- Stable crew IDs, bounded persistent names, current ship location and saved stations. Replacement recruits get fresh IDs.
- Named cards, details, skill progress, touch renaming and save/return-stations controls. Details/rename hold simulation. Basic walking/repair/healing/firefighting activity frames use code overlays on existing local art.
- Separate enemy doors/connectivity and shared side-aware routing, atmosphere, damage, repair, healing and fire behavior.
- Original Rebel Fighter rooms, doors, hull art and three human crew. Its supported source laser and selected optional medbay are documented; unsupported optional teleporter is excluded. Appears in the post-first-sector enemy pool.
- Enemy station, repair, fire response and medical AI. Healing power stays within the imported reactor limit. Piloting damage and crew presence affect evasion.
- Distinct hull destruction and crew elimination outcomes; intact enemy crew victories award salvage once, automated ships are exempt.
- Save format4 preserves player IDs/names/stations/location. Formats2/3 migrate while retaining progression; the next write preserves the old valid file byte-for-byte as backup. Future versions are protected from replacement.
- Memory tool now reads symbol sizes directly from compiled ARM ELF objects instead of invoking nm.

## Evidence

- 21 strict host checks pass, including crew names/stations, living AI, power bounds, routing, injury/healing, repairs, side-aware fire/projectile damage, crew-only outcomes, all enemy room targets and guarded framebuffer writes.
- Save tests include v2/v3 migration, full six-person roster, a named dead slot, replacement recruitment after cold-load, invalid IDs/names/stations, checksum-valid malformed data and future-version protection.
- 256 seeded map graphs, three full six-sector campaigns and three resumed full campaigns with 108 disk save/cold-load checkpoints pass. These use public combat/economy APIs. Unit outcome tests inject terminal conditions only for transition invariants; they are separate from full-run evidence.
- Inspected Rebel targeting, crew details and rename framebuffer previews; final crew UI check passed after miniature rendering changes.
- DS build passes. Application compilation is clean; existing SDK GNU-stack linker warning remains.
- ROM: 430080 bytes; SHA-256 `4b74261423e66ab0601348baa318198bbe792fa5749014d5633b941a8000ab2a`.
- ARM9 static main-RAM union: 624164 bytes; high-water offset: 624192. ARM ABI Game: 9604; Campaign: 10512 bytes. Static Campaign and save staging avoid large Game/Campaign stack copies. CPU framebuffers 196608 bytes are already included; VRAM 262144 bytes is separate.
- V11 baseline archive hash verified. Current release is releases/v12/ftl-ds-v12.nds; recoverable archive is checkpoints/v12-named-crew.zip with SHA-256 sidecar.

## Intentional approximations and remaining work

Movement, healing, repairs, skill training and enemy decisions are authored demake rules. Humans only, six-person limit. Activity overlays are not full original directional/combat/death animation sequences. Enemy interiors and crew are visible without sensor upgrades; proper sensor gating is pending. Save policy is still map-only; transient enemy state is not persisted. Crew-only and hull victories currently share ordinary salvage amounts.

Boarding deployment/recall, melee, sabotage, species, stranded-crew resolution, outcome presentation, extra playable ships, larger eight-sector maps, full events/equipment and original flagship mechanics remain unfinished. Per-ship simulation APIs are in place, while player art/furnishings and ship configuration still assume Kestrel. V12 does not complete the entire crew/boarding milestone.

Emulator input/audio/DLDI and physical DS tests remain pending. Host previews do not prove touch comfort, hardware frame times, stack/heap high-water or save durability.

## Exact next steps

1. Add installable teleporter state and management controls, bounded boarding-party selection, power/cooldown and deploy/recall APIs.
2. Implement shared room occupancy, melee and sabotage across factions; enemy boarding orders, medbay/fire/oxygen interactions and clear ship-view selection.
3. Resolve dead, away, stranded and returning crew on hull loss, crew-only victory, escape and encounter transitions without duplication.
4. Separate terminal outcome presentation from simulation/reward application; add hull-only ship explosions and death/boarding activity frames.
5. Continue all system/equipment, larger campaign/fleet, events, hangar and release gates in FULL_GAME_CHECKLIST.md. Hardware checks proceed when input and hardware are available.
