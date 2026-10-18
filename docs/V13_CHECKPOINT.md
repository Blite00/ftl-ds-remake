# V13 checkpoint: boarding and battle aftermath

October 3, 2026. The full base-game goal remains active. This checkpoint records the verified software subset; emulator and physical release acceptance remain pending.

## Implemented

- Store teleporter installation for 120 scrap; 40-scrap upgrades to three bars, reactor allocation, damage, repair and 20/15/10-second recharge.
- Two-person party selection, gather, deploy, recall and HOME/ENEMY room orders. Six-human player roster remains the limit.
- Boarding in both directions, shared faction melee/sabotage, defending crew, oxygen/fire/medical interactions and hostile recall.
- Rebel boarding configuration using the original Fighter's optional teleporter; no additional invented original hull.
- Crew-only results, survivor departure guard, explicitly confirmed abandonment and dead-away cleanup without resurrection.
- Surviving hostile boarders remain after their hull is destroyed. Last friendly crew loss causes defeat.
- Two-second hull-only blast/debris presentation, paused clock and one-time reward dispatch. Crew-only results do not explode the ship.
- Map-only save format 5 includes player teleporter state and crew combat timers. Formats 2–4 migrate; prior valid files are preserved as backup. V9's separate save remains untouched.

## Validation evidence

- The combined 27-check host suite passed. Evidence: `build/v13-checks.txt`; disposable run directory `build/check-ea386bfa7b714beb9b9ea51aa3606632`. Includes focused boarding/campaign/save/outcome coverage.
- Added `boarding_run_smoke`: earned-scrap store purchase, real crew walking, one successful deployment/defender-pursuit/recall sortie and a completed 36-jump six-sector campaign with all three unique crew identities alive. This is one controlled strategy, not coverage of the full game's campaign or boarding content.
- The suite also passed 256 map seeds, three ordinary complete six-sector campaigns and three resumed campaigns with 108 disk cold loads.
- Earlier full-run timeouts used a driver without suitable defender orders; the passing driver now responds to hostile boarding.
- Final ROM build passed: 444416 bytes, SHA-256 `cce40d486caa88a85a985d5ffaa8ffa0e0212567ccb704a315022f050f8dd9d0`.
- Current ARM9 static allocated union: 638740 bytes; high-water offset 638768. ARM ABI Game 9748 bytes; Campaign 10660 bytes. These types are already included in static allocations. Evidence: `build/v13-memory.txt`. Runtime stack/heap and frame timings remain unmeasured.
- Following the final destruction-label correction, the focused boarding UI check passed again, including all 61 frames of both outcomes, overlay handling and framebuffer guards. Host framebuffer previews are not emulator screenshots; Boarding HOME/ENEMY, abandonment confirmation, enemy destruction and player destruction previews were inspected. Destroyed hulls no longer show active shields or player evasion, and stale boarding/details/feedback cannot cover the result screen.
- Release copy: `releases/v13/ftl-ds-v13.nds`. Recoverable source/assets/ROM archive: `checkpoints/v13-boarding.zip` with verified `v13-boarding.sha256`. V12 baseline archive hash was verified before packaging; prior checkpoints and user saves remain intact.
- Emulator input/audio/DLDI and physical DS frame timing, stack/heap, controls and save durability remain pending.

## Intentional approximations and limitations

Humans only, six crew, parties of at most two; arrival checks friendly occupancy by room area rather than individual tile assignment. Melee applies six damage per second per fighter; sabotage removes one system bar per five seconds. Enemy tactics and recharge timing are authored policies. Existing activity overlays are not complete original directional/death animation assets.

Escape, surrender, species traits, resistant doors and sensor-gated information remain unimplemented. Saved encounters are not supported. The campaign remains six sectors with twelve beacons; expanded eight-sector maps, fleet presentation, full event/equipment content, hangar and original flagship mechanics remain unfinished. This checkpoint does not close the full crew/boarding or full-game goal.

## Exact next steps

1. Use the verified V13 source/ROM checkpoint as the recoverable baseline for the next integration.
2. Continue targeted playtesting against alternative boarding and defense strategies; passing scripted routes do not prove overall balance.
3. Implement separate hull/system/crew damage and breaches/sealing, then functional doors/sensors and species with eight-crew support before completing weapon/system/catalog expansion. Larger fleet/maps can proceed independently. FULL_GAME_CHECKLIST.md remains the acceptance definition.
4. Exercise visible emulator and physical DS gates when access is available, continuing independent software work meanwhile.
