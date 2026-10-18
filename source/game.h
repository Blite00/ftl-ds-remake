#ifndef FTL_DS_GAME_H
#define FTL_DS_GAME_H
/* Native reconstruction: timing, movement, repairs and AI are provisional. */
#include "drone_types.h"
#define GAME_HZ 30
#define GAME_SHIELD_RECHARGE_TICKS (2*GAME_HZ)
/* Authored DS timing: 40 seconds at one effective engine bar. */
#define GAME_PLAYER_FTL_CHARGE (40*GAME_HZ)
#define GAME_MAX_ROOMS 32
#define GAME_MAX_DOORS 48
#define GAME_CREW_COUNT 3
#define GAME_WEAPON_COUNT 2
#define GAME_MAX_CREW 8
/* Current-encounter enemy slots; the flagship carries eleven crew. */
#define GAME_MAX_ENEMY_CREW 12
#define GAME_MAX_INTRUDERS 48
#define GAME_MAX_HOSTILES (GAME_MAX_ENEMY_CREW+GAME_MAX_INTRUDERS)
#define GAME_MAX_WEAPONS 4
#define GAME_MAX_ENEMY_WEAPONS 2
/* Per-volley planned targets; base-game weapons fire at most three shots. */
#define GAME_MAX_VOLLEY 4
/* V51 base-game weapon list: 0..6 are the historical catalog (saved ids), 7 is
 * the enemy-only Artemis variant, 8.. are imported additions (weapon_catalog.h). */
#define GAME_ENEMY_ARTEMIS 7
#define GAME_CATALOG_COUNT 39
#define GAME_WEAPON_ARTILLERY_FED 38
#define GAME_SYSTEM_COUNT 5
/* ENEMY_SHIP (V52): any imported base-game ship, chosen by Game.enemy_ship. */
enum GameEnemyKind { ENEMY_SCOUT, ENEMY_INTERCEPTOR, ENEMY_ARMORED, ENEMY_FLAGSHIP, ENEMY_ASSAULT, ENEMY_REBEL, ENEMY_REBEL_BOARDER, ENEMY_REBEL_FAT, ENEMY_SHIP, ENEMY_KIND_COUNT };
#define GAME_MAX_EFFECTS 12
#define GAME_EFFECT_IMPACT_TICKS 8
enum GameEffectKind { EFFECT_LASER, EFFECT_MISSILE, EFFECT_ION, EFFECT_BEAM, EFFECT_BOMB };
enum GameEffectResult { EFFECT_PENDING, EFFECT_HIT, EFFECT_SHIELD, EFFECT_MISS, EFFECT_INTERCEPT };
/* duration is travel time; the resolved impact remains visible for 8 ticks.
   side 0 targets the enemy, side 1 targets the player. room is a room ID. */
/* hull_bust doubles hull damage to rooms without a system; lockdown seals the room;
 * ion is carried by ion bombs; negative crew damage heals (Healing Burst). */
