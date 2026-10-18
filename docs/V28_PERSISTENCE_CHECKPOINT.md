# V28 development checkpoint: carried intruder persistence

October 3, 2026. Save/resume foundation verified; full player retreat and the full-game goal remain incomplete. V27 remains the latest completed feature release.

## Implemented

- Save format 21 appends a count and up to 48 records after the unchanged historical payload. Empty appendices occupy 4 bytes. Each record occupies 128 bytes: slot, identity, physical room/destination, departed-ship station, ship, species, health, movement/repair/heal/combat/sabotage/ability/death state, four skills and 13 name characters, all explicit little-endian uint32 fields.
- Physical slots 8–55 are retained, including sparse rosters. Carried station values retain their original enemy provenance; home room/destination must exist and be reachable. Names, unique identities, species health, timers and per-room live occupancy are validated. Unpromoted current-enemy home records remain rejected.
- Live and pending-death intruders survive reload. Fully processed corpses are omitted. Home service continues movement, melee, hazards and once-only death effects.
- Saves with carried state restore no remote ship or weapons. The affected screen now says NO ENEMY CONTACT / NO REMOTE SHIP. Load and backup messages direct the player to home defense.
- Save buffers are bounded at 16384 bytes; a compile-time bound covers 8192 historical bytes plus 6148 appendix bytes. Game and Campaign sizes are unchanged.
- Old versions 2–20 load with no carried roster. Save21 keeps previous payload bytes in place and preserves the valid prior file as backup. Existing fixture offsets/format assertions were updated while retaining historical corruption and migration coverage.

## Evidence

- `tools/check.ps1`: 87 C host checks and 28 Python compiler tests passed; `build/v28-persistence-checks.txt`, isolated run `build/check-72dce97286714e9cb614aab7cd1ff45b`.
- Six earned eight-sector regression runs: 389 jumps and 389 disk cold loads. These remain existing-campaign evidence; no player retreat input is available yet.
- `save_intruder_smoke`: all 48 carried records/slot 55 round-trip exactly, 9864-byte full-roster fixture, checksum-correct malformed field/count/slot/ID/name/occupancy rejection, failed loads leave their destination unchanged, pending Zoltan death occurs once, and clean corpse retirement removes false danger.
- `intruder_resume_smoke`: exact full-roster equality, then a sparse last-slot hazard/melee fixture with six disk cold loads. Its final save is byte-identical to uninterrupted 180 service ticks. These are explicitly constructed persistence fixtures, not earned campaigns.
- Three final focused tests passed undefined-behavior traps: `build/v28-persistence-final-focused.txt`. The restored UI and original escape UI were checked again after the final wording correction: `build/v28-persistence-final-ui.txt`.
- Actual archived save20 writer migration: 3716→3720 bytes, old payload prefix exact, four zero count bytes appended, backup exact. Old/backup SHA-256 `ab2fcc1faf4289d0920e81b3d01faec6bb36bfc0dffa2bb10949d7ab1f9366f8`; new `feaa7a272dff3e56b77a4a570030e0175b8a058c7f07b08ddf1fb82af5a73d93`. Evidence and isolated writer/reader/verification sources: `build/v28-intruder-save-migration.json` and its matching directory.
- Independent migration exposed a mismatched disk version header during development; it was corrected and verified directly rather than relying on successful backup fallback. The first broad run also found an outdated Defense-run header assertion; the corrected full run passes. Earlier failure output is retained separately.
- Actual restored framebuffer inspected: `build/carried-resumed-v28.png`. All 104 existing previews remain identical to the V27 manifest; comparison recorded in `build/v28-persistence-preview-comparison.json`.
- ROM build passes with the existing SDK executable-stack metadata warning. Static ARM9 union 781028 bytes, high-water 781056; Game 17708 / Campaign 21748 bytes. Runtime memory/timing remain unmeasured.
- Development ROM: `releases/development/v28-intruder-persistence.nds`, 539648 bytes, SHA-256 `8094ca6b82cc6ed0f9f65a371e76e2c0a83ab56dcaf13a8d507efddc6b42cd72`. Map header remains V27 until the retreat feature is integrated.
- Recoverable development checkpoint: `checkpoints/v28-intruder-persistence.zip`, manifest plus SHA-256 sidecar. Prior intruder-foundation archive verified against `3b313cc8a8823bd73dbfaf3a68329b0f2cc2f6f30d558ede22686e3202c3f743`.

## Exact next steps

1. Make encounter initialization return success; add an explicit validated departure operation. Check readiness/fuel/destination/confirmation before mutation, promote home intruders, explicitly abandon friendly away crew only with confirmation, detach remote state and preserve home hazards.
2. Share arrival resolution with normal travel. Advance fuel/travel/fleet exactly once; clear abandoned source callbacks without salvage, station rewards or boss-phase advancement. Separate current-enemy outcome ownership from ongoing home intruder danger.
3. Provide immediate home-defense access on peaceful/event/store arrival with clear menu/narrative clock behavior. Integrate charge/readiness, linked destination selection, cancel and named abandonment confirmation for both touch and buttons.
4. Test repeated retreats, maximum occupancy, source events, boss retreats, stranded crew and save recovery through ordinary earned campaign runs; inspect all affected screens before completing V28.

No new extracted content was enabled. The serialization layout is a DS implementation choice. Surrender, remaining equipment/content/hulls, authentic finale, progression and release acceptance remain required. Physical DS, emulator input/audio/storage and runtime profiling remain pending.
