# Physical DS test checklist (V71)

Use `releases/v71/ftl-ds.nds` on the target DS and flashcart. Copy any existing `/ftl-ds-campaign.sav` first. Note **pass/fail plus what you saw** for each line. Nothing here has been verified on hardware yet.

## Boot and storage
1. The ROM launches from the flashcart menu with its icon and title. The title menu shows V71, with Continue enabled when a save exists.
2. Continue loads the existing run. X on the map saves. Power-cycle, then Continue resumes the same beacon and scrap.
3. **Listen during a save:** the music may stutter (see the host analysis). Note whether it stutters and for how long.

## Input
4. Rapid separate taps each register, and holds do not repeat.
5. In combat, the D-pad, L/R, A/B/X/Y and Start each do what Controls lists.
6. Crew setup: the keyboard, editing names and LAUNCH all work.
7. Main menu FINALE switches between TUNED and ORIGINAL by touch and with A.

## Timing and pacing
8. Time one weapon's full charge with a stopwatch in a quiet fight, then in a busy one (flagship stage 2 with drones, or a boarding fight). The two times match.
9. Pause, the Actions menu and the logs (Select) hold time. Resuming does not jump ahead.
10. In a long fight of 5 minutes or more, animations stay smooth and there are no stalls over a second.

## Audio
11. Each screen plays its music: menu, map, battle, finale (Last Stand) and victory.
12. Volume steps OFF/LOW/MED/HIGH are audible, and HIGH is loud enough on the speaker.
13. Overlapping volleys and shield hits play without crackling. Music ducks under effects and recovers.
14. **Listen for dropouts:** no gaps in the music during busy combat, map jumps and sector changes.

## Finale (flagship)
15. With TUNED, each stage is winnable with a reasonable build. Stage 2's drones and the SURGE warning appear, and stage 3's SUPER shield absorbs hits and blocks the teleporter.
16. With ORIGINAL on a new run, the flagship visibly has more shield bars.
17. Victory and defeat screens return to the menu.

## V46 combat view
19. The system icons are readable on the DS screen and change colour when a system is damaged, destroyed, unpowered or ionized. Crew visibly walk through doors. Incoming shots show target brackets. On the aim screen, your shots show HIT, MISS or SHIELD tags. Moving the gunner out of the weapons room shows UNMANNED and stops firing.

## V55 cloaking and augments
20. Buy cloaking at a store (more offers, page 4). In combat, Actions shows CLOAK POWER and ACTIVATE CLOAK. While cloaked, the Kestrel is drawn faded, CLOAK nS counts down on the top screen, and enemy weapon bars stop charging. Afterwards the system shows a recharge wait. Buy an augment and check that it appears on the Upgrade page.

## V56 drones
21. Buy a drone schematic from a store's second drone offer and fit it in the Drones panel between fights. Deploy it in combat. Repair and anti-personnel drones show as a small cyan square in their room. A boarding drone breaches an enemy room after about 3 s and shows there on the aim view. A hull repair drone visibly restores 3 to 5 hull, then reads as spent.

## V57 hangar
22. New Run opens the hangar. The D-pad moves between ships, Y/L/R switches Type A/B, and touching works too. Locked ships show a dark silhouette with their full unlock text. START only works on unlocked ships. B returns to the menu. Crew naming shows the chosen ship.
23. Reach sector 5 with the Kestrel: a "New ship unlocked" message appears. Reopen New Run: the Engi Cruiser (The Torus) is now selectable and plays with 3 crew, an Ion Blast II, a Combat Drone and 15 drone parts. The file `/ftl-ds-profile.dat` and its `.bak` copy exist on the SD card. The campaign save is unaffected.
24. With a Hull or System Repair drone, deploy it from the Drones panel between fights while the crew page is open and unpaused: hull or systems repair.

## V58 artillery and store systems
25. Once the Federation Cruiser is unlocked: in combat, `ART nS` counts down from about 40 s and the Artillery Beam fires through shields. The Actions menu has an ARTILLERY power row, and the Upgrade page has an ARTY upgrade button.
26. On a ship without shields (Stealth), a store's last page can offer "Shield system (install)". After buying it, shields appear in their room and survive a save and reload.

## V59 achievements
27. Earn an achievement, for example by letting the Kestrel drop to 1 HP and then repairing it fully. An "Achievement: ..." message appears. In the hangar, Kestrel Type B lists its three achievements with progress. After two of them, the Red-Tail is selectable. The progress survives a power cycle and lives in `/ftl-ds-profile.dat`, not in the campaign save.

## V60 hazards
28. At a hazard beacon (map hazard icon), check that each banner reads clearly: ASTEROID FIELD, SUN: SOLAR FLARE nS (red near zero), NEBULA: SENSORS BLIND, or ION STORM: REACTOR HALVED (the power readout shows half). Flares visibly start fires on both ships.

## V61 quests
29. When an event mentions a quest, a purple QUEST beacon appears on the map (or the text says it will be in the next sector). Visiting it plays the quest. Save and reload with a quest open: it is still there.

## V62 events
30. Long event replies fit inside their boxes, with no cut-off text. Intruders that beam aboard without a ship are fought on the crew page. A store reached through an event opens after Continue. The original Engi, Mantis, Rock, Slug and Zoltan story quests can appear from sector 2 on. Report any you complete, and whether they unlock the cruiser in the hangar.

## V64 tutorial
31. Main menu → TUTORIAL. Each step's instruction is readable on the top screen. Do the action asked (power the engines, send crew, open the airlock doors, jump, power the shields and the laser, fire, missile). SKIP or SELECT returns to the menu at any point. Afterwards, Continue still loads your run unchanged.

## V68 shots and alarms
32. In combat, your shots leave the gun mounts on the front of your ship and fly toward the enemy, and enemy shots fly toward you. A hit lands in the room you aimed at, a miss flies past, and a shot your shields block stops at the bubble. Check a Type B ship too.
33. When you are hit you hear the original hull-hit sound and your ship shakes and flashes red. A fire, breach, low oxygen, low hull or boarders each sound one alarm, show its original warning, and go quiet once fixed.
34. On the aim screen (Y), aim weapon 1 and weapon 2 at different rooms. Each room shows its weapon's number, and with autofire on each weapon hits its own room. B clears the selected weapon's target, and that weapon stops firing until you pick a room again.
35. When your ship is hit, the room that was hit flashes red with a "-N" damage tag on the bottom screen and blinks on the top-screen overview. Crew in that room lose about 15 HP per point of damage. Crew in other rooms are not hurt.

## Stability
36. Play a full run, or at least 30 minutes, with no crash, freeze or corrupted graphics. Close and reopen the lid; play resumes cleanly.

Report any failure with the step number and, if possible, a photo. The diagnostics log `/ftl-ds.log` on the SD card holds per-frame work timing (`work_us` lines); please include it.