typedef struct { int hull,system,crew,fire_chance,breach_chance,hull_bust,lockdown,ion; } GameDamage;
typedef struct {
    int active,age,duration,side,room,kind,result,damage,piercing,fire_chance,system_damage,crew_damage,breach_chance,ion_damage;
    int beam_x0,beam_y0,beam_x1,beam_y1,bomb_friendly,hull_bust,lockdown;
    int origin; /* V59: player shot source (achievements.h AchOrigin). */
    int mount; /* V68 firing weapon mount + 1 (0: no mount, e.g. drones); render only, not saved. */
} GameEffect;
enum GameSystem { SYS_SHIELDS, SYS_ENGINES, SYS_WEAPONS, SYS_OXYGEN, SYS_MEDBAY };
enum GameResult { RESULT_NONE, RESULT_PLAYER_HULL, RESULT_ENEMY_HULL, RESULT_PLAYER_CREW, RESULT_ENEMY_CREW, RESULT_ENEMY_ESCAPE, RESULT_PLAYER_ESCAPE, RESULT_ENEMY_SURRENDER };
#define GAME_CREW_NAME_SIZE 13
#define GAME_CREW_NAME_LEN GAME_CREW_NAME_SIZE
enum GameOutcome { GAME_PLAYING, GAME_VICTORY, GAME_DEFEAT, GAME_ESCAPE, GAME_SURRENDER };
/* hit_ticks/hit_damage (V70): a short impact flash on the hit room; display only, never saved. */
#define GAME_ROOM_HIT_TICKS (GAME_HZ*3/4)
typedef struct { int id,x,y,w,h,system,oxygen,exposed; int fires,fire_ticks,firefight_ticks,breaches,breach_repair_ticks,lockdown_ticks; int hit_ticks,hit_damage; } GameRoom;
/* open is effective state; transit_ticks > 0 means an automatic opening. */
typedef struct { int x,y,room_a,room_b,vertical,open,transit_ticks,force_ticks,force_idle_ticks,jam_ticks; } GameDoor;
enum GameSubsystem { SUB_DOORS, SUB_SENSORS, SUB_COUNT };
typedef struct { int room,capacity,health,ion_bars,ion_ticks; } Subsystem;
typedef struct { int room,capacity,health,power,bonus_power,ion_bars,ion_ticks; } GameSystemState;
typedef struct {
    int hull,max_hull,shields,shield_ticks,oxygen,reactor,room_count,automated;
    GameRoom rooms[GAME_MAX_ROOMS];
    GameSystemState systems[GAME_SYSTEM_COUNT];
} Ship;
typedef struct { int room,destination,move_ticks,repair_ticks,health,heal_accumulator; int skill[4]; unsigned id; char name[GAME_CREW_NAME_SIZE]; int station,ship,combat_ticks,sabotage_ticks,species,ability_ticks,death_processed; } Crew;
typedef struct {
    const char *name;
    int cooldown_ticks,charge_ticks,power,damage,shots,missiles;
    int shield_piercing,pending_shots,shot_ticks,target_room;
    int catalog_id,enabled,aim_room,charge_accumulator;
    int system_damage,crew_damage,fire_chance,breach_chance,hull_bust,lockdown;
    GameDamage queued_damage;
    int ion_damage,queued_ion_damage,bomb,queued_bomb,bomb_side,queued_bomb_side;
    int beam_length,beam_target_valid,beam_x0,beam_y0,beam_x1,beam_y1;
    int queued_beam_length,queued_beam_x0,queued_beam_y0,queued_beam_x1,queued_beam_y1;
    int queued_target_count,queued_targets[GAME_MAX_VOLLEY]; /* Enemy volleys only. */
} WeaponState;
typedef struct { int installed,room,capacity,health,power,cooldown_ticks,bonus_power,ion_bars,ion_ticks; } Teleporter;
/* V42 flagship artillery: an independent room system with its own weapon.
 * Charge rate scales with effective power (ArtillerySystem: 1.5 - 0.25*power). */
