# V39 enemy boarding AI

## Sources

- Disassembly of the installed FTL 1.6.14 `FTLGame.exe` (read-only):
  - `ShipAI::OnInit` (0x625410): party sizes.
  - `ShipAI::GetTeleportCommand` (0x6257d0): send/receive commands and the send counter.
  - `CompleteShip::InitiateTeleport` (0x409ae0): landing room.
  - `ShipAI::SetEscapeTimer` default (15 s).
- [XFTL `doc/ship-ai` and `doc/crew-ai`](https://gitlab.com/znixian/xftl/-/tree/master/doc), reviewed 2026-10-05: evac conditions, the teleport-task priorities in `CrewAI::PrioritizeTask`/`AttemptToAssign`, and `CrewAI::UpdateIntruders`/`PrioritizeIntruderRoom`.
- These were not compared with controlled play sessions in the original game.

## Verified behavior and DS implementation

| Rule | Evidence | DS |
|---|---|---|
| Boarding mode is set from the blueprint: sabotage for most ships, invasion for the flagship | XFTL; `OnInit` branches on `boardingAI` 1/2 | `enemy_board_mode`: 1 for any enemy with a teleporter, 2 reserved for `ENEMY_FLAGSHIP` |
| Sabotage requests `crew/2` and keeps `crew - crew/2` home. Invasion requests `3*crew/4` | `sar 1` at 0x625485; `lea [eax+eax*2]; sar 2` at 0x6254a0 | `boarding_enemy_request` evaluates the counts the first time the AI runs, before any losses |
| Every loop the request is (crew aboard - stay). A negative value asks for a boarder back | XFTL; `iTeleportRequest < 0` takes the receive branch at 0x6258cd | Same; a negative request forces evacuation |
| After a send fills the request, `iTimesTeleported++`. Sending stops at 2 | 0x625888; XFTL | `enemy_board_sends` (encounter-local) |
| AI sends use target room -1, so `rand() % roomCount` picks a uniformly random player room | 0x409b70 → 0x409d8b | `landing_room` picks uniformly among rooms with free space. V37 always used weapons |
| Teleport tasks have priority 32 when the teleporter is charged and 65 otherwise. Crew below 50% health, or bad at combat (<1x damage or <100 HP) while not everyone is, get 100 | XFTL `PrioritizeTask` | `boarding_enemy_party` |
| Each task goes to the least-busy crewmember (highest current task value) whose current task is less important. Manning values: piloting 64, shields 65, weapons 66, engines 67, oxygen 68, sensors 69, doors 70, teleporter 71, medbay 72, idle 1000 | XFTL `AttemptToAssign` | The pilot boards only after others. Engi and Zoltan crew who are manning a station stay home |
| Sabotage boarders below 25% health are recalled. Invasion boarders have no minimum | XFTL `BoardingGoal`, `ReadyForEvac` | Same |
| Forced evac happens when escaping with less than 15 s left, or with 3 or more completely broken systems on the AI ship | XFTL `RequiredEvac`; 15 s default at 0x6254e0 | Counts main systems, doors/sensors, teleporter, drones and piloting |
| Evac teleports back from the first qualifying boarder's room | XFTL `ReadyForEvac` | Everyone in that room returns, up to two |
| Intruder room scores: fire 100, air below 10% 70, +5 when slots differ from the party, +10 when not adjacent, +20 without a working system. The current room wins ties; otherwise one of the best is chosen at random. Each boarder's choice is also ordered for the boarders after it | XFTL `PrioritizeIntruderRoom`, `UpdateIntruders` | Implemented, including the cascade and random tie-break. V37 had the base scores |

## DS approximations (not claimed as original)

- The teleporter pad holds two crew, so larger requests take several sends; a send counts as completed only when it fills the request. Teleporter cooldown remains the V22 authored formula.
- The original sends whoever stands on the pad when `ReadyForTeleport`; the DS sends staged healthy crew on the pad.
- The AI runs once per simulated second. Room-level positions replace slot positions. When the random landing room is full, the DS picks among rooms with space; the original's slot overflow is unverified.
- Some intruder-score details are omitted: ion-drone handling, the doors-broken quirk and crew-mode targeting.
- The port's LCG replaces `rand()`.

## Tests

`tests/enemy_boarding_v39_smoke.c` covers sabotage and invasion sizes, the two-send limit, landing spread, health/forced/stay-behind evacuation and fighter selection. It also checks charged versus uncharged priority, scheduler staging and grouped targeting with ties over 20 seeds. Existing boarding, crew AI and campaign suites pass. One fixture that installs a teleporter by hand now enables sabotage mode explicitly.
