# V38 enemy weapon AI

## Sources

- Primary evidence: read-only disassembly of the locally installed `FTLGame.exe` (FTL 1.6.14 Windows, per `FTL.log`). Functions were located with the FTL Hyperspace project's 1.6.9 Win32 byte signatures (`CombatAI.zhl`); `LinkShip`, `SetTarget`, `ChooseSecondTarget` and `ChooseTarget` matched exactly. The constructor, `PrioritizeSystem` and `UpdateWeapons` were identified by position and call structure. No game code or data is copied into the ROM.
- Cross-checks: [XFTL `doc/combat-ai`](https://gitlab.com/znixian/xftl/-/blob/master/doc/combat-ai) and Hyperspace's `CombatAI::UpdateWeapons` rewrite (`src/game/Gameplay/CustomWeapons.cpp`). Both were reviewed 2026-10-05.
- This is reverse-engineering evidence. It has not been compared with controlled play sessions in the original game.

## Verified behavior (normal difficulty)

| Rule | Evidence |
|---|---|
| `CombatAI` constructor sets `system_targets = {shields, engines, weapons, oxygen, drones}` (IDs 0,1,3,2,4) | Five consecutive `push_back` constants at 0x62a68d–0x62a6f3 |
| `PrioritizeSystem`: easy 20%, normal 33% (`rand()%100 < n`) to pick `system_targets[rand()%5]`; it is used only if the player has that system. Otherwise it returns -1 | `sbb/and/add` producing 20/33 at 0x62b224; `HasSystem` call at 0x62b291 |
| `ChooseTarget`: -1 → uniform `rand() % roomCount` room; else the system's room. The aim point is the room centre | 0x62bf10–0x62bfaf |
| `UpdateWeapons`: each charged weapon fires independently. There is no cross-weapon volley synchronisation on any difficulty | Loop at 0x62c07a; no difficulty read |
| One target is chosen per required target. A duplicate removes the earlier equal point before append, so targets are distinct while rooms allow | `std::remove` + append at 0x62c235–0x62c2d5 |
| `NumTargetsRequired` is a per-weapon field; beams need two targets (start, aim) | 0x40d5f0; Hyperspace's `NumTargetsRequired` comment |
| Skip firing if the player is cloaked or the AI is cloaked without `bFiringWhileCloaked` | XFTL; consistent with the vtable check at 0x62c0e7 |
| Hard mode uses a priority list (shields if powered, etc.) with 50/25/25 odds | XFTL only; not implemented, since the DS port uses normal difficulty |
| `ChooseSecondTarget(p)` = `p` + 100 px at `rand()%360`° | 0x62b190. No direct caller was found in `UpdateWeapons`, so it is not used for ship weapons |

## DS implementation

- `game_enemy_plan_volley` implements PrioritizeSystem and ChooseTarget for normal difficulty. Drones enter the list only when the player has the drone system. Medbay, piloting, sensors and doors receive only the random-room share. A volley request is capped at the player's room count; vanilla would hang, and Hyperspace fixes it the same way.
- Every enemy volley plans all targets at fire time. Rounds launch in order against those rooms. A queued volley keeps its plan after later power loss, so V31's queued-payload guarantee still holds.
- Beams start at the first chosen room centre and point toward the second. The beam is extended to its full length and trimmed only at the player hull's bounding box. Rooms lie inside that box, so damage is unaffected. The previous fixed adjacent-room beam choice is removed.
- Both the authored two-slot loadouts and the legacy single-weapon path use these planners. Bombs and ion shots use the same room planner.

## DS approximations (unchanged, not claimed as original)

- The 0.2 s spacing between burst rounds and the 12-tick projectile flight remain authored.
- The manned-weapons charge bonus, enemy reactor/weapon power order and automated repair timing remain authored. ShipAI::CheckPowerLevels from XFTL is not yet ported.
- Charge-goal logic (`SelectChargeGoal`, early fire with chance 1/(remaining+1)) applies only to Advanced Edition charge weapons, so it is out of scope.
- Enemy combat drones still target a random room every authored interval.
- The random generator is the port's LCG, not MSVCRT `rand()`. Distributions match, but sequences do not.

## Test evidence

`tests/enemy_weapon_ai_smoke.c` checks the per-room distribution over 20,000 draws (±15%), including the absent-drones fallback, medbay exclusion and drones joining once installed. It also checks distinct three-room volleys, the cap at room count, and beam validity/length. An integrated legacy volley must fire in planned order. `enemy_loadout_smoke` now checks that a queued Burst volley keeps three distinct planned rooms across power loss.

Balance effect: shots now reach corridors and empty rooms (about 67% random rooms instead of 100% systems). This means less system damage and more hull/fire/breach spread. The scripted campaign pilot in `campaign_v10_smoke.c` previously repaired only system rooms; it now also fights fires and seals breaches as a group and retreats to the medbay when hurt, using ordinary crew orders. No game rule was relaxed to make tests pass.