#define GAME_MAX_AUGMENTS 3
#define GAME_MAX_ARTILLERY 4
typedef struct { int room,capacity,health,power,ion_bars,ion_ticks,charge,weapon; WeaponState w; } Artillery;
/* Enemy cloaking: 5 s per bar; ending applies 4 ion bars (20 s lockout). */
typedef struct { int room,capacity,health,power,ion_bars,ion_ticks,active_ticks,fire_while_cloaked; } EnemyCloak;
/* V43 flagship stage-2 drones (BOSS_2 droneList order) and the power surge. */
#define GAME_FLAGSHIP_DRONES 4
#define GAME_SURGE_DRONES 6
enum FlagshipDroneKind { FDRONE_DEFENSE, FDRONE_COMBAT, FDRONE_BEAM, FDRONE_BOARDER };
typedef struct { int kind,deployed,charge,flight,crew; } FlagshipDrone; /* crew: landed boarder slot or -1. */
typedef struct { int kind,shots,next_tick; } SurgeDrone;
typedef struct { int room,capacity,health,power,charge_ticks,cooldown_ticks,damage,fire_chance,shots,ion_bars,ion_ticks; } EnemyDrone;
#define USED_WEAPON 1
#define USED_MISSILE 2
#define USED_DRONE 4
#define USED_ARTILLERY 8
typedef struct { int used,hull_hit,crew_lost,enemy_kills,cloak_full,cloak_avoided,zoltan_broken,defense_drone,killing_origin,ended,start_shield,prev_hull; unsigned enemy_alive,crew_alive,earned; } GameAchTrack;
typedef struct {
    Ship player,enemy;
    Subsystem subsystems[2][SUB_COUNT];
    int pilot_ion_ticks[2];
    EnemyDrone enemy_drone;
    DroneControl drones;
    int drone_parts;
    Teleporter teleporter[2];
    Crew crew[GAME_MAX_CREW],enemy_crew[GAME_MAX_HOSTILES];
    int enemy_crew_count,enemy_pilot_room,enemy_pilot_health,result_reason;
    unsigned next_crew_id;
    GameDoor enemy_doors[GAME_MAX_DOORS];
    int enemy_door_count;
    unsigned char enemy_adjacency[GAME_MAX_ROOMS][GAME_MAX_ROOMS];
    WeaponState weapons[GAME_MAX_WEAPONS];
    WeaponState enemy_weapon;
    WeaponState enemy_weapon_secondary;
    int enemy_weapon_count; /* 0 retains legacy single-weapon behavior; 1/2 opt in. */
    GameEffect effects[GAME_MAX_EFFECTS];
    GameDoor doors[GAME_MAX_DOORS];
    int door_count;
    int crew_count,weapon_count,pilot_room,pilot_health,enemy_kind;
    int paused,outcome,ticks,target_room,missiles;
    int autofire[GAME_MAX_WEAPONS]; /* Encounter-local; off on init/jump/load. */
    int enemy_fire_ticks,enemy_pending_shots,enemy_shot_ticks,enemy_missiles;
    int player_ftl_charge; /* Transient, 0..GAME_PLAYER_FTL_CHARGE units. */
    int enemy_escape_ticks,enemy_escape_total,enemy_escape_threshold,enemy_escape_active,enemy_departed;
    /* Encounter-local original boarding AI (V39): 0 none, 1 sabotage, 2 invasion;
     * stay-behind count (-1 until first evaluated) and completed sends (max 2). */
    int enemy_board_mode,enemy_board_stay,enemy_board_sends;
    /* V42 flagship stage (-1 otherwise), artillery and cloaking. */
    int flagship_stage,artillery_count;
    int interim_flagship; /* Set by legacy/policy-0 campaigns: keep the V28 authored platforms. */
    Artillery artillery[GAME_MAX_ARTILLERY];
    EnemyCloak enemy_cloak;
    FlagshipDrone flagship_drones[GAME_FLAGSHIP_DRONES];
    int flagship_drone_count,flagship_drone_parts;
    int surge_ticks,surge_beams; /* Ticks to the next surge (0 = none); beam share of each drone surge. */
    int surge_count,super_shield,super_shield_max; /* V44 stage 3: laser surges fired; Zoltan shield points. */
    int surge_drones; /* Drones per stage-2 surge (6 on normal). */
    int flagship_authentic; /* 1: original normal numbers; 0 (default): DS finale tuning, docs/V44_CHECKPOINT.md. */
    int enemy_ship; /* V52: imported_enemies index for ENEMY_SHIP encounters (encounter-local). */
    /* V55: player cloaking (capacity 0 = not installed; active/ion are encounter-local),
     * augments (augment_catalog.h ids, -1 empty) and the Zoltan shield points. */
    EnemyCloak player_cloak;
    int augments[GAME_MAX_AUGMENTS];
    int player_super_shield;
    /* V57: player layout (player_ships.h index: class*2+variant, 0 = Kestrel A) and
     * its weapon slot count. Both follow from the layout and are not saved. */
    int player_ship,weapon_slots;
    /* V58 Federation Artillery Beam (capacity 0 = none): system levels 1-4, charge in
     * ticks; fires the ARTILLERY_FED beam across the enemy when charged. */
    Artillery player_artillery;
    /* V59 achievements: encounter tracking (achievements.c), the source tag for the next
     * player launch, and the run-long Kestrel "hull fell to 1" latch (saved). */
    GameAchTrack ach;
    int launch_origin,tough_low;
    int launch_mount; /* V68 transient: mount + 1 for the next launch (GameEffect.mount). */
    /* V60 beacon hazard for this fight (GameEnvironment) and its next asteroid/flare tick. */
    int environment,env_ticks;
    /* V62 event status: power cap per side and system code (0-4 main systems,
     * 6 sensors, 7 doors; -1 none). Lasts until the next jump. */
    int status_cap[2][8];
    SurgeDrone surge[GAME_SURGE_DRONES];
    unsigned int rng;
    unsigned char adjacency[GAME_MAX_ROOMS][GAME_MAX_ROOMS];
    char message[96];
} Game;
void game_init_crew(Game *game,int faction,int index,int room);
/* V57 playable layouts: game_init() is Kestrel A; game_init_ship() starts any of the
 * 18 base-game layouts with its original crew, systems, weapons, drones and augments.
 * game_ship_room() is the room a system code (player_ships.h) uses on this layout. */
