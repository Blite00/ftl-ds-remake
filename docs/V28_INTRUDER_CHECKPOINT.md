# V28 development checkpoint: carried intruder foundation

October 3, 2026. Verified development integration; player retreat and the full game remain incomplete. V27 remains the latest completed feature release.

## Implemented

- Partitioned hostile roster: eight current-enemy slots plus 48 carried-home slots; total high-water count covers both. The Kestrel has 48 floor cells. Future hulls must validate their floor bounds before enabling this roster policy.
- `game_carry_intruders` atomically promotes living or pending-death current home crew, preserving every Crew field. It does not consume fuel, leave the encounter or enable a jump. Insufficient capacity and invalid carried locations fail without mutation.
- Encounter replacement preserves carried slots. Current-enemy counts, AI station ownership and teleporter masks remain restricted to the first eight; masks never shift by carried indices.
- Pending Zoltan deaths survive promotion and encounter replacement and resolve once. A dead record cannot be reused before its death is processed.
- Hostile home movement, melee and sabotage run during service simulation. Remote enemy crew remain frozen; existing friendly-away service rules are retained. Hostiles cannot repair or heal using player systems or contribute player reactor power.
- Species bounds, ability clocks and death chains include the final hostile slot. Existing dynamic renderer/fire/breach loops cover the expanded roster.
- Save20 refuses live or pending-death home hostile state because it has no hostile appendix. Invalid high-water counts also reject. This temporary guard prevents data loss; it is not the finished save or retreat policy.

## Validation

- 85 C host checks and 28 Python compiler tests passed: `build/v28-intruder-checks.txt`, isolated directory `build/check-1701ed1ceb9949d5a948c98d0d8059a0`.
- Six ordinary earned eight-sector runs: 389 jumps/cold loads, 70 original choices and two trades. These remain existing-campaign regression evidence, not completed retreat campaign evidence.
- Final pending-death changes were additionally checked with all four relevant tests: carried core, carried hazards, carried UI/save and player FTL. All pass `-fsanitize=undefined -fsanitize-undefined-trap-on-error`; log `build/v28-intruder-final-focused.txt`. The separate UBSan runtime library is unavailable, so trap instrumentation was used.
- Roster fixture fills all 48 Kestrel cells through six promotions and encounter replacements, checks byte-preserved Crew records, identity uniqueness, overflow failure, delayed deaths and unrelated-teleporter rejection. This is an explicit fixture, not an earned campaign.
- Last-slot fixtures cover movement/melee/sabotage, fire/Rock immunity, oxygen loss, species power/repair/healing exclusions, pause/remote freeze and chained deaths.
- UI/save check renders slot 55 with framebuffer sentinels, confirms its visibility and unmodified identity, and verifies rejection leaves an existing disk checkpoint unchanged. A clean checkpoint still reloads.
- Inspected `build/carried-home-v28.png` from the actual renderer. All 104 regenerated V27 previews remain byte-identical: `build/v28-intruder-preview-comparison.json`.
- ROM build passes. Existing SDK `.note.GNU-stack` warning remains.
- Static ARM9 union 760740 bytes, high-water 760768; Game 17708 / Campaign 21748 bytes. Runtime budgets remain unmeasured.
- Development ROM: `releases/development/v28-intruder-foundation.nds`, 536576 bytes, SHA-256 `8169ff1dc7bc4235ec4098dd58f2013e3d52bb8ba5a37e3033051db439316b6a`. Existing map header remains V27; player retreat controls are not advertised.
- Prior charge-foundation checkpoint verified: `3a315f1f0315a5402a5cd661b369279660c5bdc6d60954280703401ee57f7d6c`.
- Recoverable checkpoint: `checkpoints/v28-intruder-foundation.zip`, verified manifest and SHA-256 sidecar.

## Exact next steps and limits

1. Add a versioned carried-intruder save appendix with bounded records, unique IDs, valid home rooms/routes, health/species/timer checks and capacity analysis. Migrate old saves with no carried crew and preserve backups. Replace the temporary rejection only when reload can preserve every live intruder and pending consequence.
2. Make encounter initialization report success and implement validated departure/common arrival, including fuel/fleet travel, explicit friendly-away abandonment and detached remote state. Handle source-event callbacks and boss phases without retreat rewards or resets masquerading as victory.
3. Resolve enemy battle outcome separately from carried home danger. Add playable home-defense access on peaceful/event/store arrivals while retaining explicit narrative/menu clock rules.
4. Integrate charge/readiness, linked destination selection, cancel and named abandonment confirmation for touch/buttons; run ordinary earned retreat campaigns, migration checks and affected framebuffer inspections.

No new original assets or content pools were enabled. The carried-roster layout is a DS implementation choice; the charge timing remains the previously documented approximation. Surrender, broader systems/content/hulls, authentic finale, progression and full release gates remain open. Physical DS, runtime timing/memory and emulator input/audio/storage acceptance remain pending.
