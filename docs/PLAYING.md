# Playing FTL DS

Build `ftl-ds.nds` from your own copy of FTL first (see the main README). The ROM contains content converted from your copy: keep it for yourself.

```
INSTALLING ON A FLASHCART
1. Copy ftl-ds.nds anywhere on the flashcart's microSD card (for example the
   root folder). The ROM is about 44 MB.
2. Make sure the card has DLDI patching enabled (most flashcart kernels do
   this automatically). The game reads its music from inside the ROM
   (NitroFS) and writes saves to the card.
3. Start ftl-ds.nds from the flashcart menu.

SAVES AND FILES (in the root of the microSD card)
  /ftl-ds-campaign.sav       your current run (one slot); .bak is the
                             previous good save, .bad a damaged one
  /ftl-ds-profile.dat        hangar unlocks and achievements (.bak backup)
  /ftl-ds.log                diagnostics log (frame timing, events)
Back up these files before updating the ROM. Older saves are migrated
automatically by newer builds.

MAIN MENU
  Continue / New Run (opens the hangar) / Tutorial / Finale (flagship
  tuning) / Controls / Sound. D-pad + A, or touch.

STAR MAP
  L/R select a beacon, A jump, UP zoom, B center, X save, Y ship
  management, START menu. Touch a beacon to select it. The red zone is the
  Rebel fleet; QUEST beacons are purple; stores, exits and distress calls
  are labelled.

COMBAT
  D-pad picks a room, L/R cycles crew or weapons, A orders crew / fires,
  Y aim screen, X pause, B clear/back, START actions menu (power, doors,
  cloak, artillery...), SELECT log viewer. Touch works everywhere.

TUTORIAL
  Main menu -> Tutorial: 26 guided Kestrel steps. SELECT or SKIP leaves it
  at any time; it never touches your saved run.

HANGAR
  New Run opens the hangar: choose a cruiser and Type A/B. Locked ships
  show a silhouette and their original unlock condition. Unlocks and
  achievements are kept in /ftl-ds-profile.dat.

TESTING
  See DS_TEST_CHECKLIST.md in docs/: please note pass/fail and what you
  saw for each step, and include /ftl-ds.log with any report.
```