#define GAME_START_CREW_MAX 4
int game_init_ship(Game *game,int layout);
int game_ship_room(const Game *game,int code);
int game_ship_start_level(const Game *game,int code);
int game_layout_count(void);
const char *game_layout_name(int layout);
const char *game_layout_class(int layout);
const char *game_layout_unlock(int layout);
const char *game_layout_pack(int layout);
int game_layout_supported(int layout);
typedef struct { int hull,reactor,missiles,drone_parts,crew_count,weapon_count,drone_count,augment_count,weapon_slots,drone_slots;
 int crew[GAME_START_CREW_MAX],weapons[4],drones[3],augments[3],systems; } GameLayoutInfo; /* systems: bit per player_ships.h code that starts installed. */
int game_layout_info(int layout,GameLayoutInfo *info);
/* V58 player artillery: charge time at the current power (ticks), power change, firing. */
int game_player_artillery_need(const Game *game);
int game_player_artillery_power(Game *game,int delta);
int game_launch_beam_piercing(Game *game,int side,int x0,int y0,int x1,int y1,const GameDamage *damage,int piercing);
/* V60 beacon hazards: asteroid fields (asteroids hit both ships), suns (solar flares
 * start fires on both ships), nebulas (sensors cannot see the enemy) and ion storms
 * (a nebula that also halves both reactors). Set at the start of a fight. */
