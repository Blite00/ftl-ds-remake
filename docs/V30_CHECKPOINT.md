# V30 DS controls, volume and opening balance

## Delivered

- Portable, tested combat controller: D-pad spatial room focus; L/R living crew or weapons; A move/fire; Y crew/aim; X pause; B back/retreat; Start Actions. Safe crew management and unresolved-arrival home defense share applicable controls.
- Actions holds simulation and supplies power, weapon activation, autofire, doors, boarding actions, drones, retreat, sound and contextual beam/bomb targeting. Six spaced rows per page, doubled selected text, page indicators and explicit airlock labels improve readability. Existing touch controls coexist; power menus have visible +/- targets.
- Focus says, for example, SAM TO WEAPONS / A MOVE. Hazard warnings and impact messages retain priority. Some management, naming and specialist confirmations still require touch; full controller-only campaign acceptance is not claimed.
- Higher software gain after the user's report of low volume on a real DS. HIGH is default; OFF/LOW/MED/HIGH can be chosen. Mixer settings are documented in AUDIO.md; no revised acoustic measurements are claimed.
- The guaranteed Scout at beacon 1 stays introductory. Later authored first-sector generated policy4 encounters now mix Interceptor and Armored Drone. Their bursts can penetrate one shield bubble. Imported stats remain intact; BALANCE.md records the intentional DS mix and remaining loadout weaknesses.

## Evidence

103 host checks and 37 compiler tests pass (build/v30-check.log). The controller test was recompiled and rerun after the final wording edit (build/v30-controller-final.log). No old assertions were removed or weakened for this release. The ROM builds with the existing SDK executable-stack metadata warning.

The balance probe runs 2,688 controlled encounters. For 128 later-opening pool seeds, prompt weapon-targeting won 128 with mean 0.39 hull loss; delayed empty-room targeting won 120 with 8 crew defeats; no-input had 82 crew defeats and 46 timeouts. These are isolated action-policy fixtures, not human win rates or a verified original FTL difficulty calibration. Complete earned campaign regression drivers also pass.

Final controller crew/aim/Actions/doors host framebuffers were generated. Actions, page2, doors and focus labels were visually inspected; framebuffer guards and modal input/freeze tests pass. These are distinct from revised hardware input and readability acceptance.

ARM9 static allocated union: 827,764 bytes; image high-water: 827,816. Game 17,708 / Campaign 21,848 bytes; save format23 remains unchanged. Runtime stack, heap and frame-time acceptance remain open.

## Compatibility and release

Current ROM: G:/demake/ftl-ds.nds. Preserved release: releases/v30/ftl-ds.nds. Recovery: checkpoints/v30-ds-controls.zip plus SHA-256 sidecar and per-file manifest. Prior V29 releases/checkpoints remain available. User saves and installed FTL files are preserved.

Old event policies and legacy maps retain their old opening. Policy4 later opening encounters consume an extra RNG draw, so future encounters/random outcomes from an existing policy4 save can differ. Saves remain valid. Volume and controller focus are session settings and are not serialized.

## Acceptance still open / next steps

1. Human real-DS check: confirm HIGH volume is audible and comfortable, listen for distortion, try room selection/movement/firing/power without touch and assess selected text at native size.
2. Complete remaining upgrade/equipment/name/confirmation controller paths, held-key repeat and an end-to-end controller campaign. Inspect usability rather than relying only on framebuffer bounds.
3. Calibrate broader campaign economy, varied original loadouts and hazards against original FTL behavior. Some single-shot enemies still cannot pressure a healthy starting shield. Do not weaken global shields to conceal missing loadouts.
4. Continue the full-game roadmap: surrender callbacks, content/equipment/ships, authentic flagship/base defense, progression and hardware save/audio/runtime acceptance. The full objective remains incomplete.

ROM: 564,224 bytes. SHA-256: `9e165ccd70a9b147bbeef99b264b551288a036c03477ef0d2ef864e212c060ea`. Machine-readable evidence: build/v30-evidence.json.
