# V26 checkpoint - original station-defense encounters

Date: October 3, 2026. Full base-game goal remains active.

## Playable changes

- Two original complete station-defense closures, attack/avoid decisions, explicit Engage, original hull/crew callbacks and separate station rewards.
- Exact SHIPS_AUTO and SHIPS_REBEL membership, including the new original Rebel Rigger geometry/art/systems and supported laser-plus-drone loadout.
- Enemy repair AI budgets drone power and restores drone control.
- New-run event policy2; old six-root and authored policies remain unchanged. Save 19 uses the same payload layout as 18 and resets transient encounter state on load.
- Wrapped reward hints; controller/touch engagement checks and three-row bounds across every compiled choice.

## Verification

- `tools/check.ps1`: 76 C checks and 24 Python compiler tests passed (build/v26-checks.txt).
- Focused combat fixtures: 128 pool launches, 96 hull/crew reward chains; delay/pause/defeat/away-crew/cap/uniqueness/placement guards. These fixtures force terminal states and do not establish earned playability.
- Six earned eight-sector runs: 392 jumps and 392 disk cold loads, 68 imported decisions, 2 trades, 9 station battles, both roots and all 4 pool enemies. All 9 were hull wins; crew-only station victory remains an earned-campaign coverage gap. Seeds 1, 42, 8675309 at both branch preferences; no injected resources or victories.
- Actual archived V25 writer migration: 3716 to 3716 bytes, entire payload and backup identical; policy1 and actual unique-event bit retained. See build/v26-release-migration.json.
- `tools/preview.ps1`: actual renderer frames generated; inspected station introductions, Engage, hull/crew outcomes/rewards and Rigger normal/aim/boarding. Found and fixed clipped standard reward hint, regenerated and re-inspected.
- `build.ps1`: ROM built. Existing SDK .note.GNU-stack warning remains.
- `python tools/memory_report.py`: static ARM9 union 742188, high-water 742216; Game 13460, Campaign 17500 bytes. These are not runtime memory/performance measurements.

## Release identity

ROM: releases/v26/ftl-ds-v26.nds, 530432 bytes.
SHA-256: `e24f397cbd6b6a512da4748a840eb83ee86c6665328c63ed93c07df4871cbbd8`.
Recoverable archive: checkpoints/v26-station-defense.zip with manifest and SHA-256 sidecar. Archive integrity and each manifest entry are verified when packaging.

## Fidelity and limits

Original source IDs, hashes and compiler boundaries are in V26_EVENT_DESIGN.md and generated catalogs. Original pools/text/topology are imported; placement, fixed equipment, reward numbers, drone cadence and sector hull scaling are adaptations. No unsupported escape/surrender callback is discarded to enable a root. Unsupported content remains disabled. Peaceful ships are narrative context until Engage.

Campaign completeness is not established by these runs: active hazards, original quests, broader systems/equipment/ships, authentic flagship/base defense, hangar progression and release usability remain required. Emulator input/audio/DLDI, physical DS, runtime stack/heap/frame pacing and SD interruption acceptance are pending.

## Exact next steps

1. Implement enemy FTL charging/escape with engine/pilot disruption and explicit outcomes; compile complete AUTO_WARNING including pursuit consequences, while rejecting broader unsupported closures.
2. Add original system/equipment conditions, quests and persistent effects with save migration and earned campaign coverage.
3. Implement beacon-owned hazards that persist through battle, servicing and saves; centralize storm reactor budgets and nebula visibility/pursuit behavior.
4. Continue the remaining full-game checklist: systems/drones/augments, general playable hulls, authentic finale, progression and DS release acceptance.
