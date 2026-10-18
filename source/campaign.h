#ifndef FTL_DS_CAMPAIGN_H
#define FTL_DS_CAMPAIGN_H
#include "game.h"
#include "rewards.h"
#define CAM_NODE_COUNT 24
#define CAM_SECTOR_COUNT 8
#define CAM_LEGACY_NODE_COUNT 12
#define CAM_LEGACY_SECTOR_COUNT 6
#define CAM_GENERATED_NODE_COUNT 23
#define CAM_CARGO_COUNT 4
/* V55: offer 15 installs cloaking, 16 sells one augment (save29 appends their stock). */
#define CAM_OFFER_COUNT 17
#define CAM_SAVED_OFFERS 15
#define CAM_QUESTS 4
enum { ACH_RUN_CREW_SHIPS, ACH_RUN_NEBULA, ACH_RUN_ROCK_SHIPS, ACH_RUN_CREW_CHOICES };
#define ACH_FLAG_WEAPONS 1u
#define ACH_FLAG_REACTOR 2u
#define ACH_FLAG_HAZARD 4u
#define CAM_BOSS_PATH_MAX 6
#define CAM_BASE_TURNS 4
/* V63: SECTOR_SECRET is the hidden Crystal homeworld, entered only through a
 * <secretSector/> event (never offered as a route; never the final sector). */
enum SectorType { SECTOR_UNCHARTED=0, SECTOR_CIVILIAN, SECTOR_ENGI, SECTOR_REBEL, SECTOR_PIRATE, SECTOR_FINAL, SECTOR_SECRET, SECTOR_TYPE_COUNT };
enum CampaignMode { CAM_MAP, CAM_COMBAT, CAM_EVENT, CAM_COMPLETE, CAM_DEFEAT, CAM_STORE };
enum CampaignNode { NODE_START, NODE_FIGHT, NODE_SUPPLY, NODE_DISTRESS, NODE_STORE, NODE_REPAIR, NODE_QUEST, NODE_HAZARD, NODE_EXIT };
enum CampaignEvent { EVENT_NONE, EVENT_SUPPLY, EVENT_DISTRESS, EVENT_REPAIR, EVENT_QUEST, EVENT_HAZARD, EVENT_EXIT, EVENT_AFTERMATH, EVENT_IMPORTED };
/* Authored, bounded demake campaign. All added campaign fields are integers;
 * Game weapon names must still be reconstructed rather than serialized. */