enum GameEnvironment { ENV_NONE, ENV_ASTEROID, ENV_SUN, ENV_NEBULA, ENV_STORM };
void game_set_environment(Game *game,int environment);
/* V62 event status (type 0 limit, 1 divide, 2 loss, 3 clear) and its enforcement. */
void game_apply_status(Game *game,int type,int side,int system,int amount);
void game_clear_status(Game *game);
void game_enforce_status(Game *game,int side);
/* V62: spawn event boarders (species 0-6, 7 random) aboard the player ship. */
int game_spawn_intruders(Game *game,int count,int species);
int game_reactor(const Game *game);
int game_sensors_blinded(const Game *game);
int game_rename_crew(Game *game,int index,const char *name);
int game_save_stations(Game *game);
int game_return_stations(Game *game);
const char *game_crew_name(const Game *game,int index);
const char *game_crew_action(const Game *game,int index);
int game_next_room(const Game *game,int side,int source,int destination);
int game_crew_next_room_side(const Game *game,int faction,int index);
GameDoor *game_ship_doors(Game *game,int side,int *count);
/* Current enemy ownership is slots 0..11; carried home intruders are 12..59. */
int game_alive_enemy_crew(const Game *game);
/* Atomic promotion of living/pending-death current home boarders; 0 on failure. */
int game_carry_intruders(Game *game);
int game_away_crew(const Game *game);
int game_abandon_crew(Game *game);
void game_init(Game *game);
void game_service_step(Game *game);
/* Bounded enemy crew/power decisions; called once per simulated second. */
void game_enemy_crew_orders(Game *game);
/* Original CrewAI bCalmShip: no fire, breach, intruder or damaged system aboard the enemy. */
int game_enemy_calm(const Game *game);
/* V42 Rebel flagship (imported BOSS_N_NORMAL stages). */
#define GAME_FLAGSHIP_ROOMS 19
int game_is_flagship(const Game *game);
int game_enemy_cloaked(const Game *game);
int game_artillery_effective(const Game *game,int index);
int game_flagship_drones_active(const Game *game);
/* Seconds until a flagship power surge when within the 5 s warning, else 0. */
int game_surge_warning(const Game *game);
/* V44 flagship super shield points (0 when down or absent). */
int game_enemy_super_shield(const Game *game);
/* Original per-room crew counts (BossShip::crewCounts); counts[i] for room i. */
void game_flagship_crew_counts(const Game *game,int *counts);
int game_flagship_set_crew(Game *game,const int *counts);
int game_begin_encounter(Game *game,int kind,int sector,int phase);
/* V52: number of imported hulls; ENEMY_SHIP accepts indices 4..count-1 (0..3 have their own kinds). */
int game_ship_count(void);
const char *game_ship_blueprint(int index);
/* Pure departure preflight and atomic execution; failure leaves every byte intact. */
int game_can_depart(const Game *game,int abandon_away);
int game_depart(Game *game,int abandon_away);
/* Authored DS ceasefire: atomically exchange living boarding parties. */
int game_can_accept_surrender(const Game *game);
int game_accept_surrender(Game *game);
/* Encounter-local authored loadout; catalog1..4 and enemy-only Artemis7, no beam/bomb. Failure is
 * byte-identical. Configures weapons/ammo, never system capacity or reactor. */
int game_enemy_set_loadout(Game *game,const int *catalog_ids,int count,int missiles);
int game_enemy_weapon_slots(const Game *game);
const WeaponState *game_enemy_weapon_at(const Game *game,int slot);
int game_enemy_weapon_powered(const Game *game,int slot);
/* Original FTL 1.6.14 CombatAI normal-difficulty targeting (see docs/V38_WEAPON_AI.md).
 * Fills count distinct player room IDs (count is capped by room count); returns the count. */
int game_enemy_plan_volley(Game *game,int *rooms,int count);
/* Enemy beam line: start at one chosen room centre, aimed at a second, extended to length. */
int game_enemy_plan_beam(Game *game,int length,int *x0,int *y0,int *x1,int *y1);
int game_equip(Game *game,int slot,int catalog_id);
int game_toggle_weapon(Game *game,int slot);
int game_remove_weapon(Game *game,int slot);
int game_reorder_weapon(Game *game,int from,int to);
int game_set_weapon_target(Game *game,int slot,int room);
/* V69: like the original, each weapon keeps its own target; B clears it and the weapon
 * holds fire (autofire included) until a room is picked again. */
#define GAME_AIM_HOLD (-2)
int game_clear_weapon_target(Game *game,int slot);
/* The room a weapon is aimed at, or -1 when it holds fire. */
int game_weapon_aim(const Game *game,int slot);
int game_alive_crew(const Game *game);
/* V46 house rule: volleys start only while the weapons room is manned (side 0
 * player, 1 enemy; automated enemies count as manned). Charging is unaffected. */
