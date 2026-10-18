#ifndef FTL_DS_DRONE_TYPES_H
#define FTL_DS_DRONE_TYPES_H
#define GAME_MAX_DRONES 3
#define GAME_DRONE_CATALOG_COUNT 9
#define GAME_DRONE_COOLDOWN_TICKS (10*GAME_HZ)
#define GAME_DEFENSE_COOLDOWN_TICKS GAME_HZ
/* V56 unit fields (encounter-only, never saved): interior/boarding drones keep a
 * room, health and work timer; hull repair keeps its remaining repairs; boarding
 * drones count flight time before landing. */
typedef struct { int catalog_id,enabled,deployed,charge_ticks,target_room; int unit_room,unit_health,work_ticks,remaining,flight; } DroneState;
typedef struct {
 int installed,room,capacity,health,power,bonus_power,ion_bars,ion_ticks,slot_count;
 DroneState slots[GAME_MAX_DRONES];
} DroneControl;
#endif