typedef struct {
    Game game;
    PendingReward reward; /* Resolve before further actions or safe-checkpoint save. */
    int mode,current,selected,fuel,scrap,jumps;
    int visited[CAM_NODE_COUNT];
    char text[2048];
    unsigned int seed,rng;
    int node_count,sector_count;
    int sector,sector_jumps,boss_phase,quest,event_id,event_resolved,pursued;
    int boss_resume_phase; /* -1 not begun;0..2 authored final phase to retry after retreat. */
    int result_ticks; /* Transient outcome presentation; rewards follow this gate once. */
    int links[CAM_NODE_COUNT],node_kind[CAM_NODE_COUNT],node_y[CAM_NODE_COUNT];
    int stock[CAM_NODE_COUNT][CAM_OFFER_COUNT];
    int cargo[CAM_CARGO_COUNT];
    int drone_cargo[CAM_CARGO_COUNT];
    /* V57: layouts unlocked during this run (bit = layout), merged into the SD
     * profile by the caller and then cleared. Never saved with the campaign. */
    unsigned unlock_pending;
    /* V59 run-long achievement progress (save30): ships won by killing the crew,
     * nebula beacons visited, Rock ships beaten, crew blue options taken; flags
     * for weapons/reactor upgrades and hazard beacons. */
    int ach_counter[4];
    unsigned ach_flags;
    /* V60: hazard at the current beacon (GameEnvironment), set by its event and
     * cleared on every jump; fights here start in it. Not saved. */
    int beacon_env;
    /* V61 quest beacons (save31): compiled event node per slot (-1 empty) and its
     * beacon in the current sector (-1 = it appears in the next sector). Quests
     * left behind when leaving a sector are lost, as in the original. */
    int quest_event[CAM_QUESTS],quest_node[CAM_QUESTS];
    /* V62 (save32): quest slots whose beacon is not marked (homeworld specials,
     * which look like ordinary beacons in the original) and the homeworld specials
     * already placed this run (bit per category-7 pool entry). */
    unsigned quest_hidden,specials_done;
    /* V62 transient event outcomes: boarders waiting for this event's fight
     * (count, species) and a store to open after the event. Not saved. */
    int pending_boarders,pending_boarder_species,store_after_event;
    /* V63: a <secretSector/> event jumps to the hidden Crystal sector when the
     * event is closed (campaign_continue). Transient like the fields above. */
    int secret_jump;
    int route_policy; /* 0 preserves pre-V24 generation; 1 enables sector branches. */
    int sector_types[CAM_SECTOR_COUNT];
    int next_sector[2];
    int event_policy; /* 0 authored, 1 six economy roots, 2 station roots, 3 escape scout, 4 weapon finds/civilian rescue, 5 generic Rebel. */
    int event_seen; /* Encountered compiled roots 0..30; unique roots never repeat. */
    unsigned event_seen_ext[8]; /* V53 (save28): roots 31..286. */
    int imported_ship; /* Transient loaded encounter descriptor, -1 when absent. */
    int imported_battle; /* 0 idle, 1 awaiting Engage, 2 combat, 3 outcome pending, 4 escape warning, 5 surrender dialogue. */
    int imported_surrender_threshold; /* -1 disabled/offered; otherwise once-only hull trigger. */
    int imported_escape_warned; /* Once-only presentation, including after refused surrender. */
    int imported_node; /* Transient compiled node, -1 outside an original event. */
    /* V41 original final sector (finale_policy 1; 0 keeps the interim exit battle).
     * StarMap::UpdateBoss state: route toward the Federation base (last generated
     * node), alternating move flag, temporary reversal and base countdown. */
    int finale_policy,boss_path_len,boss_loc,boss_move,boss_reversed,base_arrived,base_greeted;
    int boss_path[CAM_BOSS_PATH_MAX];
    unsigned fleet_nodes,fleet_pending; /* Rebel-controlled beacons and next jump's takeovers. */
    /* V42 BossShip::crewCounts: surviving flagship crew per room, reused by stages 2-3. */
    int boss_crew_valid,boss_crew[GAME_FLAGSHIP_ROOMS];
    /* V45 finale choice made at New Run: 0 DS-tuned flagship (default), 1 original normal numbers. */
    int flagship_original;
} Campaign;
int campaign_surrender_pending(const Campaign *campaign);
const char *campaign_text(const Campaign *campaign);
int campaign_choice_blue(const Campaign *campaign,int index);
void campaign_choice_hint(const Campaign *campaign,int index,char *buffer,int size);
int campaign_imported_begin(Campaign *campaign,int root);
/* V61: add a quest (compiled event node); 1 placed on the map, 2 next sector, 0 full. */
int campaign_quest_add(Campaign *campaign,int event);
int campaign_quest_at(const Campaign *campaign,int node);
int campaign_event_node_count(void);
unsigned campaign_event_catalog_hash(void);
int campaign_imported_count(void);
const char *campaign_imported_id(int root);
const char *campaign_sector_name(int type);
/* V51: catalog id sold by store offer n (4,5,6,8,9,10), else -1. */
int campaign_offer_weapon(const Campaign *c,int n);
/* V55: augment sold by this store (offer 16), and the scrap-gain rules of Scrap
 * Recovery Arm (+10% each) and Repair Arm (15% of the gain repairs 1 hull). */