int game_weapons_manned(const Game *game,int side);
int game_artillery_manned(const Game *game,int index);
/* 1 while a moving crewmember waits at a door (room full, sealed or locked). */
int game_crew_blocked(const Game *game,int faction,int index);
/* Effective sensors level: subsystem level plus one while a crewmember mans it. */
int game_sensor_level(const Game *game);
int game_station_bonus(const Game *game,int system);
/* V55 augments: ids follow generated/augment_catalog.h (stable order). */
enum GameAugment { AUG_ROCK_ARMOR, AUG_CRYSTAL_SHARDS, AUG_ENERGY_SHIELD, AUG_NANO_MEDBAY, AUG_SLUG_GEL, AUG_CREW_STIMS, AUG_DRONE_SPEED, AUG_SYSTEM_CASING,
    AUG_ION_ARMOR, AUG_CLOAK_FIRE, AUG_REPAIR_ARM, AUG_SCRAP_COLLECTOR, AUG_ADV_SCANNERS, AUG_AUTO_COOLDOWN, AUG_SHIELD_RECHARGE, AUG_WEAPON_PREIGNITE,
    AUG_FTL_BOOSTER, AUG_FTL_JUMPER, AUG_DRONE_RECOVERY, AUG_FTL_JAMMER, AUG_STASIS_POD, AUG_COUNT };
int game_augment_count(const Game *game,int id);
int game_add_augment(Game *game,int id);
int game_remove_augment(Game *game,int slot);
const char *game_augment_name(int id);
int game_augment_cost(int id);
int game_augment_rarity(int id);
/* V55 player cloaking: 5 s per effective bar, +60% evasion, enemy weapons stop
 * charging; ending applies 4 ion bars (20 s lockout) like the recovered enemy cloak. */
int game_cloak_install(Game *game);
int game_cloak_activate(Game *game);
int game_cloak_power(Game *game,int delta);
int game_player_cloaked(const Game *game);
int game_evasion(const Game *game);
const char *game_enemy_name(const Game *game);
int game_enemy_import_index(const Game *game);
/* Source geometry/art, including explicitly authored variants. */
int game_enemy_layout_index(const Game *game);
const char *game_weapon_name(int catalog_id);
/* Short DS label (at most 8 characters) and player availability (not enemy-only). */
const char *game_weapon_short(int catalog_id);
int game_weapon_player(int catalog_id);
int game_weapon_rarity(int catalog_id);
int game_weapon_price(int catalog_id);
#define GAME_ION_TICKS (5*GAME_HZ)
#define GAME_ION_MAX_BARS 8
#define GAME_ION_MAX_TICKS (40*GAME_HZ)
#define GAME_BEAM_UNITS 32
int game_set_beam_target(Game *game,int weapon,int x0,int y0,int x1,int y1);
int game_set_bomb_target(Game *game,int weapon,int side,int room);
int game_launch_bomb(Game *game,int side,int room,const GameDamage *damage);
int game_launch_beam(Game *game,int side,int x0,int y0,int x1,int y1,const GameDamage *damage);
int game_launch_ion(Game *game,int side,int room,int amount);
int game_launch_effect(Game *game,int side,int room,int kind,const GameDamage *damage,int piercing);
/* Encounter-local DS interpretation: latch at sampled absolute hull threshold,
 * then count operational ticks. Range: 1..3600*GAME_HZ ticks, 0..1000 hull.
 * Never enabled implicitly; duplicate configuration is rejected. */
int game_enemy_escape_start(Game *game,int delay_ticks,int hull_min,int hull_max);
int game_enemy_escape_ready(const Game *game);
int game_player_ftl_operational(const Game *game);
int game_player_ftl_ready(const Game *game);
void game_step(Game *game);
void game_toggle_pause(Game *game);
int game_toggle_door(Game *game,int index);
void game_close_all_doors(Game *game);
int game_order_crew(Game *game,int crew,int room);
int game_crew_next_room(const Game *game,int crew);
int game_crew_healing(const Game *game,int crew);
int game_adjust_power(Game *game,int system,int delta);
int game_fire(Game *game,int weapon,int target_room);
int game_toggle_autofire(Game *game,int weapon);
int game_weapon_powered(const Game *game,int weapon);
int game_used_power(const Game *game);
int game_shield_capacity(const Ship *ship);
int game_system_power(const Ship *ship,int system);
int game_order_group(Game *game,unsigned mask,int room);
const char *game_system_name(int system);
#endif
