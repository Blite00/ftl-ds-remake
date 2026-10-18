# V18 checkpoint: Halberd Beam and point targeting

October 3, 2026. Software integration verified; emulator and physical DS acceptance remain pending. The full base-game goal remains incomplete.

## Implemented

- Original Halberd Beam (`BEAM_2`) at stable catalog ID 5. Imported damage 2 per room, length 80, charge 17 seconds, power 3 and cost 65. Existing IDs 0–4 are unchanged. No ammunition, fire or breach effect for this entry. Raw source omits shots; the beam scheduler emits one sweep.
- Real point-to-point segments cross bounded ship geometry. Each intersected room takes damage once; beams bypass evasion. Shield layers reduce damage without directly consuming shield bubbles. Targeting is independent of projectile room targeting, including ships without a weapons room.
- Two stylus taps select the endpoints, then FIRE/A launches. For controller targeting, L/R selects a room and Down selects its center as an endpoint. Repeated points and premature firing are rejected with feedback. The committed line is shown before firing; switching views/weapons cancels an unfinished point selection.
- A launched beam retains its original endpoints and damage when the next line changes. Autofire retains the committed line; encounter transitions and map loads clear targets. The bounded effects pool queues/retries a beam when full.
- A gold/white sweep distinguishes beams. Store page three now includes Halberd at offer 9, with blank rows safely ignored. Buying requires free cargo and preserves scrap/stock on failure.
- Map-only save format 10 appends beam-offer stock. Formats 2–9 migrate with valid backups retained. Beam targeting and charge remain transient, consistent with the existing map-only save policy.

## Source facts and intentional approximations

Imported weapon values are documented in WEAPON_IMPORT.md and generated/weapon_catalog.json. Geometry uses 32 fixed units per tile; source beam length converts at 35 source pixels per tile, rounding Halberd to 73 units. The second endpoint is clamped to that length. A line must cross a room interior: a shared edge or corner touch alone does not damage that room.

Shield layers are sampled once at impact for the whole line. Each layer subtracts one hull/system damage and five crew HP from the current authored physical payload; zero-damage beams do not cause physical effects. Shield-system damage may reduce shield capacity normally. Damage applies together at the end of a twelve-tick visual sweep rather than continuously following the moving tip. These geometry, crew-damage and timing policies are DS approximations, not recovered PC-engine behavior.

Both-side beam effects and enemy scheduling are supported by focused tests. No new beam enemy loadout is admitted to campaign pools in this checkpoint. Only Halberd is newly playable; other beam entries, bombs and remaining equipment content are still required.

## Validation evidence

44 combined strict host checks passed, plus a separately compiled/verified earned-beam campaign (45 distinct checks, now all registered). After a final correction allowing a valid beam line without a projectile room target, focused beam tests and the earned-beam run passed again. Existing projectile tests retain their original validation rules. Evidence: build/v18-checks.txt, v18-beam-final.txt and v18-beam-run.txt.

The Halberd run buys the weapon for earned scrap, installs it after ordinary upgrades, observes beam impacts with hull damage, completes all six current sectors and performs 36 disk cold loads preserving equipment/cargo/stock. Final hull 30, scrap 92. It injects no resources, damage or victories. Existing ion, boarding, mixed-species, ordinary full-run and 108-checkpoint resumed campaign checks also pass. These runs do not prove the future eight-sector/full-content campaign.

Focused coverage includes exact source values, rational room intersection and corner exclusion, length bounds, shield reduction, no evasion, once-per-room damage, queued snapshots, pause, effect-pool retry, both-side launch, UI point picking, blank rows and frame-buffer bounds. Inspected actual renderer images: beam-target-v18.png, beam-sweep-v18.png and store-beam-v18.png. Physical stylus/controller usability remains pending.

An additional compatibility check compiled the actual archived V17 source to write a format-9 ion-equipped save, then loaded/resaved it through current code. Seed, resources, equipment and old stock survived; new beam stock initialized. The preserved backup matched the original SHA-256 exactly: `fbdba677c81da13745feda978ab21fed4886fd6fe3c5d084668272d0dd6f7c91`. Evidence: build/v18-release-migration.json.

ROM build passed. ARM9 static sections 677612 bytes; image high-water 677640 bytes. Game 13228 bytes; Campaign 14236 bytes. Stack/heap high-water and frame timing remain unmeasured. The toolchain's existing executable-stack metadata warning remains a known build limitation.

Release: releases/v18/ftl-ds-v18.nds, 472064 bytes; SHA-256 `57ac694c1758ffad104c7c71f5174c7853d7d016fc6274f10119c766ea0803a7`.
Recoverable archive: checkpoints/v18-beam-weapons.zip, with manifest and SHA-256 sidecar. The V17 archive hash was verified before work; earlier releases remain preserved.

## Exact next steps

1. Add bombs with shield bypass, evasion, ammunition use and distinct hull/system/crew effects, beginning with a verified original entry and DS target/impact feedback.
2. Complete equipment ordering, removal and exact sale selection; broaden supported weapon/enemy loadouts and original catalog coverage.
3. Continue drone control, cloaking, augments and ship-specific mechanics before enabling their original ships.
4. Expand the generated campaign to eight sectors with larger maps, visible fleet movement, original event support and final-sector/base defense.
5. Continue full crew/species campaign coverage, Crystal acquisition and animation sets, then hangar/unlocks and release gates in FULL_GAME_CHECKLIST.md.
6. Emulator input/audio/DLDI and physical DS performance, lid handling, input and SD durability remain pending; continue independent implementation meanwhile.