int campaign_offer_augment(const Campaign *c);
/* V56: drone schematic sold as offer 14. */
int campaign_offer_drone(const Campaign *c);
/* V58: system sold as offer 15 (player_ships.h code; -1 none) and its price. */
int campaign_offer_system(const Campaign *c);
int campaign_system_price(int code);
void campaign_gain_scrap(Campaign *c,int amount);
const char *campaign_sector_summary(int type);
int campaign_route_count(const Campaign *campaign);
void campaign_init(Campaign *campaign);
void campaign_init_seed(Campaign *campaign,unsigned int seed);
void campaign_init_legacy(Campaign *campaign);
/* V57: start a run on a layout (0 Kestrel A); loader base for a layout. */
int campaign_init_ship(Campaign *campaign,unsigned int seed,int layout);
void campaign_init_legacy_layout(Campaign *campaign,int layout);
void campaign_init_legacy_seed(Campaign *campaign,unsigned int seed);
int campaign_node_captured(const Campaign *campaign,int index,int extra_jumps);
/* V41 final sector queries (all -1/0 outside an active base-defense sector). */
int campaign_finale_active(const Campaign *campaign);
int campaign_boss_node(const Campaign *campaign);
int campaign_base_node(const Campaign *campaign);
int campaign_boss_moves_next(const Campaign *campaign);
int campaign_base_turns_left(const Campaign *campaign); /* -1 until the flagship reaches the base. */
int campaign_node_rebel(const Campaign *campaign,int index);
int campaign_fleet_x(const Campaign *campaign,int extra_jumps);
int campaign_wait(Campaign *campaign);
int campaign_jump(Campaign *campaign,int index);
/* V71: queue progress unlocks the run has earned (Kestrel in sector 5+ -> Engi Cruiser). */
void campaign_check_unlocks(Campaign *campaign);
int campaign_can_retreat(const Campaign *campaign,int index);
int campaign_retreat(Campaign *campaign,int index,int abandon_away);
void campaign_continue(Campaign *campaign);
void campaign_step(Campaign *campaign);
/* Explicit arrival defense clock; never resolves the paused narrative. */
int campaign_can_service_home(const Campaign *campaign);
int campaign_service_home(Campaign *campaign);
int campaign_home_threat(const Campaign *campaign);
int campaign_can_jump(const Campaign *campaign,int index);
int campaign_linked(const Campaign *campaign,int from,int to);
int campaign_node_x(const Campaign *campaign,int index);
int campaign_node_y(const Campaign *campaign,int index);
const char *campaign_node_name(const Campaign *campaign,int index);
int campaign_choice_count(const Campaign *campaign);
const char *campaign_choice_name(const Campaign *campaign,int index);
int campaign_choice_enabled(const Campaign *campaign,int index);
int campaign_choice(Campaign *campaign,int index);
int campaign_offer_count(const Campaign *campaign);
const char *campaign_offer_name(const Campaign *campaign,int index);
int campaign_offer_price(const Campaign *campaign,int index);
int campaign_purchase(Campaign *campaign,int index);
int campaign_upgrade_price(const Campaign *campaign,int system); /* -1 reactor */
int campaign_upgrade(Campaign *campaign,int system);
int campaign_swap(Campaign *campaign,int cargo,int slot);
int campaign_rescue(Campaign *campaign);
int campaign_teleporter_price(const Campaign *campaign);
int campaign_install_teleporter(Campaign *campaign);
int campaign_upgrade_teleporter(Campaign *campaign);
int campaign_abandon_boarders(Campaign *campaign);
int campaign_subsystem_upgrade_price(const Campaign *campaign,int kind);
int campaign_upgrade_subsystem(Campaign *campaign,int kind);
/* V55: cloaking upgrades (levels 2-3), -1 when absent or at maximum. */
int campaign_cloak_upgrade_price(const Campaign *campaign);
int campaign_upgrade_cloak(Campaign *campaign);
/* V58: Artillery Beam upgrades (levels 2-4: 30/50/80 scrap), -1 when absent or maximum. */
int campaign_artillery_upgrade_price(const Campaign *campaign);
int campaign_upgrade_artillery(Campaign *campaign);
int campaign_species_offer(const Campaign *campaign);
int campaign_dismiss_crew(Campaign *campaign,int index);
int campaign_unequip(Campaign *campaign,int slot);
int campaign_reorder_weapon(Campaign *campaign,int from,int to);
int campaign_sell_weapon(Campaign *campaign,int installed,int index);
int campaign_weapon_sell_price(const Campaign *campaign,int installed,int index);
int campaign_drone_upgrade_price(const Campaign *campaign);
int campaign_upgrade_drones(Campaign *campaign);
int campaign_drone_swap(Campaign *campaign,int cargo,int slot);
int campaign_drone_unequip(Campaign *campaign,int slot);
int campaign_drone_reorder(Campaign *campaign,int from,int to);
int campaign_drone_sell_price(const Campaign *campaign,int installed,int index);
int campaign_sell_drone(Campaign *campaign,int installed,int index);
#endif
