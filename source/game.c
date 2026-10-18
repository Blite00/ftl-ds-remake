#include "game.h"
#include "drones.h"
#include "ftl_data.h"
#include "enemy_data.h"
#include "enemy_catalog.h"
#include "fire.h"
#include "boarding.h"
#include "breach.h"
#include "subsystems.h"
#include "species.h"
#include "beam.h"
#include "flagship_data.h"
#include "weapon_properties.h"
#include "weapon_catalog.h"
#include "enemy_weapon_variants.h"
#include "augment_catalog.h"
#include "player_ships.h"
#include "achievements.h"
#include <string.h>
#include <stdio.h>

static int minimum(int a,int b) { return a<b?a:b; }
static void say(Game *g,const char *s) { snprintf(g->message,sizeof(g->message),"%s",s); }
static void flagship_setup(Game*g,int stage);
const char *game_system_name(int s) {
    static const char *names[]={"Shields","Engines","Weapons","Oxygen","Medbay"};
    return s>=0 && s<GAME_SYSTEM_COUNT?names[s]:"Room";
}
static int identify_system(const char *name) {
    if (!strcmp(name,"shields")) return SYS_SHIELDS;
    if (!strcmp(name,"engines")) return SYS_ENGINES;
    if (!strcmp(name,"weapons")) return SYS_WEAPONS;
    if (!strcmp(name,"oxygen")) return SYS_OXYGEN;
    if (!strcmp(name,"medbay")) return SYS_MEDBAY;
    return -1;
}
static int room_index(const Ship *s,int id) {
    int i;
    for(i=0;i<s->room_count;i++) if(s->rooms[i].id==id) return i;
    return -1;
}
static int effective(const Ship *s,int system) {
    const GameSystemState*d=&s->systems[system];int manual=d->power-d->ion_bars;if(manual<0)manual=0;return minimum(d->health,manual+d->bonus_power);
}
int game_system_power(const Ship*s,int system){return system>=0&&system<GAME_SYSTEM_COUNT?effective(s,system):0;}
int game_shield_capacity(const Ship *ship) { return effective(ship,SYS_SHIELDS)/2; }
typedef char augment_catalog_matches[IMPORTED_AUGMENT_COUNT==AUG_COUNT?1:-1];
int game_augment_count(const Game*g,int id){int n=0;for(int i=0;i<GAME_MAX_AUGMENTS;i++)n+=g->augments[i]==id;return n;}
const char*game_augment_name(int id){return id>=0&&id<AUG_COUNT?imported_augments[id].name:"";}
int game_augment_cost(int id){return id>=0&&id<AUG_COUNT?imported_augments[id].cost:0;}
int game_augment_rarity(int id){return id>=0&&id<AUG_COUNT?imported_augments[id].rarity:0;}
int game_add_augment(Game*g,int id){
    if(id<0||id>=AUG_COUNT||(!imported_augments[id].stackable&&game_augment_count(g,id)))return -1;
    for(int i=0;i<GAME_MAX_AUGMENTS;i++)if(g->augments[i]<0){g->augments[i]=id;return i;}
    return -1;
}
int game_remove_augment(Game*g,int slot){if(slot<0||slot>=GAME_MAX_AUGMENTS||g->augments[slot]<0)return 0;g->augments[slot]=-1;return 1;}
static int aug_roll(Game*g,int id){int n=game_augment_count(g,id);if(!n)return 0;g->rng=g->rng*1664525u+1013904223u;return (int)((g->rng>>16)%100u)<imported_augments[id].value*n;}
static void augment_upkeep(Game*g);
int game_player_cloaked(const Game*g){return g->player_cloak.capacity>0&&g->player_cloak.active_ticks>0;}
int game_cloak_install(Game*g){int i;
    if(g->player_cloak.capacity>0){say(g,"Cloaking already installed.");return 0;}
    i=game_ship_room(g,10);if(i>=0){EnemyCloak*k=&g->player_cloak;memset(k,0,sizeof *k);k->room=i;k->capacity=k->health=1;say(g,"Cloaking installed. Allocate power to use it.");return 1;}
    say(g,"This hull has no cloaking room.");return 0;
}
int game_cloak_power(Game*g,int delta){EnemyCloak*k=&g->player_cloak;
    if(k->capacity<=0){say(g,"No cloaking system.");return 0;}
    if(delta<0){if(k->power<=0)return 0;k->power--;if(k->active_ticks&&k->power<=k->ion_bars)k->active_ticks=0;say(g,"Cloak power reduced.");return 1;}
    if(k->power>=k->health){say(g,"Cloak at maximum power.");return 0;}
    if(game_used_power(g)>=game_reactor(g)){say(g,"No spare reactor power.");return 0;}
    k->power++;say(g,"Cloak power added.");return 1;
}
int game_cloak_activate(Game*g){EnemyCloak*k=&g->player_cloak;int p=minimum(k->power,k->health)-k->ion_bars;
    if(k->capacity<=0){say(g,"No cloaking system.");return 0;}
    if(g->outcome!=GAME_PLAYING){say(g,"Cloak works only in combat.");return 0;}
    if(k->active_ticks>0){say(g,"Already cloaked.");return 0;}
    if(k->ion_bars>0){say(g,"Cloak recharging.");return 0;}
    if(p<=0){say(g,"Cloak needs power.");return 0;}
    k->active_ticks=5*GAME_HZ*p;g->ach.cloak_full=g->enemy.hull>=g->enemy.max_hull;g->ach.cloak_avoided=0;say(g,"Cloaked: +60% evasion, enemies paused.");return 1;
}
int game_used_power(const Game *g) {
    int n=g->teleporter[0].power+g->drones.power+g->player_cloak.power+g->player_artillery.power,i;
    for(i=0;i<GAME_SYSTEM_COUNT;i++) n+=g->player.systems[i].power;
    return n;
}
/* Catalog entries 0/1 use imported stats; added lasers are demake choices. */
typedef char weapon_catalog_matches[IMPORTED_WEAPON_COUNT==GAME_CATALOG_COUNT?1:-1];
const char *game_weapon_name(int id) {
    static const char *names[]={"Artemis", "Burst Laser II", "Basic Laser", "Heavy Laser", "Ion Blast I", "Halberd Beam", "Small Bomb"};
    if(id>=0&&id<7)return names[id];
    return id>=0 && id<GAME_CATALOG_COUNT?imported_weapon_catalog[id].name:"Empty";
}
const char *game_weapon_short(int id){return id>=0&&id<GAME_CATALOG_COUNT?imported_weapon_catalog[id].short_name:"";}
int game_weapon_player(int id){return id>=0&&id<GAME_CATALOG_COUNT&&!imported_weapon_catalog[id].enemy_only;}
int game_weapon_rarity(int id){return id>=0&&id<GAME_CATALOG_COUNT?imported_weapon_catalog[id].rarity:0;}
/* Historical DS prices for 0..6; imported weapons use the original store cost. */
int game_weapon_price(int id) {static const int prices[]={38,65,28,48,30,65,45};if(id>=0&&id<7)return prices[id];return game_weapon_player(id)?imported_weapon_catalog[id].cost:0;}
static void weapon_from_catalog(WeaponState*w,int id){
    const ImportedWeapon*d=&imported_weapon_catalog[id];const ImportedWeaponProperties*p=&imported_weapon_properties[id];
    w->cooldown_ticks=d->cooldown_ms*GAME_HZ/1000;w->power=d->power;
    w->damage=d->damage;w->shots=d->shots;w->missiles=d->missiles;w->shield_piercing=d->shield_piercing;w->ion_damage=d->ion;w->bomb=!strcmp(d->type,"BOMB");w->beam_length=(d->beam_length*GAME_BEAM_UNITS+17)/35;
    w->system_damage=p->has_system_damage?p->system_damage:w->damage;
    w->crew_damage=p->has_personnel_damage?p->personnel_damage*15:w->damage*15;
    w->fire_chance=p->fire_chance*10;w->breach_chance=p->breach_chance*10;if(w->fire_chance>100)w->fire_chance=100;if(w->breach_chance>100)w->breach_chance=100;
    w->hull_bust=d->hull_bust;w->lockdown=d->lockdown;
}
static GameDamage weapon_damage(const WeaponState*w){return (GameDamage){w->damage,w->system_damage,w->crew_damage,w->fire_chance,w->breach_chance,w->hull_bust,w->lockdown,w->bomb?w->ion_damage:0};}
int game_equip(Game *g,int slot,int id) {
    WeaponState *w;
    if(slot<0 || slot>=GAME_MAX_WEAPONS || slot>g->weapon_count || !game_weapon_player(id))return 0;
    w=&g->weapons[slot];memset(w,0,sizeof(*w));
    w->catalog_id=id;w->name=game_weapon_name(id);w->enabled=1;w->aim_room=-1;
    weapon_from_catalog(w,id);
    g->autofire[slot]=0;if(slot==g->weapon_count)g->weapon_count++;
    return 1;
}
int game_enemy_weapon_slots(const Game*g){return g?(g->enemy_weapon_count?g->enemy_weapon_count:1):0;}
const WeaponState*game_enemy_weapon_at(const Game*g,int slot){if(!g||slot<0||slot>=game_enemy_weapon_slots(g))return NULL;return slot?&g->enemy_weapon_secondary:&g->enemy_weapon;}
int game_enemy_weapon_powered(const Game*g,int slot){
    const WeaponState*w=game_enemy_weapon_at(g,slot);int available;
    if(!w||g->enemy.hull<=0||g->enemy_departed)return 0;
    available=effective(&g->enemy,SYS_WEAPONS);
    if(!g->enemy_weapon_count)return available>=w->power;
    for(int i=0;i<game_enemy_weapon_slots(g);i++){const WeaponState*a=game_enemy_weapon_at(g,i);int on=a->enabled&&a->power<=available;if(i==slot)return on;if(on)available-=a->power;}
    return 0;
}
int game_enemy_set_loadout(Game*g,const int*ids,int count,int missiles){
    WeaponState staged[GAME_MAX_ENEMY_WEAPONS];int power=0;
    if(!g||!ids||count<1||count>GAME_MAX_ENEMY_WEAPONS||missiles<0||missiles>1000000||g->outcome||g->enemy.hull<=0||g->enemy_departed)return 0;
    memset(staged,0,sizeof staged);
    for(int i=0;i<count;i++){
        int id=ids[i];if(id<0||id>=GAME_CATALOG_COUNT)return 0;
        WeaponState*w=&staged[i];w->catalog_id=id;w->enabled=1;w->aim_room=-1;w->target_room=-1;
        if(id==GAME_ENEMY_ARTEMIS){
            const ImportedEnemyWeaponVariant*d=&imported_enemy_artemis;
            w->name=d->name;w->cooldown_ticks=d->cooldown_ms*GAME_HZ/1000;w->power=d->power;w->damage=d->damage;w->shots=d->shots;w->missiles=d->missiles;w->shield_piercing=d->shield_piercing;w->ion_damage=d->ion;
            w->system_damage=d->has_system_damage?d->system_damage:d->damage;w->crew_damage=d->has_personnel_damage?d->personnel_damage*15:d->damage*15;w->fire_chance=d->fire_chance*10;w->breach_chance=d->breach_chance*10;
            /* Raw imported stun metadata is retained by the importer. AE stun
             * remains outside this base-game implementation. */
        }else{w->name=game_weapon_name(id);weapon_from_catalog(w,id);}
        power+=w->power;
    }
    if(power>g->enemy.systems[SYS_WEAPONS].capacity)return 0;
    /* Configuration is allowed only between volleys, never erase in-flight shots. */
    if(g->enemy_pending_shots||g->enemy_weapon.pending_shots||g->enemy_weapon_secondary.pending_shots)return 0;
    for(int i=0;i<GAME_MAX_EFFECTS;i++)if(g->effects[i].active&&g->effects[i].side==1)return 0;
    g->enemy_weapon=staged[0];g->enemy_weapon_secondary=staged[1];g->enemy_weapon_count=count;g->enemy_missiles=missiles;
    g->enemy_fire_ticks=g->enemy_pending_shots=g->enemy_shot_ticks=0;return 1;
}
static void enemy_loadout_stop(Game*g){
    if(!g->enemy_weapon_count)return;
    g->enemy_weapon.charge_ticks=g->enemy_weapon.pending_shots=g->enemy_weapon.shot_ticks=0;
    g->enemy_weapon_secondary.charge_ticks=g->enemy_weapon_secondary.pending_shots=g->enemy_weapon_secondary.shot_ticks=0;
    g->enemy_fire_ticks=g->enemy_pending_shots=g->enemy_shot_ticks=0;
}
/* Equipment moves preserve the weapon identity, including queued attacks.
   Campaign management decides when removing an equipped weapon is allowed. */
int game_remove_weapon(Game*g,int slot){int i;
    if(slot<0||slot>=g->weapon_count)return 0;
    for(i=slot;i+1<g->weapon_count;i++){g->weapons[i]=g->weapons[i+1];g->autofire[i]=g->autofire[i+1];}
    g->weapon_count--;memset(&g->weapons[g->weapon_count],0,sizeof g->weapons[0]);g->autofire[g->weapon_count]=0;return 1;
}
int game_reorder_weapon(Game*g,int from,int to){WeaponState moved;int i,autofire;
    if(from<0||from>=g->weapon_count||to<0||to>=g->weapon_count)return 0;
    if(from==to)return 1;
    moved=g->weapons[from];autofire=g->autofire[from];
    if(from<to)for(i=from;i<to;i++){g->weapons[i]=g->weapons[i+1];g->autofire[i]=g->autofire[i+1];}
    else for(i=from;i>to;i--){g->weapons[i]=g->weapons[i-1];g->autofire[i]=g->autofire[i-1];}
    g->weapons[to]=moved;g->autofire[to]=autofire;return 1;
}
int game_toggle_weapon(Game *g,int slot) {
    if(g->outcome==GAME_DEFEAT || slot<0 || slot>=g->weapon_count)return 0;
    g->weapons[slot].enabled=!g->weapons[slot].enabled;
    if(!g->weapons[slot].enabled)g->weapons[slot].charge_ticks=0;
    say(g,g->weapons[slot].enabled?"Weapon enabled.":"Weapon disabled.");return 1;
}
int game_set_weapon_target(Game *g,int slot,int room) {
    if(slot<0 || slot>=g->weapon_count || (room!=-1 && room_index(g->weapons[slot].bomb&&g->weapons[slot].bomb_side?&g->player:&g->enemy,room)<0))return 0;
    g->weapons[slot].aim_room=room;return 1;
}
int game_clear_weapon_target(Game*g,int slot){
    if(slot<0||slot>=g->weapon_count)return 0;
    g->weapons[slot].aim_room=GAME_AIM_HOLD;g->weapons[slot].beam_target_valid=0;return 1;
}
int game_weapon_aim(const Game*g,int slot){
    if(slot<0||slot>=g->weapon_count)return -1;
    return g->weapons[slot].aim_room>=0?g->weapons[slot].aim_room:g->weapons[slot].aim_room==GAME_AIM_HOLD?-1:g->target_room;
}
int game_set_bomb_target(Game*g,int weapon,int side,int room){
    if(weapon<0||weapon>=g->weapon_count||!g->weapons[weapon].bomb||side<0||side>1||room_index(side?&g->player:&g->enemy,room)<0)return 0;
    g->weapons[weapon].bomb_side=side;g->weapons[weapon].aim_room=room;return 1;
}
int game_weapon_powered(const Game *g,int weapon) {
    int available=effective(&g->player,SYS_WEAPONS),i;
    /* Display order is power priority; disabled/unaffordable slots consume no bars. */
    if(weapon<0 || weapon>=g->weapon_count || !g->weapons[weapon].enabled)return 0;
    for(i=0;i<g->weapon_count;i++) {
        int slot=i;
        if(!g->weapons[slot].enabled)continue;
        if(available<g->weapons[slot].power) {if(slot==weapon)return 0;continue;}
        available-=g->weapons[slot].power;if(slot==weapon)return 1;
    }
    return 0;
}
int game_alive_crew(const Game *g) {int i,n=0;for(i=0;i<g->crew_count;i++)n+=g->crew[i].health>0;return n;}
static int station_crew(const Game *g,int room) {
    int i;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0 && g->crew[i].ship==0 && g->crew[i].room==room && g->crew[i].destination==room)return i;
    return -1;
}
int game_station_bonus(const Game *g,int system) {
    int room=system<0?g->pilot_room:(system<GAME_SYSTEM_COUNT?g->player.systems[system].room:-1);
    int i=station_crew(g,room),skill=system==SYS_ENGINES?1:system==SYS_WEAPONS?2:system==SYS_SHIELDS?3:0;
    return i<0?0:5+minimum(10,g->crew[i].skill[skill]/(60*GAME_HZ)*5);
}
int game_ship_count(void){return IMPORTED_ENEMY_COUNT;}
const char *game_ship_blueprint(int i){return i>=0&&i<IMPORTED_ENEMY_COUNT?imported_enemies[i].blueprint:"";}
int game_enemy_import_index(const Game *g) {if(g->enemy_kind==ENEMY_SHIP)return g->enemy_ship>=4&&g->enemy_ship<IMPORTED_ENEMY_COUNT?g->enemy_ship:-1;return g->enemy_kind==ENEMY_SCOUT?0:g->enemy_kind==ENEMY_ASSAULT?1:(g->enemy_kind==ENEMY_REBEL||g->enemy_kind==ENEMY_REBEL_BOARDER )?2:g->enemy_kind==ENEMY_REBEL_FAT?3:-1;}
int game_enemy_layout_index(const Game *g) {
    int imported=game_enemy_import_index(g);
    return imported>=0?imported:g->enemy_kind==ENEMY_INTERCEPTOR?0:g->enemy_kind==ENEMY_ARMORED?1:(g->enemy_kind==ENEMY_FLAGSHIP&&g->flagship_stage<0)?3:-1;
}
int game_evasion(const Game *g) {
    if(g->pilot_health<=0 || g->pilot_ion_ticks[0]>0 || station_crew(g,g->pilot_room)<0)return 0;
    if(effective(&g->player,SYS_ENGINES)<=0)return 0;
    return minimum(45,effective(&g->player,SYS_ENGINES)*5+game_station_bonus(g,-1)+game_station_bonus(g,SYS_ENGINES));
}
void game_init(Game *g) {game_init_ship(g,0);}
int game_layout_count(void){return PLAYER_SHIP_COUNT;}
const char*game_layout_name(int k){return k>=0&&k<PLAYER_SHIP_COUNT?player_ships[k].name:"";}
const char*game_layout_class(int k){return k>=0&&k<PLAYER_SHIP_COUNT?player_ships[k].klass:"";}
const char*game_layout_pack(int k){return k>=0&&k<PLAYER_SHIP_COUNT?player_ships[k].pack:"";}
const char*game_layout_unlock(int k){return k>=0&&k<PLAYER_SHIP_COUNT?player_ships[k].unlock:"";}
int game_layout_supported(int k){return k>=0&&k<PLAYER_SHIP_COUNT;}
int game_layout_info(int k,GameLayoutInfo*o){const PlayerShipData*d;int i;
    if(k<0||k>=PLAYER_SHIP_COUNT||!o)return 0;
    d=&player_ships[k];memset(o,0,sizeof *o);
    o->hull=d->hull;o->reactor=d->reactor;o->missiles=d->missiles;o->drone_parts=d->drone_parts;o->weapon_slots=d->weapon_slots;o->drone_slots=d->drone_slots;
    o->crew_count=minimum(d->crew_count,GAME_START_CREW_MAX);o->weapon_count=d->weapon_count;o->drone_count=d->drone_count;o->augment_count=d->augment_count;
    for(i=0;i<o->crew_count;i++){o->crew[i]=d->crew[i];}
    for(i=0;i<4;i++){o->weapons[i]=d->weapons[i];}
    for(i=0;i<3;i++){o->drones[i]=d->drones[i];o->augments[i]=d->augments[i];}
    for(i=0;i<d->system_count;i++){if(d->systems[i].start)o->systems|=1<<d->systems[i].code;}
    return 1;}
int game_ship_room(const Game*g,int code){const PlayerShipData*d;int i;if(g->player_ship<0||g->player_ship>=PLAYER_SHIP_COUNT)return -1;d=&player_ships[g->player_ship];
    for(i=0;i<d->system_count;i++){if(d->systems[i].code==code)return d->systems[i].room;}
    return -1;}
int game_ship_start_level(const Game*g,int code){const PlayerShipData*d;int i;if(g->player_ship<0||g->player_ship>=PLAYER_SHIP_COUNT)return 0;d=&player_ships[g->player_ship];
    for(i=0;i<d->system_count;i++){if(d->systems[i].code==code&&d->systems[i].start)return d->systems[i].power;}
    return 0;}
int game_init_ship(Game *g,int layout) {
    int i,j;const PlayerShipData*d;
    if(layout<0||layout>=PLAYER_SHIP_COUNT)return 0;
    d=&player_ships[layout];
    memset(g,0,sizeof(*g));g->player_ship=layout;game_clear_status(g);g->weapon_slots=minimum(d->weapon_slots,GAME_MAX_WEAPONS);
    for(i=0;i<GAME_MAX_AUGMENTS;i++)g->augments[i]=-1;
    drones_init(g);
    g->rng=0x46544c44u;subsystems_init(g,0);
    g->crew_count=minimum(d->crew_count,GAME_START_CREW_MAX);g->pilot_room=game_ship_room(g,5);g->pilot_health=1;
    g->player.hull=g->player.max_hull=d->hull;
    g->player.reactor=d->reactor;
    g->player.oxygen=1000;
    g->player.room_count=minimum(d->room_count,GAME_MAX_ROOMS);
    g->missiles=d->missiles;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)g->player.systems[i].room=-1;
    for(i=0;i<g->player.room_count;i++) {
        GameRoom *r=&g->player.rooms[i];
        r->id=d->rooms[i].id;r->x=d->rooms[i].x;r->y=d->rooms[i].y;
        r->w=d->rooms[i].w;r->h=d->rooms[i].h;r->system=-1;r->oxygen=1000;
    }
    for(i=0;i<d->system_count;i++) {
        const PlayerShipSystem *data=&d->systems[i];
        int s=data->code<GAME_SYSTEM_COUNT?data->code:-1;
        if(s<0 || !data->start) continue;
        g->player.systems[s].room=data->room;
        g->player.systems[s].capacity=data->power;
        g->player.systems[s].health=data->power;
        g->player.systems[s].power=s==SYS_MEDBAY?0:data->power;
        j=room_index(&g->player,data->room);
        if(j>=0) g->player.rooms[j].system=s;
    }
    g->door_count=minimum(d->door_count,GAME_MAX_DOORS);
    for(i=0;i<g->door_count;i++) {
        GameDoor *door=&g->doors[i];
        door->x=d->doors[i].x;door->y=d->doors[i].y;
        door->room_a=d->doors[i].room_a;door->room_b=d->doors[i].room_b;
        door->vertical=d->doors[i].vertical;
        int a=room_index(&g->player,d->doors[i].room_a);
        int b=room_index(&g->player,d->doors[i].room_b);
        if(a>=0 && b>=0) g->adjacency[a][b]=g->adjacency[b][a]=1;
    }
    /* Optional starting systems: teleporter, drone control, cloaking. */
    for(i=0;i<d->system_count;i++){const PlayerShipSystem*data=&d->systems[i];if(!data->start)continue;
        if(data->code==8&&boarding_install(g,0)){g->teleporter[0].capacity=g->teleporter[0].health=data->power;}
        else if(data->code==9&&drones_install(g)){g->drones.capacity=g->drones.health=data->power;g->drones.power=minimum(data->power,2);}
        else if(data->code==10&&game_cloak_install(g)){g->player_cloak.capacity=g->player_cloak.health=data->power;g->player_cloak.power=data->power;}
        else if(data->code==11){Artillery*a=&g->player_artillery;memset(a,0,sizeof *a);a->room=data->room;a->capacity=a->health=a->power=data->power;a->weapon=GAME_WEAPON_ARTILLERY_FED;weapon_from_catalog(&a->w,GAME_WEAPON_ARTILLERY_FED);}}
    while(game_used_power(g)>g->player.reactor) {
        int cut=0;
        for(i=0;i<GAME_SYSTEM_COUNT&&!cut;i++) {
            if(g->player.systems[i].power>0) {g->player.systems[i].power--;cut=1;}
        }
        if(!cut&&g->drones.power>0){g->drones.power--;cut=1;}
        if(!cut&&g->player_cloak.power>0){g->player_cloak.power--;cut=1;}
        if(!cut&&g->teleporter[0].power>0){g->teleporter[0].power--;cut=1;}
        if(!cut&&g->player_artillery.power>0){g->player_artillery.power--;cut=1;}
        if(!cut)break;
    }
    g->player.shields=effective(&g->player,SYS_SHIELDS)/2;
    for(i=0;i<d->weapon_count&&i<g->weapon_slots;i++)game_equip(g,i,d->weapons[i]);
    g->drones.slot_count=minimum(d->drone_slots,GAME_MAX_DRONES);
    for(i=0;i<d->drone_count&&i<g->drones.slot_count;i++)if(g->drones.installed)drones_equip(g,i,d->drones[i]);
    g->drone_parts=d->drone_parts;
    for(i=0;i<d->augment_count;i++)game_add_augment(g,d->augments[i]);
    {static const int order[]={SYS_ENGINES,SYS_WEAPONS,SYS_SHIELDS,SYS_OXYGEN,SYS_MEDBAY}; /* Kestrel order: pilot, engines, weapons. */int k=0;
    for(i=0;i<g->crew_count;i++) {
        int room=g->pilot_room;
        /* Two crew: the second mans the weapons (weapons need a gunner) when the ship has any. */
        if(i==1&&g->crew_count==2&&g->weapon_count>0&&g->player.systems[SYS_WEAPONS].room>=0)room=g->player.systems[SYS_WEAPONS].room;
        else if(i>0){room=-1;while(k<5&&room<0){room=g->player.systems[order[k]].room;k++;}if(room<0)room=g->player.rooms[i%g->player.room_count].id;}
        game_init_crew(g,0,i,room);
        game_set_crew_species(g,0,i,d->crew[i]>=0&&d->crew[i]<SPEC_COUNT?d->crew[i]:SPEC_HUMAN);
    }}
    game_begin_encounter(g,ENEMY_SCOUT,0,0);
    say(g,"Select crew, then a room. Charge weapons and fire.");
    return 1;
}
const char *game_enemy_name(const Game *g) {
    static const char *names[]={"Auto Scout","Interceptor","Armored Drone","Rebel Command","Auto-Assault","Rebel Fighter","Rebel Boarder","Rebel Rigger"};
    if(g->enemy_kind==ENEMY_SHIP&&g->enemy_ship>=0&&g->enemy_ship<IMPORTED_ENEMY_COUNT)return imported_enemies[g->enemy_ship].name;
    if(game_is_flagship(g))return "Rebel Flagship";
    return names[g->enemy_kind>=0 && g->enemy_kind<ENEMY_KIND_COUNT?g->enemy_kind:0];
}
/* V52 generic original enemy ship: imported rooms, doors and starting systems
 * (levels grow by one every two sectors up to the blueprint maximum), crew of
 * the blueprint's species, and two weapons from its original weapon list that
 * fit the weapon system (rarer weapons allowed later). Authored scaling. */
static int species_code(const char*s){return !strcmp(s,"engi")?SPEC_ENGI:!strcmp(s,"mantis")?SPEC_MANTIS:!strcmp(s,"rock")?SPEC_ROCK:!strcmp(s,"slug")?SPEC_SLUG:!strcmp(s,"energy")?SPEC_ZOLTAN:!strcmp(s,"crystal")?SPEC_CRYSTAL:SPEC_HUMAN;}
static unsigned ship_rng(Game*g){g->rng=g->rng*1664525u+1013904223u;return g->rng>>8;}
static void ship_weapons(Game*g,const ImportedEnemy*d,int sector,int capacity){
    int best[2]={-1,-1},count=0;
    for(int attempt=0;attempt<32&&d->weapon_count>0;attempt++){
        int pick[2],n=capacity>=3?2:1,power=0,ok=1;
        for(int k=0;k<n;k++){pick[k]=d->weapon_ids[ship_rng(g)%(unsigned)d->weapon_count];
            int id=pick[k],rar=id>=0&&id<GAME_CATALOG_COUNT?imported_weapon_catalog[id].rarity:9;
            if(id<0||id>=GAME_CATALOG_COUNT||id==31||rar>1+sector/3)ok=0;else power+=imported_weapon_catalog[id].power;}
        if(ok&&power<=capacity){best[0]=pick[0];best[1]=n>1?pick[1]:-1;count=n;break;}
    }
    if(!count){best[0]=2;count=1;if(capacity<1){g->enemy.systems[SYS_WEAPONS].capacity=g->enemy.systems[SYS_WEAPONS].health=g->enemy.systems[SYS_WEAPONS].power=1;}}
    if(!game_enemy_set_loadout(g,best,count,d->missiles+sector)){int basic=2;game_enemy_set_loadout(g,&basic,1,d->missiles);}
}
static void flagship_weapon(WeaponState*w,int index);
static void ship_setup(Game*g,int sector){
    const ImportedEnemy*d=&imported_enemies[g->enemy_ship];int i,used=0;
    g->enemy.hull=g->enemy.max_hull=d->hull+sector;g->enemy.automated=d->automated;g->enemy_missiles=d->missiles;
    g->enemy.room_count=minimum(d->room_count,GAME_MAX_ROOMS);g->enemy.oxygen=d->automated?0:1000;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)g->enemy.systems[i].room=-1;
    for(i=0;i<g->enemy.room_count;i++){const ImportedEnemyRoom*src=&d->rooms[i];GameRoom*r=&g->enemy.rooms[i];r->id=src->id;r->x=src->x;r->y=src->y;r->w=src->w;r->h=src->h;r->system=-1;r->oxygen=g->enemy.oxygen;}
    for(i=0;i<d->system_count;i++){const ImportedEnemySystem*src=&d->systems[i];int level=src->power+sector/3,system=identify_system(src->name);
        /* Optional systems (start="false": cloaking, teleporters, drones, medbays...)
         * appear more often in later sectors, like the original's level-based fitting. */
        if(!src->start&&(int)(ship_rng(g)%100u)>=sector*12)continue;
        if(level>src->maximum)level=src->maximum;
        if(level<1)level=1;
        if(!strcmp(src->name,"pilot")){g->enemy_pilot_room=src->room;g->enemy_pilot_health=1;continue;}
        if(system>=0){GameSystemState*y=&g->enemy.systems[system];y->room=src->room;y->capacity=y->health=y->power=level;used+=level;int j=room_index(&g->enemy,src->room);if(j>=0)g->enemy.rooms[j].system=system;continue;}
        if(!strcmp(src->name,"doors")||!strcmp(src->name,"sensors")){Subsystem*y=&g->subsystems[1][!strcmp(src->name,"doors")?SUB_DOORS:SUB_SENSORS];y->room=src->room;y->capacity=y->health=level;continue;}
        if(!strcmp(src->name,"drones")){EnemyDrone*y=&g->enemy_drone;y->room=src->room;y->capacity=y->health=y->power=level<2?2:level;y->cooldown_ticks=10*GAME_HZ;y->damage=imported_combat_drone.damage;y->fire_chance=imported_combat_drone.fire_chance*10;y->shots=imported_combat_drone.shots;used+=y->power;continue;}
        if(!strcmp(src->name,"teleporter")){Teleporter*t=&g->teleporter[1];memset(t,0,sizeof *t);t->installed=1;t->room=src->room;t->capacity=t->health=t->power=level;used+=level;continue;}
        if(!strcmp(src->name,"cloaking")){EnemyCloak*k=&g->enemy_cloak;memset(k,0,sizeof *k);k->room=src->room;k->capacity=k->health=k->power=level;used+=level;continue;}
        /* V67: the half-built flagship (BOSS_SPECIAL) mounts ARTILLERY_BOSS_1 and _2 at power 1, as in its blueprint. */
        if(!strcmp(src->name,"artillery")&&g->artillery_count<GAME_MAX_ARTILLERY&&g->artillery_count<FLAGSHIP_WEAPON_COUNT){Artillery*a=&g->artillery[g->artillery_count];memset(a,0,sizeof *a);
            a->room=src->room;a->capacity=a->health=a->power=src->power;a->weapon=g->artillery_count;flagship_weapon(&a->w,a->weapon);g->artillery_count++;used+=src->power;continue;}
    }
    g->enemy.reactor=d->reactor>used?d->reactor:used;
    g->enemy_door_count=minimum(d->door_count,GAME_MAX_DOORS);
    for(i=0;i<g->enemy_door_count;i++){const ImportedEnemyDoor*src=&d->doors[i];GameDoor*o=&g->enemy_doors[i];int a,b;memset(o,0,sizeof *o);o->x=src->x;o->y=src->y;o->room_a=src->room_a;o->room_b=src->room_b;o->vertical=src->vertical;
        a=room_index(&g->enemy,o->room_a);b=room_index(&g->enemy,o->room_b);if(a>=0&&b>=0)g->enemy_adjacency[a][b]=g->enemy_adjacency[b][a]=1;}
    if(!d->automated){int n=minimum(minimum(d->crew_count+sector/3,d->crew_max),GAME_MAX_ENEMY_CREW),species=species_code(d->crew_species);
        static const int stations[]={SYS_WEAPONS,SYS_SHIELDS,SYS_ENGINES,SYS_MEDBAY,SYS_OXYGEN};
        if(g->enemy_crew_count<n)g->enemy_crew_count=n;
        for(i=0;i<n;i++){int room=i==0&&g->enemy_pilot_room>=0?g->enemy_pilot_room:-1;
            if(room<0){int s=stations[(i-1+5)%5];room=g->enemy.systems[s].room;}
            if(room<0)room=g->enemy.rooms[i%g->enemy.room_count].id;
            game_init_crew(g,1,i,room);game_set_crew_species(g,1,i,species);}}
    g->enemy.shields=effective(&g->enemy,SYS_SHIELDS)/2;
    if(g->enemy.systems[SYS_WEAPONS].room>=0)ship_weapons(g,d,sector,g->enemy.systems[SYS_WEAPONS].capacity);
    g->enemy_board_mode=g->teleporter[1].installed?1:0;g->enemy_board_stay=-1;g->enemy_board_sends=0;
    species_refresh_power(g);
    g->target_room=g->enemy.systems[SYS_WEAPONS].room>=0?g->enemy.systems[SYS_WEAPONS].room:g->enemy.rooms[0].id;
}
static int begin_encounter(Game *g,int kind,int sector,int phase);
static unsigned random_below(Game*g,unsigned n);
void game_clear_status(Game*g){for(int s=0;s<2;s++)for(int k=0;k<8;k++)g->status_cap[s][k]=-1;}
void game_enforce_status(Game*g,int side){Ship*ship=side?&g->enemy:&g->player;
    for(int k=0;k<GAME_SYSTEM_COUNT;k++)if(g->status_cap[side][k]>=0&&ship->systems[k].power>g->status_cap[side][k])ship->systems[k].power=g->status_cap[side][k];
    ship->shields=minimum(ship->shields,game_shield_capacity(ship));}
void game_apply_status(Game*g,int type,int side,int system,int amount){Ship*ship;int capacity,cap;
    if(side<0||side>1||system<0||system>=8||system==5)return;
    ship=side?&g->enemy:&g->player;
    if(type==3){g->status_cap[side][system]=-1;return;}
    capacity=system<GAME_SYSTEM_COUNT?ship->systems[system].capacity:g->subsystems[side][system==7?SUB_DOORS:SUB_SENSORS].capacity;
    cap=type==0?amount:type==1?(amount>0?capacity/amount:capacity):capacity-amount;
    g->status_cap[side][system]=cap<0?0:cap;
    game_enforce_status(g,side);}
int game_spawn_intruders(Game*g,int count,int species){int spawned=0,room;
    if(count<1||g->player.room_count<=0)return 0;
    room=g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id;
    for(int n=0;n<count;n++){int slot=-1;
        for(int i=0;i<GAME_MAX_ENEMY_CREW&&slot<0;i++)if(i>=g->enemy_crew_count||(g->enemy_crew[i].health<=0&&(!g->enemy_crew[i].id||g->enemy_crew[i].death_processed)))slot=i;
        if(slot<0)break;
        game_init_crew(g,1,slot,room);game_set_crew_species(g,1,slot,species>=0&&species<SPEC_COUNT?species:(int)random_below(g,(unsigned)SPEC_COUNT));
        g->enemy_crew[slot].ship=0;g->enemy_crew[slot].station=room;
        if(g->enemy_crew_count<=slot)g->enemy_crew_count=slot+1;
        spawned++;}
    if(spawned)say(g,"Intruders have boarded the ship!");
    return spawned;}
int game_reactor(const Game*g){return g->environment==ENV_STORM?g->player.reactor/2:g->player.reactor;}
int game_sensors_blinded(const Game*g){return g->environment==ENV_NEBULA||g->environment==ENV_STORM;}
/* Ion storm: both reactors halve; power comes off the least vital systems first. */
static int cut_power(int*p){if(*p>0){(*p)--;return 1;}return 0;}
void game_set_environment(Game*g,int env){
    g->environment=env>=ENV_NONE&&env<=ENV_STORM?env:ENV_NONE;
    g->env_ticks=g->environment==ENV_ASTEROID?3*GAME_HZ:g->environment==ENV_SUN?20*GAME_HZ:0;
    if(g->environment!=ENV_STORM)return;
    while(game_used_power(g)>game_reactor(g)){
        if(cut_power(&g->drones.power)||cut_power(&g->teleporter[0].power)||cut_power(&g->player_cloak.power)||cut_power(&g->player_artillery.power))continue;
        if(cut_power(&g->player.systems[SYS_MEDBAY].power)||cut_power(&g->player.systems[SYS_ENGINES].power)||cut_power(&g->player.systems[SYS_WEAPONS].power))continue;
        if(cut_power(&g->player.systems[SYS_SHIELDS].power)||cut_power(&g->player.systems[SYS_OXYGEN].power))continue;
        break;}
    {int total,i;g->enemy.reactor=g->enemy.reactor>1?g->enemy.reactor/2:1;
     for(;;){total=g->enemy_drone.power+g->teleporter[1].power;for(i=0;i<GAME_SYSTEM_COUNT;i++)total+=g->enemy.systems[i].power;
      if(total<=g->enemy.reactor)break;
      if(cut_power(&g->enemy_drone.power)||cut_power(&g->teleporter[1].power)||cut_power(&g->enemy.systems[SYS_MEDBAY].power)||cut_power(&g->enemy.systems[SYS_ENGINES].power))continue;
      if(cut_power(&g->enemy.systems[SYS_WEAPONS].power)||cut_power(&g->enemy.systems[SYS_SHIELDS].power)||cut_power(&g->enemy.systems[SYS_OXYGEN].power))continue;
      break;}}
    g->player.shields=minimum(g->player.shields,game_shield_capacity(&g->player));g->enemy.shields=minimum(g->enemy.shields,game_shield_capacity(&g->enemy));
}
/* Asteroids: one every 2-7 s at a random ship (1 damage, stopped by shields).
 * Suns: a solar flare every 25-35 s starts a fire on each ship. */
static void environment_step(Game*g){int side,i;
    if((g->environment!=ENV_ASTEROID&&g->environment!=ENV_SUN)||g->player.hull<=0||g->enemy.room_count<=0)return;
    if(--g->env_ticks>0)return;
    if(g->environment==ENV_ASTEROID){GameDamage rock={1,1,0,0,0,0,0,0};const Ship*s;side=random_below(g,2u);s=side?&g->player:&g->enemy;
        if(side||g->enemy.hull>0){g->launch_origin=0;game_launch_effect(g,side,s->rooms[random_below(g,(unsigned)s->room_count)].id,EFFECT_LASER,&rock,0);}
        g->env_ticks=2*GAME_HZ+random_below(g,5u*GAME_HZ);return;}
    for(side=0;side<2;side++){Ship*s=side?&g->enemy:&g->player;int n=1;
        if(side&&g->enemy.hull<=0)continue;
        for(i=0;i<n;i++)fire_ignite(s,s->rooms[random_below(g,(unsigned)s->room_count)].id,1);}
    say(g,"Solar flare! Fires on both ships.");
    g->env_ticks=25*GAME_HZ+random_below(g,10u*GAME_HZ);
}
/* V59: every encounter starts fresh achievement tracking. */
int game_begin_encounter(Game *g,int kind,int sector,int phase){int ok=begin_encounter(g,kind,sector,phase);if(ok)achievements_begin_encounter(g);return ok;}
static int begin_encounter(Game *g,int kind,int sector,int phase) {
    int i,j;
    if(!g||g->enemy_crew_count<0||g->enemy_crew_count>GAME_MAX_HOSTILES||g->crew_count<1||g->crew_count>GAME_MAX_CREW)return 0;
    for(i=GAME_MAX_ENEMY_CREW;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].ship!=0&&(g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))){say(g,"Invalid carried intruder location.");return 0;}
    for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++)if((g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))&&g->enemy_crew[i].ship==0){say(g,"Carry hostile boarders before departure.");return 0;}
    if(game_away_crew(g)){say(g,"Recall or abandon the away crew first.");return 0;}
    boarding_prepare_departure(g);
    drones_reset(g);
    if(kind<0 || kind>=ENEMY_KIND_COUNT)kind=ENEMY_SCOUT;
    if(sector<0)sector=0;
    if(sector>7)sector=7;
    if(phase<0)phase=0;
    if(phase>2)phase=2;
    memset(&g->enemy_drone,0,sizeof(g->enemy_drone));g->enemy_drone.room=-1;
    g->flagship_stage=-1;g->artillery_count=0;g->flagship_drone_count=g->flagship_drone_parts=g->surge_ticks=0;g->surge_count=g->super_shield=g->super_shield_max=0;memset(g->surge,0,sizeof g->surge);memset(g->artillery,0,sizeof g->artillery);memset(&g->enemy_cloak,0,sizeof g->enemy_cloak);g->enemy_cloak.room=-1;
    memset(&g->enemy,0,sizeof(g->enemy));memset(&g->enemy_weapon,0,sizeof(g->enemy_weapon));memset(&g->enemy_weapon_secondary,0,sizeof g->enemy_weapon_secondary);g->enemy_weapon_count=0;
    memset(g->effects,0,sizeof(g->effects));memset(g->autofire,0,sizeof(g->autofire));
    g->player_ftl_charge=0;
    g->enemy_escape_ticks=g->enemy_escape_total=g->enemy_escape_threshold=g->enemy_escape_active=g->enemy_departed=0;
    g->enemy_kind=kind;g->outcome=GAME_PLAYING;g->result_reason=RESULT_NONE;g->paused=0;
    subsystems_init(g,1);g->pilot_ion_ticks[1]=0;
    memset(&g->teleporter[1],0,sizeof g->teleporter[1]);
    g->enemy_crew_count=0;for(i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++)if(g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))g->enemy_crew_count=i+1;
    g->enemy_door_count=0;g->enemy_pilot_room=-1;g->enemy_pilot_health=0;
    memset(g->enemy_crew,0,GAME_MAX_ENEMY_CREW*sizeof g->enemy_crew[0]);memset(g->enemy_adjacency,0,sizeof g->enemy_adjacency);
    g->enemy_fire_ticks=g->enemy_pending_shots=g->enemy_shot_ticks=0;g->enemy_missiles=ENEMY_MISSILES;
    for(i=0;i<g->weapon_count;i++) {
        WeaponState *w=&g->weapons[i];w->charge_ticks=w->charge_accumulator=w->pending_shots=w->shot_ticks=0;w->aim_room=-1;w->bomb_side=w->queued_bomb_side=w->queued_bomb=0;w->beam_target_valid=0;w->queued_beam_length=0;
        if(game_augment_count(g,AUG_WEAPON_PREIGNITE))w->charge_ticks=w->cooldown_ticks; /* V55 Weapon Pre-igniter. */
    }
    g->player_cloak.active_ticks=g->player_cloak.ion_bars=g->player_cloak.ion_ticks=0;
    g->player_super_shield=game_augment_count(g,AUG_ENERGY_SHIELD)?5:0; /* V55 Zoltan Shield: 5 points per encounter (DS reading). */
    if(kind==ENEMY_SHIP){if(g->enemy_ship<4||g->enemy_ship>=IMPORTED_ENEMY_COUNT){g->enemy_kind=kind=ENEMY_REBEL;}else{ship_setup(g,sector);say(g,"Encounter started. Crew and ship state preserved.");return 1;}}
    g->enemy.hull=g->enemy.max_hull=ENEMY_HULL;
    g->enemy.reactor=ENEMY_REACTOR;g->enemy.automated=ENEMY_AUTOMATED;
    g->enemy.room_count=minimum(ENEMY_ROOM_COUNT,GAME_MAX_ROOMS);
    g->enemy.oxygen=g->enemy.automated?0:1000;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)g->enemy.systems[i].room=-1;
    for(i=0;i<g->enemy.room_count;i++) {
        GameRoom *r=&g->enemy.rooms[i];
        r->id=enemy_rooms[i].id;r->x=enemy_rooms[i].x;r->y=enemy_rooms[i].y;
        r->w=enemy_rooms[i].w;r->h=enemy_rooms[i].h;r->system=-1;
        r->oxygen=g->enemy.oxygen;
    }
    for(i=0;i<ENEMY_SYSTEM_COUNT;i++) {
        const EnemySystem *data=&enemy_systems[i];
        int system=identify_system(data->name);
        if(system<0 || !data->start)continue;
        g->enemy.systems[system].room=data->room;
        g->enemy.systems[system].capacity=data->power;
        g->enemy.systems[system].health=data->power;
        g->enemy.systems[system].power=data->power;
        j=room_index(&g->enemy,data->room);
        if(j>=0)g->enemy.rooms[j].system=system;
    }
    g->enemy.shields=effective(&g->enemy,SYS_SHIELDS)/2;
    {
        const EnemyWeapon *data=&enemy_weapons[0];
        WeaponState *w=&g->enemy_weapon;
        w->name=data->name;w->cooldown_ticks=data->cooldown_ms*GAME_HZ/1000;
        w->damage=data->damage;w->shots=data->shots;w->power=data->power;
        w->missiles=data->missiles;w->shield_piercing=data->shield_piercing;
    }
    g->target_room=g->enemy.systems[SYS_WEAPONS].room;

    /* Authored combat profiles reuse source hulls AND room geometry. Their
     * strengths and automated behavior remain DS approximations. */
    if(kind!=ENEMY_SCOUT && kind!=ENEMY_ASSAULT && kind!=ENEMY_REBEL && kind!=ENEMY_REBEL_BOARDER && kind!=ENEMY_REBEL_FAT && !(kind==ENEMY_FLAGSHIP&&!g->interim_flagship)) {
        const ImportedEnemy *layout=&imported_enemies[game_enemy_layout_index(g)];
        int rooms=layout->room_count;
        memset(g->enemy.rooms,0,sizeof(g->enemy.rooms));
        memset(g->enemy.systems,0,sizeof(g->enemy.systems));
        g->enemy.room_count=rooms;g->enemy.automated=1;
        for(i=0;i<rooms;i++) {
            const ImportedEnemyRoom *src=&layout->rooms[i];
            GameRoom *r=&g->enemy.rooms[i];r->id=src->id;r->x=src->x;r->y=src->y;r->w=src->w;r->h=src->h;r->system=-1;
        }
        for(i=0;i<GAME_SYSTEM_COUNT;i++)g->enemy.systems[i].room=-1;
        for(i=0;i<3;i++) {
            GameSystemState *sys=&g->enemy.systems[i];
            for(j=0;j<layout->system_count;j++)if(identify_system(layout->systems[j].name)==i)sys->room=layout->systems[j].room;
            sys->capacity=i==SYS_SHIELDS?(kind==ENEMY_INTERCEPTOR?0:2):i==SYS_ENGINES?2:3;
            sys->health=sys->power=sys->capacity;
            j=room_index(&g->enemy,sys->room);if(j>=0)g->enemy.rooms[j].system=i;
        }
        g->enemy.hull=g->enemy.max_hull=(kind==ENEMY_INTERCEPTOR?9:kind==ENEMY_ARMORED?12:18)+sector*2+phase*3;
        g->enemy_weapon.shots=kind==ENEMY_FLAGSHIP?3:2;
        g->enemy_weapon.cooldown_ticks=(kind==ENEMY_INTERCEPTOR?11:13)*GAME_HZ;
        g->enemy_weapon.damage=1;
        if(kind==ENEMY_FLAGSHIP) {
            g->enemy.systems[SYS_SHIELDS].capacity=phase==2?4:2;
            g->enemy.systems[SYS_SHIELDS].health=g->enemy.systems[SYS_SHIELDS].power=g->enemy.systems[SYS_SHIELDS].capacity;
            if(phase==1){g->enemy_weapon.shots=1;g->enemy_weapon.damage=2;g->enemy_weapon.missiles=1;g->enemy_weapon.shield_piercing=5;}
        }
    } else {g->enemy.hull+=sector*2;g->enemy.max_hull=g->enemy.hull;if(sector>=2)g->enemy_weapon.shots=2;}
    if(kind==ENEMY_ASSAULT || kind==ENEMY_REBEL || kind==ENEMY_REBEL_BOARDER || kind==ENEMY_REBEL_FAT) {
        const ImportedEnemy *data=&imported_enemies[game_enemy_import_index(g)];
        memset(&g->enemy,0,sizeof g->enemy);
        g->enemy.hull=g->enemy.max_hull=data->hull+sector*2;
        g->enemy.reactor=data->reactor;g->enemy.automated=data->automated;g->enemy_missiles=data->missiles;
        g->enemy.room_count=minimum(data->room_count,GAME_MAX_ROOMS);
        for(i=0;i<GAME_SYSTEM_COUNT;i++)g->enemy.systems[i].room=-1;
        for(i=0;i<g->enemy.room_count;i++) {
            const ImportedEnemyRoom *src=&data->rooms[i];GameRoom*r=&g->enemy.rooms[i];
            r->id=src->id;r->x=src->x;r->y=src->y;r->w=src->w;r->h=src->h;r->system=-1;r->oxygen=data->automated?0:1000;
        }
        for(i=0;i<data->system_count;i++) {
            const ImportedEnemySystem *src=&data->systems[i];int system=identify_system(src->name);
            if(!src->start && !(kind!=ENEMY_ASSAULT && !strcmp(src->name,"medbay")))continue;
            if(!strcmp(src->name,"pilot")){g->enemy_pilot_room=src->room;g->enemy_pilot_health=src->power;}
            if(system>=0){GameSystemState *s=&g->enemy.systems[system];s->room=src->room;s->capacity=s->health=s->power=src->power;
                j=room_index(&g->enemy,src->room);if(j>=0)g->enemy.rooms[j].system=system;}
            if(!strcmp(src->name,"drones")) {
                EnemyDrone*d=&g->enemy_drone;d->room=src->room;d->capacity=d->health=d->power=src->power;
                d->cooldown_ticks=10*GAME_HZ;d->damage=imported_combat_drone.damage;
                d->fire_chance=imported_combat_drone.fire_chance*10;d->shots=imported_combat_drone.shots;
            }
        }
        /* Base blueprint has no weapon power: one combat drone is its offense. */
        memset(&g->enemy_weapon,0,sizeof g->enemy_weapon);g->enemy_weapon.name="None";
        g->enemy_weapon.power=1;g->enemy_weapon.cooldown_ticks=10*GAME_HZ;
        if(kind!=ENEMY_ASSAULT){const ImportedEnemyWeapon*w=&imported_rebel_weapon;
            g->enemy_weapon.name=w->name;g->enemy_weapon.damage=w->damage;g->enemy_weapon.shots=w->shots;
            g->enemy_weapon.power=w->power;g->enemy_weapon.cooldown_ticks=w->cooldown_ms*GAME_HZ/1000;
            g->enemy.systems[SYS_MEDBAY].power=0;

        }
    }
    {int imported=game_enemy_layout_index(g);
     if(imported>=0){const ImportedEnemy *data=&imported_enemies[imported];
        g->enemy_door_count=minimum(data->door_count,GAME_MAX_DOORS);
        for(i=0;i<g->enemy_door_count;i++){
            const ImportedEnemyDoor *src=&data->doors[i];GameDoor *d=&g->enemy_doors[i];int a,b;
            memset(d,0,sizeof *d);d->x=src->x;d->y=src->y;d->room_a=src->room_a;d->room_b=src->room_b;d->vertical=src->vertical;
            a=room_index(&g->enemy,d->room_a);b=room_index(&g->enemy,d->room_b);
            if(a>=0&&b>=0)g->enemy_adjacency[a][b]=g->enemy_adjacency[b][a]=1;
        }
        if(game_enemy_import_index(g)>=0&&!data->automated){g->enemy.oxygen=1000;int current_count=minimum(data->crew_count,GAME_MAX_ENEMY_CREW);if(g->enemy_crew_count<current_count)g->enemy_crew_count=current_count;
            for(i=0;i<current_count;i++)game_init_crew(g,1,i,i==0?g->enemy_pilot_room:g->enemy.systems[i==1?SYS_WEAPONS:SYS_ENGINES].room);
        }
     }
    }
    if(kind==ENEMY_FLAGSHIP&&!g->interim_flagship)flagship_setup(g,phase);
    if(kind==ENEMY_REBEL_BOARDER){boarding_install(g,1);g->enemy.systems[SYS_WEAPONS].power=1;g->teleporter[1].power=1;}
    g->enemy_board_mode=g->teleporter[1].installed?(kind==ENEMY_FLAGSHIP?2:1):0;g->enemy_board_stay=-1;g->enemy_board_sends=0;
    g->enemy.shields=effective(&g->enemy,SYS_SHIELDS)/2;
    species_refresh_power(g);
    g->target_room=g->enemy.systems[SYS_WEAPONS].room;
    g->enemy_weapon.system_damage=g->enemy_weapon.damage;g->enemy_weapon.crew_damage=g->enemy_weapon.damage*15;
    g->enemy_weapon.fire_chance=(kind==ENEMY_SCOUT||kind==ENEMY_REBEL||kind==ENEMY_REBEL_BOARDER||kind==ENEMY_REBEL_FAT)?imported_rebel_weapon.fire_chance*10:0;
    say(g,"Encounter started. Crew and ship state preserved.");
    return 1;
}
void game_toggle_pause(Game *g) {
    if(g->outcome!=GAME_DEFEAT) {g->paused=!g->paused;say(g,g->paused?"Paused: issue orders or aim weapons.":"Combat resumed.");}
}
static void update_exposure(Game *g);
int game_toggle_door(Game *g,int index) {
    GameDoor *door;
    if(g->outcome==GAME_DEFEAT || index<0 || index>=g->door_count)return 0;
    if(!subsystem_level(g,0,SUB_DOORS)){say(g,"Door control needs repair.");return 0;}
    door=&g->doors[index];if(species_door_locked(g,0,door)){say(g,"Crystal lockdown seals this room.");return 0;}
    if(door->jam_ticks){say(g,"Broken door is stuck open.");return 0;}
    door->open=!door->open;door->transit_ticks=0;
    update_exposure(g);
    if(door->room_a<0 || door->room_b<0)
        say(g,door->open?"Airlock opened: connected rooms will vent.":"Airlock sealed.");
    else say(g,door->open?"Door opened.":"Door closed.");
    return 1;
}
void game_close_all_doors(Game *g) {
    int i,jammed=0;
    if(g->outcome==GAME_DEFEAT)return;
    if(!subsystem_level(g,0,SUB_DOORS)){say(g,"Door control needs repair.");return;}
    for(i=0;i<g->door_count;i++){if(g->doors[i].jam_ticks){jammed++;continue;}g->doors[i].open=0;g->doors[i].transit_ticks=0;}
    update_exposure(g);
    say(g,jammed?"Sealed working doors; broken doors stay open.":"All doors and airlocks closed.");
}
GameDoor *game_ship_doors(Game *g,int side,int *count){*count=side?g->enemy_door_count:g->door_count;return side?g->enemy_doors:g->doors;}
static int crew_pass_door(Game *g,int faction,int side,int from,int to) {
    int i,n;GameDoor *doors=game_ship_doors(g,side,&n);
    for(i=0;i<n;i++){GameDoor*d=&doors[i];if((d->room_a==from&&d->room_b==to)||(d->room_a==to&&d->room_b==from)){
        if(!subsystem_door_pass(g,faction,side,i))return 0;
        if(!d->open||d->transit_ticks>0){d->open=1;d->transit_ticks=15;}return 1;}}
    return 0;
}
int game_next_room(const Game *g,int side,int source,int dest) {
    const Ship*s=side?&g->enemy:&g->player;
    const unsigned char(*adj)[GAME_MAX_ROOMS]=side?g->enemy_adjacency:g->adjacency;
    int q[GAME_MAX_ROOMS],parent[GAME_MAX_ROOMS],head=0,tail=0,i,node;
    int a=room_index(s,source),b=room_index(s,dest);if(a<0||b<0)return -1;
    for(i=0;i<GAME_MAX_ROOMS;i++)parent[i]=-1;
    q[tail++]=a;parent[a]=a;
    while(head<tail){node=q[head++];if(node==b)break;
        for(i=0;i<s->room_count;i++)if(adj[node][i]&&parent[i]<0){parent[i]=node;q[tail++]=i;}}
    if(parent[b]<0)return -1;
    while(b!=a&&parent[b]!=a)b=parent[b];
    return s->rooms[b].id;
}
int game_crew_next_room_side(const Game*g,int faction,int index){const Crew*c;
    if(index<0||index>=(faction?g->enemy_crew_count:g->crew_count))return -1;
    c=faction?&g->enemy_crew[index]:&g->crew[index];return game_next_room(g,c->ship,c->room,c->destination);
}
int game_crew_next_room(const Game*g,int index){return game_crew_next_room_side(g,0,index);}
static int crew_healing(const Game*g,const Crew*c,int faction){const Ship*s=c->ship?&g->enemy:&g->player;
    return !g->paused&&g->outcome!=GAME_DEFEAT&&c->ship==faction&&c->health>0&&c->health<species_max_health(c->species)&&c->room==c->destination&&
        c->room==s->systems[SYS_MEDBAY].room&&effective(s,SYS_MEDBAY)>0;
}
int game_crew_healing(const Game*g,int index){return index>=0&&index<g->crew_count&&crew_healing(g,&g->crew[index],0);}
int game_order_group(Game*g,unsigned mask,int room){int i,side=-1;
    if(!mask||(mask>>g->crew_count)||g->outcome==GAME_DEFEAT)return 0;
    for(i=0;i<g->crew_count;i++)if(mask&(1u<<i)){Crew*c=&g->crew[i];if(c->health<=0||(side>=0&&side!=c->ship)||game_next_room(g,c->ship,c->room,room)<0){say(g,"Select living crew on the same ship with a valid route.");return 0;}side=c->ship;}
    for(i=0;i<g->crew_count;i++)if(mask&(1u<<i))game_order_crew(g,i,room);
    return 1;
}
int game_order_crew(Game*g,int index,int room){Crew*c;
    if(g->outcome==GAME_DEFEAT||index<0||index>=g->crew_count||g->crew[index].health<=0)return 0;
    c=&g->crew[index];
    if(game_next_room(g,c->ship,c->room,room)<0){say(g,"No route to that room.");return 0;}
    c->destination=room;c->move_ticks=c->repair_ticks=c->heal_accumulator=0;say(g,"Crew order set.");return 1;
}
void game_init_crew(Game*g,int faction,int index,int room){
    static const char*names[]={"Alex","Sam","Jordan","Morgan","Casey","Robin","Taylor","Riley","Avery","Blake","Quinn","Reese"};Crew*c;
    if(index<0||index>=(faction?GAME_MAX_HOSTILES:GAME_MAX_CREW))return;
    c=faction?&g->enemy_crew[index]:&g->crew[index];memset(c,0,sizeof *c);
    if(!g->next_crew_id)g->next_crew_id=1;
    c->id=g->next_crew_id++;c->ship=faction;c->room=c->destination=c->station=room;c->health=100;
    snprintf(c->name,sizeof c->name,"%s",names[(c->id-1)%12]);
}
int game_rename_crew(Game*g,int index,const char*name){int n=0,start=0,end,i;char clean[GAME_CREW_NAME_SIZE]={0};
    if(index<0||index>=g->crew_count||!name)return 0;
    while(n<64&&name[n])n++;
    if(n==64)return 0;
    while(start<n&&name[start]==' ')start++;
    end=n;
    while(end>start&&name[end-1]==' ')end--;
    if(end-start<1||end-start>=GAME_CREW_NAME_SIZE)return 0;
    for(i=start;i<end;i++){unsigned char c=(unsigned char)name[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c==' '||c=='-'||c==39))return 0;}
    memcpy(clean,name+start,(unsigned)(end-start));memcpy(g->crew[index].name,clean,sizeof clean);return 1;
}
int game_save_stations(Game*g){int i,n=0;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==0){g->crew[i].station=g->crew[i].room;n++;}return n;}
int game_return_stations(Game*g){int i,n=0;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==0)n+=game_order_crew(g,i,g->crew[i].station);return n;}
const char*game_crew_name(const Game*g,int i){return i>=0&&i<g->crew_count?g->crew[i].name:"";}
const char*game_crew_action(const Game*g,int i){const Crew*c;const Ship*s;int r;
    if(i<0||i>=g->crew_count)return "";
    c=&g->crew[i];s=c->ship?&g->enemy:&g->player;r=room_index(s,c->room);
    if(c->health<=0)return "DEAD";
    if(boarding_contested(g,c,0))return "FIGHT";
    if(c->ship==1&&c->room==c->destination)return "SABOTAGE";
    if(c->room!=c->destination)return "WALK";
    if(r>=0&&s->rooms[r].fires)return "FIREFIGHT";
    if(r>=0&&s->rooms[r].breaches&&c->ship==0)return "SEAL";
    if(crew_healing(g,c,0))return "HEAL";
    if(r>=0&&s->rooms[r].system>=0&&s->systems[s->rooms[r].system].health<s->systems[s->rooms[r].system].capacity)return "REPAIR";
    {int sub;for(sub=0;sub<SUB_COUNT;sub++)if(g->subsystems[c->ship][sub].room==c->room&&g->subsystems[c->ship][sub].health<g->subsystems[c->ship][sub].capacity)return "REPAIR";}
    if(c->ship==0&&c->room==g->pilot_room&&!g->pilot_health)return "REPAIR";
    return "IDLE";
}
static int can_carry_intruders(const Game*g){int need=0,free_count=0,i;
    if(!g||g->enemy_crew_count<0||g->enemy_crew_count>GAME_MAX_HOSTILES)return 0;
    for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++)need+=(g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))&&g->enemy_crew[i].ship==0;
    for(i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++){const Crew*c=&g->enemy_crew[i];
        if((c->health>0||(c->id&&!c->death_processed))&&c->ship!=0)return 0;
        free_count+=c->health<=0&&(!c->id||c->death_processed);
    }
    return need<=free_count;
}
int game_carry_intruders(Game*g){int i,next=GAME_MAX_ENEMY_CREW;
    if(!can_carry_intruders(g))return 0;
    for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++)if((g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))&&g->enemy_crew[i].ship==0){
        while(g->enemy_crew[next].health>0||(g->enemy_crew[next].id&&!g->enemy_crew[next].death_processed))next++;
        g->enemy_crew[next]=g->enemy_crew[i];memset(&g->enemy_crew[i],0,sizeof g->enemy_crew[i]);
        if(g->enemy_crew_count<=next)g->enemy_crew_count=next+1;
        next++;
    }
    return 1;
}
int game_can_depart(const Game*g,int abandon_away){int i;
    if(!g||(abandon_away!=0&&abandon_away!=1)||g->crew_count<1||g->crew_count>GAME_MAX_CREW||
       g->weapon_count<0||g->weapon_count>GAME_MAX_WEAPONS||g->player.room_count<1||g->player.room_count>GAME_MAX_ROOMS||!game_player_ftl_ready(g)||
       (g->enemy_departed&&g->result_reason==RESULT_PLAYER_ESCAPE)||!can_carry_intruders(g))return 0;
    if(!abandon_away&&game_away_crew(g))return 0;
    for(i=0;i<GAME_MAX_HOSTILES;i++){const Crew*c=&g->enemy_crew[i];
        if(i<GAME_MAX_ENEMY_CREW&&i>=g->enemy_crew_count)continue;
        if(c->health<=0&&(!c->id||c->death_processed))continue;
        if(i>=GAME_MAX_ENEMY_CREW&&c->ship!=0)return 0;
        if(c->ship==0&&(room_index(&g->player,c->room)<0||room_index(&g->player,c->destination)<0||game_next_room(g,0,c->room,c->destination)<0))return 0;
    }
    return 1;
}
int game_depart(Game*g,int abandon_away){int i;
    if(!game_can_depart(g,abandon_away))return 0;
    /* Preflight proves the promotion cannot fail; no Game-sized stack staging. */
    if(!game_carry_intruders(g))return 0;
    for(i=0;i<g->crew_count;i++)if(g->crew[i].ship==1){g->crew[i].health=0;g->crew[i].death_processed=1;}
    boarding_prepare_departure(g);
    memset(g->enemy_crew,0,GAME_MAX_ENEMY_CREW*sizeof g->enemy_crew[0]);
    g->enemy_crew_count=0;for(i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++)if(g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))g->enemy_crew_count=i+1;
    memset(&g->enemy,0,sizeof g->enemy);memset(&g->enemy_weapon,0,sizeof g->enemy_weapon);memset(&g->enemy_weapon_secondary,0,sizeof g->enemy_weapon_secondary);g->enemy_weapon_count=0;
    memset(&g->enemy_drone,0,sizeof g->enemy_drone);g->enemy_drone.room=-1;
    memset(g->enemy_doors,0,sizeof g->enemy_doors);memset(g->enemy_adjacency,0,sizeof g->enemy_adjacency);g->enemy_door_count=0;
    memset(g->subsystems[1],0,sizeof g->subsystems[1]);memset(&g->teleporter[1],0,sizeof g->teleporter[1]);
    g->enemy_pilot_room=-1;g->enemy_pilot_health=g->pilot_ion_ticks[1]=0;
    g->enemy_fire_ticks=g->enemy_pending_shots=g->enemy_shot_ticks=g->enemy_missiles=0;
    g->enemy_escape_ticks=g->enemy_escape_total=g->enemy_escape_threshold=g->enemy_escape_active=0;
    memset(g->effects,0,sizeof g->effects);memset(g->autofire,0,sizeof g->autofire);drones_reset(g);
    for(i=0;i<g->weapon_count;i++){WeaponState*w=&g->weapons[i];
        w->charge_ticks=w->charge_accumulator=w->pending_shots=w->shot_ticks=0;w->target_room=w->aim_room=-1;
        w->queued_damage=(GameDamage){0};w->queued_ion_damage=w->queued_bomb=w->queued_bomb_side=w->bomb_side=0;
        w->beam_target_valid=w->queued_beam_length=w->queued_beam_x0=w->queued_beam_y0=w->queued_beam_x1=w->queued_beam_y1=0;
    }
    g->target_room=-1;g->player_ftl_charge=0;g->enemy_departed=1;g->result_reason=RESULT_PLAYER_ESCAPE;
    return 1;
}
typedef struct {int player_room[GAME_MAX_CREW],enemy_room[GAME_MAX_ENEMY_CREW],promote[GAME_MAX_ENEMY_CREW];} SurrenderPlan;
static int surrender_room(const Ship*s,int preferred,int*occupancy){
    int at=room_index(s,preferred);
    if(at>=0&&occupancy[at]<s->rooms[at].w*s->rooms[at].h){occupancy[at]++;return preferred;}
    for(int i=0;i<s->room_count;i++)if(occupancy[i]<s->rooms[i].w*s->rooms[i].h){occupancy[i]++;return s->rooms[i].id;}
    return -1;
}
static int surrender_plan(const Game*g,SurrenderPlan*p){int home[GAME_MAX_ROOMS]={0},remote[GAME_MAX_ROOMS]={0},next=GAME_MAX_ENEMY_CREW;
    if(!g||g->outcome!=GAME_PLAYING||g->enemy_departed||g->player.hull<=0||g->enemy.hull<=0||
       g->crew_count<1||g->crew_count>GAME_MAX_CREW||g->enemy_crew_count<0||g->enemy_crew_count>GAME_MAX_HOSTILES||
       g->weapon_count<0||g->weapon_count>GAME_MAX_WEAPONS||g->player.room_count<1||g->player.room_count>GAME_MAX_ROOMS||
       g->enemy.room_count<1||g->enemy.room_count>GAME_MAX_ROOMS||!game_alive_crew(g)||(!g->enemy.automated&&!game_alive_enemy_crew(g)))return 0;
    for(int i=0;i<GAME_MAX_CREW;i++)p->player_room[i]=-1;
    for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)p->enemy_room[i]=p->promote[i]=-1;
    for(int side=0;side<2;side++){const Ship*s=side?&g->enemy:&g->player;for(int r=0;r<s->room_count;r++)if(s->rooms[r].w<1||s->rooms[r].h<1||s->rooms[r].w>32||s->rooms[r].h>32)return 0;}
    for(int i=0;i<g->crew_count;i++){const Crew*c=&g->crew[i];
        if(c->ship<0||c->ship>1)return 0;
        if(c->health>0){int r=room_index(c->ship?&g->enemy:&g->player,c->room);if(r<0)return 0;if(!c->ship)home[r]++;}
    }
    for(int i=0;i<GAME_MAX_HOSTILES;i++){const Crew*c=&g->enemy_crew[i];
        if(i<GAME_MAX_ENEMY_CREW&&i>=g->enemy_crew_count)continue;
        if(c->health<=0&&(!c->id||c->death_processed))continue;
        if(c->ship<0||c->ship>1||(i>=GAME_MAX_ENEMY_CREW&&c->ship!=0))return 0;
        int r=room_index(c->ship?&g->enemy:&g->player,c->room);if(r<0)return 0;
        if(i<GAME_MAX_ENEMY_CREW&&c->health>0&&c->ship==1)remote[r]++;
    }
    for(int r=0;r<g->player.room_count;r++)if(home[r]>g->player.rooms[r].w*g->player.rooms[r].h)return 0;
    for(int r=0;r<g->enemy.room_count;r++)if(remote[r]>g->enemy.rooms[r].w*g->enemy.rooms[r].h)return 0;
    for(int i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==1){
        p->player_room[i]=surrender_room(&g->player,g->crew[i].station,home);if(p->player_room[i]<0)return 0;
    }
    for(int i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++){const Crew*c=&g->enemy_crew[i];
        if(c->ship!=0)continue;
        if(c->health>0){p->enemy_room[i]=surrender_room(&g->enemy,c->station,remote);if(p->enemy_room[i]<0)return 0;}
        else if(c->id&&!c->death_processed){
            while(next<GAME_MAX_HOSTILES&&(g->enemy_crew[next].health>0||(g->enemy_crew[next].id&&!g->enemy_crew[next].death_processed)))next++;
            if(next==GAME_MAX_HOSTILES)return 0;
            p->promote[i]=next++;
        }
    }
    return 1;
}
int game_can_accept_surrender(const Game*g){SurrenderPlan plan;return surrender_plan(g,&plan);}
static void ceasefire_return(Crew*c,int side,int room){
    c->ship=side;c->room=c->destination=room;
    c->move_ticks=c->repair_ticks=c->heal_accumulator=c->combat_ticks=c->sabotage_ticks=0;
}
int game_accept_surrender(Game*g){SurrenderPlan plan;int i;
    if(!surrender_plan(g,&plan))return 0;
    /* Authored DS negotiation exchanges living parties without teleporter power.
     * Local pending deaths keep their location; remote death effects are retired
     * at the ceasefire so later corpse normalization cannot cause home damage. */
    for(i=0;i<g->crew_count;i++){
        if(plan.player_room[i]>=0)ceasefire_return(&g->crew[i],0,plan.player_room[i]);
        else if(g->crew[i].ship==1&&g->crew[i].health<=0)g->crew[i].death_processed=1;
    }
    for(i=0;i<GAME_MAX_ENEMY_CREW;i++){
        if(plan.enemy_room[i]>=0)ceasefire_return(&g->enemy_crew[i],1,plan.enemy_room[i]);
        if(plan.promote[i]>=0){int slot=plan.promote[i];g->enemy_crew[slot]=g->enemy_crew[i];memset(&g->enemy_crew[i],0,sizeof g->enemy_crew[i]);if(g->enemy_crew_count<=slot)g->enemy_crew_count=slot+1;}
        else if(g->enemy_crew[i].ship==1&&g->enemy_crew[i].health<=0)g->enemy_crew[i].death_processed=1;
    }
    memset(g->effects,0,sizeof g->effects);memset(g->autofire,0,sizeof g->autofire);drones_reset(g);
    for(i=0;i<g->weapon_count;i++){WeaponState*w=&g->weapons[i];w->pending_shots=w->shot_ticks=0;w->queued_damage=(GameDamage){0};w->queued_ion_damage=w->queued_bomb=w->queued_bomb_side=w->queued_beam_length=0;}
    enemy_loadout_stop(g);g->enemy_pending_shots=g->enemy_fire_ticks=g->enemy_shot_ticks=g->enemy_weapon.pending_shots=g->enemy_weapon.charge_ticks=0;
    g->enemy_drone.charge_ticks=0;g->enemy_escape_ticks=g->enemy_escape_total=g->enemy_escape_threshold=g->enemy_escape_active=0;
    g->outcome=GAME_SURRENDER;g->result_reason=RESULT_ENEMY_SURRENDER;species_refresh_power(g);
    say(g,"Surrender accepted. Weapons stopped.");
    return 1;
}
int game_alive_enemy_crew(const Game*g){int i,n=0;for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++)n+=g->enemy_crew[i].health>0;return n;}
int game_adjust_power(Game *g,int system,int delta) {
    GameSystemState *s;
    if(g->outcome==GAME_DEFEAT || system<0 || system>=GAME_SYSTEM_COUNT || (delta!=1 && delta!=-1)) return 0;
    s=&g->player.systems[system];
    if(s->power+delta<0 || s->power+delta+s->bonus_power>s->health) {say(g,"System capacity reached.");return 0;}
    if(delta>0 && g->status_cap[0][system]>=0 && s->power+delta>g->status_cap[0][system]) {say(g,"An event limits this system's power.");return 0;}
    if(delta>0 && game_used_power(g)>=game_reactor(g)) {say(g,"No free reactor power.");return 0;}
    s->power+=delta;
    if(system==SYS_MEDBAY && effective(&g->player,SYS_MEDBAY)==0) {
        int i;for(i=0;i<g->crew_count;i++)g->crew[i].heal_accumulator=0;
    }
    say(g,"Power adjusted.");return 1;
}
int game_toggle_autofire(Game *g,int weapon) {
    if(g->outcome || weapon<0 || weapon>=g->weapon_count)return 0;
    g->autofire[weapon]=!g->autofire[weapon];
    snprintf(g->message,sizeof g->message,"%s autofire %s.",
        game_weapon_name(g->weapons[weapon].catalog_id),g->autofire[weapon]?"enabled":"disabled");
    return 1;
}
int game_fire(Game *g,int weapon,int room) {
    WeaponState *w;
    if(g->outcome || weapon<0 || weapon>=g->weapon_count) return 0;
    w=&g->weapons[weapon];
    if(g->enemy_departed&&!(w->bomb&&w->bomb_side==1)){say(g,"Enemy ship departed. No remote target.");return 0;}
    if(!w->beam_length&&room_index(w->bomb&&w->bomb_side?&g->player:&g->enemy,room)<0)return 0;
    if(w->beam_length&&!w->beam_target_valid){say(g,"Choose both beam endpoints first.");return 0;}
    if(game_enemy_cloaked(g)&&!(w->bomb&&w->bomb_side)){say(g,"Enemy ship is cloaked. Weapons cannot target it.");return 0;}
    if(!game_weapon_powered(g,weapon)) {say(g,"Weapon needs power.");return 0;}
    if(!game_weapons_manned(g,0)) {say(g,"Weapons room unmanned. Station a crewmember.");return 0;}
    if(w->charge_ticks<w->cooldown_ticks || w->pending_shots) {say(g,"Weapon is charging.");return 0;}
    if(g->missiles<w->missiles) {say(g,"No missiles remaining.");return 0;}
    g->missiles-=w->missiles;w->charge_ticks=0;w->pending_shots=w->beam_length?1:w->shots;
    if(game_player_cloaked(g)&&!game_augment_count(g,AUG_CLOAK_FIRE)){EnemyCloak*k=&g->player_cloak;k->active_ticks=k->active_ticks>2*GAME_HZ?k->active_ticks-2*GAME_HZ:1;}
    w->shot_ticks=0;w->target_room=room;g->target_room=room;
    w->queued_bomb=w->bomb;w->queued_bomb_side=w->bomb_side;
    w->queued_beam_length=w->beam_length;w->queued_beam_x0=w->beam_x0;w->queued_beam_y0=w->beam_y0;w->queued_beam_x1=w->beam_x1;w->queued_beam_y1=w->beam_y1;
    w->queued_ion_damage=w->ion_damage;
    w->queued_damage=weapon_damage(w);
    say(g,g->paused?"Volley queued for unpause.":"Volley launched.");return 1;
}
static unsigned int random100(Game *g) {
    g->rng=g->rng*1664525u+1013904223u;return (g->rng>>16)%100u;
}
static int enemy_manned(const Game*g,int room);
static unsigned random_below(Game*g,unsigned n){g->rng=g->rng*1664525u+1013904223u;return (g->rng>>16)%n;}
/* Recovered from FTL 1.6.14: CombatAI's constructor fills system_targets with
 * shields, engines, weapons, oxygen and drones. PrioritizeSystem picks one of
 * those 33% of the time on normal (20% easy) when the player has it; otherwise
 * ChooseTarget uses a uniformly random room. Hard-mode priorities are unused. */
#define ENEMY_SYSTEM_TARGET_PERCENT 33
static int enemy_choose_room(Game*g){int room=-1;
    if(g->player.room_count<1)return -1;
    if(random100(g)<ENEMY_SYSTEM_TARGET_PERCENT){
        static const int targets[]={SYS_SHIELDS,SYS_ENGINES,SYS_WEAPONS,SYS_OXYGEN,-1};
        int pick=targets[random_below(g,5u)];
        if(pick<0){if(g->drones.installed&&room_index(&g->player,g->drones.room)>=0)room=g->drones.room;}
        else if(g->player.systems[pick].capacity>0&&room_index(&g->player,g->player.systems[pick].room)>=0)room=g->player.systems[pick].room;
    }
    if(room<0)room=g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id;
    return room;
}
/* UpdateWeapons asks for one target per projectile and, while the ship has
 * enough rooms, drops an earlier equal target before appending the new one. */
int game_enemy_plan_volley(Game*g,int*rooms,int count){int n=0,guard=0;
    if(!g||!rooms||count<1||g->player.room_count<1)return 0;
    if(count>g->player.room_count)count=g->player.room_count;
    if(count>GAME_MAX_VOLLEY)count=GAME_MAX_VOLLEY;
    while(n<count&&guard++<256){int room=enemy_choose_room(g),i,j;
        for(i=0;i<n;i++)if(rooms[i]==room){for(j=i;j+1<n;j++)rooms[j]=rooms[j+1];n--;break;}
        rooms[n++]=room;
    }
    /* Bounded fallback for pathological RNG streaks: complete deterministically. */
    for(int r=0;n<count&&r<g->player.room_count;r++){int i,id=g->player.rooms[r].id;for(i=0;i<n&&rooms[i]!=id;i++);if(i==n)rooms[n++]=id;}
    return n;
}
static void room_centre(const GameRoom*r,int*x,int*y){*x=(2*r->x+r->w)*GAME_BEAM_UNITS/2;*y=(2*r->y+r->h)*GAME_BEAM_UNITS/2;}
/* Beams require two targets; the line keeps the weapon's full length in the
 * second target's direction and is trimmed only where it leaves the hull. */
int game_enemy_plan_beam(Game*g,int length,int*x0,int*y0,int*x1,int*y1){int rooms[2],minx=32767,miny=32767,maxx=0,maxy=0,i,dx,dy;long long d2,lo=0,hi=65536;
    if(!g||!x0||!y0||!x1||!y1||length<1||game_enemy_plan_volley(g,rooms,2)!=2)return 0;
    room_centre(&g->player.rooms[room_index(&g->player,rooms[0])],x0,y0);room_centre(&g->player.rooms[room_index(&g->player,rooms[1])],x1,y1);
    for(i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];
        if(r->x*GAME_BEAM_UNITS<minx)minx=r->x*GAME_BEAM_UNITS;
        if(r->y*GAME_BEAM_UNITS<miny)miny=r->y*GAME_BEAM_UNITS;
        if((r->x+r->w)*GAME_BEAM_UNITS>maxx)maxx=(r->x+r->w)*GAME_BEAM_UNITS;
        if((r->y+r->h)*GAME_BEAM_UNITS>maxy)maxy=(r->y+r->h)*GAME_BEAM_UNITS;}
    dx=*x1-*x0;dy=*y1-*y0;d2=(long long)dx*dx+(long long)dy*dy;if(!d2)return 0;
    while(lo<hi){long long mid=(lo+hi)/2;if(mid*mid<d2)lo=mid+1;else hi=mid;}
    {int ex=*x0+(int)((long long)dx*length/lo),ey=*y0+(int)((long long)dy*length/lo),k;
     /* Shorten toward the start until inside the hull bounds; rooms lie inside. */
     for(k=16;k>0&&(ex<minx||ex>maxx||ey<miny||ey>maxy);k--){ex=*x0+(ex-*x0)*7/8;ey=*y0+(ey-*y0)*7/8;}
     if(ex<minx)ex=minx;
     if(ex>maxx)ex=maxx;
     if(ey<miny)ey=miny;
     if(ey>maxy)ey=maxy;
     *x1=ex;*y1=ey;}
    return beam_valid_line(&g->player,*x0,*y0,*x1,*y1);
}
static void ion_stack(int*bars,int*ticks,int amount){*bars=minimum(GAME_ION_MAX_BARS,*bars+amount);*ticks=minimum(GAME_ION_MAX_TICKS,*ticks+amount*GAME_ION_TICKS);}
/* V42 Rebel flagship from imported BOSS_1..3_NORMAL stages (docs/V42_FLAGSHIP.md).
 * BossShip::StartStage (FTL 1.6.14) seeds stage 1 with one human in rooms
 * 0..6, 9, 10, 14 and 18; later stages reuse the surviving per-room counts and
 * drop rooms the smaller hull no longer has. */
static const int flagship_default_crew[GAME_FLAGSHIP_ROOMS]={1,1,1,1,1,1,1,0,0,1,1,0,0,0,1,0,0,0,1};
int game_is_flagship(const Game*g){return g&&g->enemy_kind==ENEMY_FLAGSHIP&&g->flagship_stage>=0&&g->flagship_stage<FLAGSHIP_STAGE_COUNT;}
int game_enemy_cloaked(const Game*g){return g&&g->enemy.hull>0&&!g->enemy_departed&&g->enemy_cloak.capacity>0&&g->enemy_cloak.active_ticks>0;}
int game_artillery_effective(const Game*g,int i){const Artillery*a;int p;
    if(!g||i<0||i>=g->artillery_count)return 0;
    a=&g->artillery[i];p=minimum(a->power,a->health)-a->ion_bars;return p>0?p:0;
}
void game_flagship_crew_counts(const Game*g,int*counts){int i;
    for(i=0;i<GAME_FLAGSHIP_ROOMS;i++)counts[i]=0;
    for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++){const Crew*c=&g->enemy_crew[i];
        if(c->health>0&&c->ship==1&&c->room>=0&&c->room<GAME_FLAGSHIP_ROOMS)counts[c->room]++;}
}
int game_flagship_set_crew(Game*g,const int*counts){int i,k,slot=0,carried=0;
    if(!game_is_flagship(g)||!counts)return -1;
    for(i=0;i<GAME_FLAGSHIP_ROOMS;i++)if(counts[i]<0||counts[i]>4)return -1;
    for(i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++)if(g->enemy_crew[i].health>0||(g->enemy_crew[i].id&&!g->enemy_crew[i].death_processed))carried=i+1;
    memset(g->enemy_crew,0,GAME_MAX_ENEMY_CREW*sizeof g->enemy_crew[0]);
    for(i=0;i<g->enemy.room_count&&i<GAME_FLAGSHIP_ROOMS;i++){const GameRoom*r=&g->enemy.rooms[i];
        for(k=0;k<counts[r->id]&&k<r->w*r->h&&slot<GAME_MAX_ENEMY_CREW;k++){game_init_crew(g,1,slot,r->id);g->enemy_crew[slot].ship=1;slot++;}}
    g->enemy_crew_count=carried>slot?carried:slot;
    g->enemy.automated=slot==0;
    return slot;
}
static void flagship_weapon(WeaponState*w,int index){const FlagshipWeapon*d=&flagship_weapons[index];
    memset(w,0,sizeof *w);w->name=d->id;w->enabled=1;w->aim_room=w->target_room=-1;
    w->cooldown_ticks=d->cooldown_ms*GAME_HZ/1000;w->damage=d->damage;w->shots=d->shots>0?d->shots:1;w->shield_piercing=d->shield_piercing;
    w->ion_damage=d->ion;w->system_damage=d->damage;w->crew_damage=d->damage*15;w->fire_chance=d->fire_chance*10;w->breach_chance=d->breach_chance*10;
    w->missiles=!strcmp(d->type,"MISSILES");w->beam_length=!strcmp(d->type,"BEAM")?(d->beam_length*GAME_BEAM_UNITS+17)/35:0;
}
static void flagship_setup(Game*g,int stage){const FlagshipStage*st=&flagship_stages[stage];int i,j;
    memset(&g->enemy,0,sizeof g->enemy);memset(g->enemy_adjacency,0,sizeof g->enemy_adjacency);
    memset(g->artillery,0,sizeof g->artillery);memset(&g->enemy_cloak,0,sizeof g->enemy_cloak);g->enemy_cloak.room=-1;
    memset(&g->teleporter[1],0,sizeof g->teleporter[1]);memset(&g->enemy_drone,0,sizeof g->enemy_drone);g->enemy_drone.room=-1;
    subsystems_init(g,1);
    g->flagship_stage=stage;g->artillery_count=0;
    g->enemy.hull=g->enemy.max_hull=st->hull;g->enemy.reactor=st->reactor;g->enemy.oxygen=1000;
    g->enemy.room_count=minimum(st->room_count,GAME_MAX_ROOMS);
    for(i=0;i<GAME_SYSTEM_COUNT;i++)g->enemy.systems[i].room=-1;
    for(i=0;i<g->enemy.room_count;i++){const ImportedEnemyRoom*src=&st->rooms[i];GameRoom*r=&g->enemy.rooms[i];
        r->id=src->id;r->x=src->x;r->y=src->y;r->w=src->w;r->h=src->h;r->system=-1;r->oxygen=1000;}
    g->enemy_door_count=minimum(st->door_count,GAME_MAX_DOORS);
    for(i=0;i<g->enemy_door_count;i++){const ImportedEnemyDoor*src=&st->doors[i];GameDoor*d=&g->enemy_doors[i];int a,b;
        memset(d,0,sizeof *d);d->x=src->x;d->y=src->y;d->room_a=src->room_a;d->room_b=src->room_b;d->vertical=src->vertical;
        a=room_index(&g->enemy,d->room_a);b=room_index(&g->enemy,d->room_b);if(a>=0&&b>=0)g->enemy_adjacency[a][b]=g->enemy_adjacency[b][a]=1;}
    g->enemy_pilot_room=-1;g->enemy_pilot_health=0;
    for(i=0;i<st->system_count;i++){const FlagshipSystem*s=&st->systems[i];int sys=identify_system(s->name);
        if(!strcmp(s->name,"pilot")){g->enemy_pilot_room=s->room;g->enemy_pilot_health=s->power;}
        else if(sys>=0){GameSystemState*d=&g->enemy.systems[sys];d->room=s->room;d->capacity=d->health=d->power=s->power;j=room_index(&g->enemy,s->room);if(j>=0)g->enemy.rooms[j].system=sys;}
        else if(!strcmp(s->name,"doors")){Subsystem*d=&g->subsystems[1][SUB_DOORS];d->room=s->room;d->capacity=d->health=s->power;}
        else if(!strcmp(s->name,"cloaking")){EnemyCloak*k=&g->enemy_cloak;k->room=s->room;k->capacity=k->health=k->power=s->power;}
        else if(!strcmp(s->name,"teleporter")){Teleporter*t=&g->teleporter[1];t->installed=1;t->room=s->room;t->capacity=t->health=t->power=s->power;}
        else if(!strcmp(s->name,"drones")){EnemyDrone*d=&g->enemy_drone;d->room=s->room;d->capacity=d->health=d->power=s->power;}
        else if(!strcmp(s->name,"artillery")&&g->artillery_count<GAME_MAX_ARTILLERY&&s->weapon>=0&&s->weapon<FLAGSHIP_WEAPON_COUNT){
            Artillery*a=&g->artillery[g->artillery_count++];a->room=s->room;a->capacity=a->health=a->power=s->power;a->weapon=s->weapon;flagship_weapon(&a->w,s->weapon);}
        /* Sensors are absent on normal. */
    }
    memset(g->flagship_drones,0,sizeof g->flagship_drones);memset(g->surge,0,sizeof g->surge);g->flagship_drone_count=g->flagship_drone_parts=g->surge_ticks=g->surge_beams=0;
    if(g->enemy_drone.capacity>0){static const int list[GAME_FLAGSHIP_DRONES]={FDRONE_DEFENSE,FDRONE_COMBAT,FDRONE_BEAM,FDRONE_BOARDER};
        for(i=0;i<GAME_FLAGSHIP_DRONES;i++){g->flagship_drones[i].kind=list[i];g->flagship_drones[i].crew=-1;}
        g->flagship_drone_count=GAME_FLAGSHIP_DRONES;g->flagship_drone_parts=10;}
    /* Stages 2-3 carry the power-surge timer (20-30 s). */
    if(stage>=1){g->surge_ticks=20*GAME_HZ+(int)random_below(g,(unsigned)(10*GAME_HZ));g->surge_beams=(int)random_below(g,(unsigned)GAME_SURGE_DRONES+1u);}
    /* StartStage(3) powers the super shield (12 points) before the fight. */
    g->surge_count=0;g->super_shield=g->super_shield_max=stage==2?(g->flagship_authentic?12:8):0;g->surge_drones=g->flagship_authentic?GAME_SURGE_DRONES:4;
    g->enemy.shields=effective(&g->enemy,SYS_SHIELDS)/2;
    memset(&g->enemy_weapon,0,sizeof g->enemy_weapon);g->enemy_weapon.name="Artillery";g->enemy_weapon.power=1;g->enemy_weapon.cooldown_ticks=GAME_HZ;
    memset(&g->enemy_weapon_secondary,0,sizeof g->enemy_weapon_secondary);g->enemy_weapon_count=0;g->enemy_missiles=0;
    game_flagship_set_crew(g,flagship_default_crew);
    g->target_room=g->artillery_count?g->artillery[0].room:g->enemy.rooms[0].id;
    /* Players' measured charge times put the stage 1-2 laser and missile
     * artillery at level 3 (FTL wiki charge table), below the blueprint's 4. */
    if(stage<2)for(i=0;i<g->artillery_count;i++)if(g->artillery[i].capacity>3)g->artillery[i].capacity=g->artillery[i].health=g->artillery[i].power=3;
    /* DS finale tuning until the player catalog matches the original: keep every
     * mechanic, scale its numbers (2 shield layers, level-3 artillery charging
     * 1.5x slower, 4 surge drones, an 8-point super shield, stage-3 engines 3). */
    if(!g->flagship_authentic){
        GameSystemState*sh=&g->enemy.systems[SYS_SHIELDS];sh->capacity=sh->health=sh->power=4;g->enemy.shields=2;
        for(i=0;i<g->artillery_count;i++){Artillery*a=&g->artillery[i];if(a->capacity>3)a->capacity=a->health=a->power=3;a->w.cooldown_ticks=a->w.cooldown_ticks*3/2;}
        if(stage==2){GameSystemState*en=&g->enemy.systems[SYS_ENGINES];en->capacity=en->health=en->power=3;}
    }
}
/* ArtillerySystem::OnLoop aims through GetRandomTargettingPoint: uniform rooms,
 * distinct within one volley; beams start in a random room and sweep a random
 * direction (ChooseSecondTarget's 100 px random angle) for the weapon length. */
static int artillery_targets(Game*g,int*rooms,int count){int n=0,guard=0;
    if(count>g->player.room_count)count=g->player.room_count;
    if(count>GAME_MAX_VOLLEY)count=GAME_MAX_VOLLEY;
    while(n<count&&guard++<256){int room=g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id,i;
        for(i=0;i<n&&rooms[i]!=room;i++){}
        if(i==n)rooms[n++]=room;}
    return n;
}
static int beam_extend_on(const Ship*s,int x0,int y0,int dx,int dy,int length,int*x1,int*y1);
static int beam_extend(Game*g,int x0,int y0,int dx,int dy,int length,int*x1,int*y1){return beam_extend_on(&g->player,x0,y0,dx,dy,length,x1,y1);}
static int beam_extend_on(const Ship*s,int x0,int y0,int dx,int dy,int length,int*x1,int*y1){int minx=32767,miny=32767,maxx=0,maxy=0,i,k;long long d2=(long long)dx*dx+(long long)dy*dy,lo=0,hi=65536;
    if(!d2||length<1)return 0;
    for(i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];
        if(r->x*GAME_BEAM_UNITS<minx)minx=r->x*GAME_BEAM_UNITS;
        if(r->y*GAME_BEAM_UNITS<miny)miny=r->y*GAME_BEAM_UNITS;
        if((r->x+r->w)*GAME_BEAM_UNITS>maxx)maxx=(r->x+r->w)*GAME_BEAM_UNITS;
        if((r->y+r->h)*GAME_BEAM_UNITS>maxy)maxy=(r->y+r->h)*GAME_BEAM_UNITS;}
    while(lo<hi){long long mid=(lo+hi)/2;if(mid*mid<d2)lo=mid+1;else hi=mid;}
    *x1=x0+(int)((long long)dx*length/lo);*y1=y0+(int)((long long)dy*length/lo);
    for(k=16;k>0&&(*x1<minx||*x1>maxx||*y1<miny||*y1>maxy);k--){*x1=x0+(*x1-x0)*7/8;*y1=y0+(*y1-y0)*7/8;}
    if(*x1<minx)*x1=minx;
    if(*x1>maxx)*x1=maxx;
    if(*y1<miny)*y1=miny;
    if(*y1>maxy)*y1=maxy;
    return beam_valid_line(s,x0,y0,*x1,*y1);
}
/* V58 player Federation Artillery Beam. Charge time follows the flagship artillery
 * curve scaled so level 1 is the original 40 s (32/24/16 s at 2-4 power); it fires
 * by itself across the enemy from the selected target room (else a random room). */
int game_player_artillery_need(const Game*g){const Artillery*a=&g->player_artillery;int p=minimum(a->power,a->health)-a->ion_bars;
    if(a->capacity<=0||p<=0)return 0;
    return a->w.cooldown_ticks*(6-p)/5;}
int game_player_artillery_power(Game*g,int delta){Artillery*a=&g->player_artillery;
    if(a->capacity<=0||(delta!=1&&delta!=-1)){say(g,"No artillery installed.");return 0;}
    if(a->power+delta<0||a->power+delta>a->health){say(g,"Artillery power limit.");return 0;}
    if(delta>0&&game_used_power(g)>=game_reactor(g)){say(g,"No free reactor power.");return 0;}
    a->power+=delta;say(g,"Artillery power adjusted.");return 1;}
static void player_artillery_step(Game*g){static const int dir[16][2]={{1000,0},{924,383},{707,707},{383,924},{0,1000},{-383,924},{-707,707},{-924,383},{-1000,0},{-924,-383},{-707,-707},{-383,-924},{0,-1000},{383,-924},{707,-707},{924,-383}};
    Artillery*a=&g->player_artillery;int need=game_player_artillery_need(g),r=-1,i,x0,y0,x1,y1;
    if(a->ion_ticks>0&&--a->ion_ticks==0)a->ion_bars=0;
    if(a->capacity<=0||g->enemy.hull<=0||g->player.hull<=0||g->enemy.room_count<=0||g->enemy_departed)return;
    if(!need){a->charge=0;return;}
    if(a->charge<need){a->charge++;return;}
    if(game_enemy_cloaked(g))return;
    for(i=0;i<g->enemy.room_count;i++)if(g->enemy.rooms[i].id==g->target_room)r=i;
    if(r<0)r=random_below(g,(unsigned)g->enemy.room_count);
    {const GameRoom*room=&g->enemy.rooms[r];int d=random_below(g,16u);GameDamage damage=weapon_damage(&a->w);
     x0=(2*room->x+room->w)*GAME_BEAM_UNITS/2;y0=(2*room->y+room->h)*GAME_BEAM_UNITS/2;
     g->launch_origin=ORIGIN_ARTILLERY;
     if(beam_extend_on(&g->enemy,x0,y0,dir[d][0],dir[d][1],a->w.beam_length,&x1,&y1)&&game_launch_beam_piercing(g,0,x0,y0,x1,y1,&damage,a->w.shield_piercing)){a->charge=0;g->ach.used|=USED_ARTILLERY;say(g,"Artillery Beam fired.");}
     g->launch_origin=ORIGIN_NONE;}}
static void artillery_step(Game*g){int i;
    static const int dir[16][2]={{1000,0},{924,383},{707,707},{383,924},{0,1000},{-383,924},{-707,707},{-924,383},{-1000,0},{-924,-383},{-707,-707},{-383,-924},{0,-1000},{383,-924},{707,-707},{924,-383}};
    if((!game_is_flagship(g)&&!g->artillery_count)||g->enemy.hull<=0||g->player.hull<=0||!game_alive_crew(g))return;
    for(i=0;i<g->artillery_count;i++){Artillery*a=&g->artillery[i];WeaponState*w=&a->w;int p=game_artillery_effective(g,i),need=w->cooldown_ticks*(6-p);
        if(p>0){if(!w->pending_shots&&a->charge<need&&!game_player_cloaked(g))a->charge+=4;
            if(!w->pending_shots&&a->charge>=need&&game_artillery_manned(g,i)&&!(game_enemy_cloaked(g)&&!g->enemy_cloak.fire_while_cloaked)){
                a->charge=0;w->shot_ticks=0;w->queued_beam_length=0;w->queued_target_count=0;
                if(w->beam_length){int r=random_below(g,(unsigned)g->player.room_count),d=random_below(g,16u),x1,y1;const GameRoom*room=&g->player.rooms[r];
                    w->queued_beam_x0=(2*room->x+room->w)*GAME_BEAM_UNITS/2;w->queued_beam_y0=(2*room->y+room->h)*GAME_BEAM_UNITS/2;
                    if(beam_extend(g,w->queued_beam_x0,w->queued_beam_y0,dir[d][0],dir[d][1],w->beam_length,&x1,&y1)){w->queued_beam_x1=x1;w->queued_beam_y1=y1;w->queued_beam_length=w->beam_length;w->pending_shots=1;}}
                else w->pending_shots=w->queued_target_count=artillery_targets(g,w->queued_targets,w->shots);
                w->queued_damage=weapon_damage(w);w->queued_ion_damage=w->ion_damage;
            }
        }else a->charge=0;
        if(w->pending_shots>0&&--w->shot_ticks<=0){int ok;g->launch_mount=5+i; /* Artillery uses the mounts after the four weapon slots. */
            if(w->queued_beam_length)ok=game_launch_beam(g,1,w->queued_beam_x0,w->queued_beam_y0,w->queued_beam_x1,w->queued_beam_y1,&w->queued_damage);
            else {w->target_room=w->queued_targets[w->queued_target_count-w->pending_shots];
                ok=w->queued_ion_damage?game_launch_ion(g,1,w->target_room,w->queued_ion_damage):game_launch_effect(g,1,w->target_room,w->missiles?EFFECT_MISSILE:EFFECT_LASER,&w->queued_damage,w->shield_piercing);}
            g->launch_mount=0;if(ok){w->pending_shots--;w->shot_ticks=6;}
        }
    }
}
/* CombatAI::OnLoop cloaks whenever possible and randomly decides whether to
 * fire while cloaked. 5 s per effective bar; the end applies 4 ion bars. */
static void cloak_step(Game*g){EnemyCloak*k=&g->enemy_cloak;int p;
    if(k->capacity<=0||g->enemy.hull<=0||g->enemy_departed)return;
    p=minimum(k->power,k->health)-k->ion_bars;
    if(k->active_ticks>0){if(p<=0)k->active_ticks=0;else if(--k->active_ticks==0)ion_stack(&k->ion_bars,&k->ion_ticks,4);return;}
    if(p>0&&!k->ion_bars&&g->outcome==GAME_PLAYING&&g->player.hull>0){k->active_ticks=5*GAME_HZ*p;k->fire_while_cloaked=(int)(random_below(g,2u));}
}
/* V43 stage-2 drone system: each drone needs two effective bars, powered in
 * blueprint order; deployment spends a drone part. Combat and beam drones use
 * the port's authored 10 s firing interval; the defense drone intercepts
 * player missiles once per second; the boss boarding drone flies in, breaches
 * a random room and fights as a 150 HP intruder (Rock stats stand in). */
int game_flagship_drones_active(const Game*g){int p;
    if(!game_is_flagship(g)||g->enemy_drone.capacity<=0)return 0;
    p=minimum(g->enemy_drone.power,g->enemy_drone.health)-g->enemy_drone.ion_bars;
    p=p>0?p/2:0;return p<g->flagship_drone_count?p:g->flagship_drone_count;
}
static int short_beam(Game*g,int length){
    static const int dir[8][2]={{1000,0},{707,707},{0,1000},{-707,707},{-1000,0},{-707,-707},{0,-1000},{707,-707}};
    int r=random_below(g,(unsigned)g->player.room_count),d=random_below(g,8u),x0,y0,x1,y1;const GameRoom*room=&g->player.rooms[r];
    GameDamage damage={1,1,15,10,0,0,0,0};
    x0=(2*room->x+room->w)*GAME_BEAM_UNITS/2;y0=(2*room->y+room->h)*GAME_BEAM_UNITS/2;
    if(!beam_extend(g,x0,y0,dir[d][0],dir[d][1],length,&x1,&y1))return 0;
    return game_launch_beam(g,1,x0,y0,x1,y1,&damage);
}
static int drone_laser(Game*g){GameDamage damage={1,1,15,10,0,0,0,0};
    return game_launch_effect(g,1,g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id,EFFECT_LASER,&damage,0);
}
static int boarding_drone_lands(Game*g){int slot=-1,i,room;
    for(i=0;i<GAME_MAX_ENEMY_CREW&&slot<0;i++)if(i>=g->enemy_crew_count||(g->enemy_crew[i].health<=0&&(!g->enemy_crew[i].id||g->enemy_crew[i].death_processed)))slot=i;
    room=g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id;
    breach_add(&g->player,room,1);
    if(slot<0)return -1;
    game_init_crew(g,1,slot,room);game_set_crew_species(g,1,slot,SPEC_ROCK);
    snprintf(g->enemy_crew[slot].name,sizeof g->enemy_crew[slot].name,"Drone");
    g->enemy_crew[slot].ship=0;g->enemy_crew[slot].station=room;
    if(g->enemy_crew_count<=slot)g->enemy_crew_count=slot+1;
    say(g,"Boarding drone breached the hull.");
    return slot;
}
static void flagship_drones_step(Game*g){int i,active=game_flagship_drones_active(g);
    if(!game_is_flagship(g))return;
    /* A destroyed stage takes its drone system, and any landed boarders, with it. */
    if(g->enemy.hull<=0){for(i=0;i<g->flagship_drone_count;i++){FlagshipDrone*d=&g->flagship_drones[i];if(d->crew>=0&&g->enemy_crew[d->crew].health>0)g->enemy_crew[d->crew].health=0;d->crew=-1;d->deployed=0;}return;}
    if(g->player.hull<=0)return;
    for(i=0;i<g->flagship_drone_count;i++){FlagshipDrone*d=&g->flagship_drones[i];
        /* A landed boarder stays tied to its slot; losing drone power shuts it down. */
        if(d->kind==FDRONE_BOARDER&&d->crew>=0){Crew*c=&g->enemy_crew[d->crew];
            if(i>=active&&c->health>0){c->health=0;say(g,"Boarding drone shut down.");}
            if(c->health<=0){d->crew=-1;d->deployed=0;d->charge=10*GAME_HZ;} /* Destroyed drones redeploy after 10 s. */
            continue;}
        if(i>=active){if(d->deployed)d->charge=0;continue;}
        if(!d->deployed){if(d->charge>0){d->charge--;continue;}if(g->flagship_drone_parts<=0)continue;g->flagship_drone_parts--;d->deployed=1;d->charge=0;d->crew=-1;d->flight=d->kind==FDRONE_BOARDER?3*GAME_HZ:0;continue;}
        if(d->kind==FDRONE_BOARDER){if(d->flight>0&&--d->flight==0){d->crew=boarding_drone_lands(g);if(d->crew<0)d->deployed=0;}continue;}
        if(d->kind==FDRONE_DEFENSE){int j,best=-1,left=0;
            if(d->charge<GAME_HZ){d->charge++;continue;}
            for(j=0;j<GAME_MAX_EFFECTS;j++){GameEffect*e=&g->effects[j];int flight=e->duration-e->age;
                if(!e->active||e->result!=EFFECT_PENDING||e->side!=0||e->kind!=EFFECT_MISSILE||flight<=0)continue;
                if(best<0||flight<left){best=j;left=flight;}}
            if(best>=0){g->effects[best].result=EFFECT_INTERCEPT;g->effects[best].age=g->effects[best].duration;d->charge=0;say(g,"Flagship defense drone shot down a missile.");}
            continue;}
        if(++d->charge>=10*GAME_HZ&&(d->kind==FDRONE_BEAM?short_beam(g,(20*GAME_BEAM_UNITS+17)/35):drone_laser(g)))d->charge=0;
    }
}
/* BossShip power surge: every 20-30 s with a warning 5 s ahead. Stage 2 sends
 * six extra drones (beam/combat split fixed per encounter), two shots each. */
static void surge_step(Game*g){int i;
    if(!game_is_flagship(g)||g->enemy.hull<=0||g->player.hull<=0)return;
    /* Stage 3 (BossShip::OnLoop): seven simultaneous surge lasers (LASER_HEAVY_1
     * hard-coded to 1 damage); every fourth surge restores the super shield. */
    if(g->flagship_stage==2){
        if(g->surge_ticks>0&&--g->surge_ticks==0){
            if(g->surge_count%4==3){g->super_shield=g->super_shield_max;say(g,"Flagship power surge: super shield restored.");}
            else{GameDamage d={1,1,15,flagship_surge_laser.fire_chance*10,flagship_surge_laser.breach_chance*10,0,0,0};int fired=0;
                for(i=0;i<7;i++)fired+=game_launch_effect(g,1,g->player.rooms[random_below(g,(unsigned)g->player.room_count)].id,EFFECT_LASER,&d,0);
                (void)fired;say(g,"Flagship power surge: laser barrage!");}
            g->surge_count++;g->surge_ticks=20*GAME_HZ+(int)random_below(g,(unsigned)(10*GAME_HZ));
        }
        return;
    }
    if(g->flagship_stage!=1)return;
    for(i=0;i<GAME_SURGE_DRONES;i++){SurgeDrone*s=&g->surge[i];
        if(s->shots>0&&--s->next_tick<=0&&(s->kind==FDRONE_BEAM?short_beam(g,(20*GAME_BEAM_UNITS+17)/35):drone_laser(g))){s->shots--;s->next_tick=2*GAME_HZ+(int)random_below(g,(unsigned)GAME_HZ);}}
    if(g->surge_ticks>0&&--g->surge_ticks==0){
        for(i=0;i<g->surge_drones&&i<GAME_SURGE_DRONES;i++){SurgeDrone*s=&g->surge[i];s->kind=i<g->surge_beams?FDRONE_BEAM:FDRONE_COMBAT;s->shots=2;s->next_tick=2*GAME_HZ+(int)random_below(g,(unsigned)(2*GAME_HZ));}
        g->surge_ticks=20*GAME_HZ+(int)random_below(g,(unsigned)(10*GAME_HZ));
        say(g,"Flagship power surge: drone swarm launched.");
    }
}
int game_enemy_super_shield(const Game*g){return game_is_flagship(g)&&g->enemy.hull>0?g->super_shield:0;}
int game_surge_warning(const Game*g){return game_is_flagship(g)&&g->surge_ticks>0&&g->surge_ticks<=5*GAME_HZ?(g->surge_ticks+GAME_HZ-1)/GAME_HZ:0;}
static void ion_room(Game*g,Ship*s,int room,int amount){int i,side=s==&g->enemy;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)if(s->systems[i].capacity>0&&s->systems[i].room==room)ion_stack(&s->systems[i].ion_bars,&s->systems[i].ion_ticks,amount);
    for(i=0;i<SUB_COUNT;i++)if(g->subsystems[side][i].capacity>0&&g->subsystems[side][i].room==room)ion_stack(&g->subsystems[side][i].ion_bars,&g->subsystems[side][i].ion_ticks,amount);
    if(g->teleporter[side].installed&&g->teleporter[side].room==room)ion_stack(&g->teleporter[side].ion_bars,&g->teleporter[side].ion_ticks,amount);
    if(!side&&g->drones.installed&&g->drones.room==room)ion_stack(&g->drones.ion_bars,&g->drones.ion_ticks,amount);
    if(side&&g->enemy_drone.capacity>0&&g->enemy_drone.room==room)ion_stack(&g->enemy_drone.ion_bars,&g->enemy_drone.ion_ticks,amount);
    if(side){for(i=0;i<g->artillery_count;i++)if(g->artillery[i].room==room)ion_stack(&g->artillery[i].ion_bars,&g->artillery[i].ion_ticks,amount);
        if(g->enemy_cloak.capacity>0&&g->enemy_cloak.room==room)ion_stack(&g->enemy_cloak.ion_bars,&g->enemy_cloak.ion_ticks,amount);}
    else if(g->player_cloak.capacity>0&&g->player_cloak.room==room)ion_stack(&g->player_cloak.ion_bars,&g->player_cloak.ion_ticks,amount);
    else if(g->player_artillery.capacity>0&&g->player_artillery.room==room)ion_stack(&g->player_artillery.ion_bars,&g->player_artillery.ion_ticks,amount);
    if(room==(side?g->enemy_pilot_room:g->pilot_room))g->pilot_ion_ticks[side]=minimum(GAME_ION_MAX_TICKS,g->pilot_ion_ticks[side]+amount*GAME_ION_TICKS);
    s->shields=minimum(s->shields,game_shield_capacity(s));if(!game_shield_capacity(s))s->shield_ticks=0;
}
static int room_has_system(const Game*g,const Ship*ship,int room){int i,side=ship==&g->enemy,r=room_index(ship,room);
    if(r>=0&&ship->rooms[r].system>=0)return 1;
    if(room==(side?g->enemy_pilot_room:g->pilot_room))return 1;
    for(i=0;i<SUB_COUNT;i++)if(g->subsystems[side][i].capacity>0&&g->subsystems[side][i].room==room)return 1;
    if(g->teleporter[side].installed&&g->teleporter[side].room==room)return 1;
    if(!side&&g->drones.installed&&g->drones.room==room)return 1;
    if(side&&g->enemy_drone.capacity>0&&g->enemy_drone.room==room)return 1;
    if(side){for(i=0;i<g->artillery_count;i++)if(g->artillery[i].room==room)return 1;if(g->enemy_cloak.capacity>0&&g->enemy_cloak.room==room)return 1;}
    return 0;
}
static int room_damage(Game*g,Ship*ship,int room,const GameDamage*input){int i,r=room_index(ship,room),hull_before=ship->hull;const char*damaged=NULL;
    GameDamage adjusted=*input;const GameDamage*damage=&adjusted;
    /* V55 player augments: Rock Plating may stop hull damage, Titanium Casing system
     * damage; Crystal Vengeance may answer a hit with a shard. */
    if(ship==&g->player){if(adjusted.hull>0&&aug_roll(g,AUG_ROCK_ARMOR))adjusted.hull=0;if(adjusted.system>0&&aug_roll(g,AUG_SYSTEM_CASING))adjusted.system=0;
        if((input->hull>0||input->system>0)&&g->enemy.hull>0&&!g->enemy_departed&&aug_roll(g,AUG_CRYSTAL_SHARDS)){GameDamage shard={1,1,15,0,0,0,0,0};g->launch_origin=ORIGIN_SHARD;game_launch_effect(g,0,g->enemy.rooms[random_below(g,(unsigned)g->enemy.room_count)].id,EFFECT_MISSILE,&shard,1);g->launch_origin=ORIGIN_NONE;}}
    if(r>=0&&(damage->hull>0||damage->system>0||damage->crew>0)){ship->rooms[r].hit_ticks=GAME_ROOM_HIT_TICKS;ship->rooms[r].hit_damage=damage->hull>0?damage->hull:damage->system;}
    /* Report only the observed hit, never hidden remaining system bars. */
    if(ship==&g->enemy&&damage->system>0)for(i=0;i<SUB_COUNT;i++)if(g->subsystems[1][i].room==room&&g->subsystems[1][i].health>0)damaged="Subsystem";
    subsystem_damage(g,ship==&g->enemy,room,damage->system);
    /* Original hullBust: double hull damage against a room without a system. */
    ship->hull-=damage->hull*(damage->hull_bust&&!room_has_system(g,ship,room)?2:1);if(ship->hull<0) ship->hull=0;
    if(damage->lockdown&&r>=0)ship->rooms[r].lockdown_ticks=CRYSTAL_LOCKDOWN_TICKS;
    if(r>=0 && ship->rooms[r].system>=0) {
        GameSystemState *s=&ship->systems[ship->rooms[r].system];
        if(ship==&g->enemy&&damage->system>0&&s->health>0)damaged=game_system_name(ship->rooms[r].system);
        s->health-=damage->system;if(s->health<0) s->health=0;
        s->power=minimum(s->power,s->health);
    }
    ship->shields=minimum(ship->shields,game_shield_capacity(ship));
    if(!game_shield_capacity(ship))ship->shield_ticks=0;
    if(damage->fire_chance>0 && random100(g)<(unsigned)damage->fire_chance)fire_ignite(ship,room,1);
    if(damage->breach_chance>0 && random100(g)<(unsigned)damage->breach_chance)breach_add(ship,room,1);
    if(ship==&g->player && g->drones.installed && room==g->drones.room){
        g->drones.health-=damage->system;if(g->drones.health<0)g->drones.health=0;
        g->drones.power=minimum(g->drones.power,g->drones.health);
    }
    if(ship==&g->enemy && room==g->enemy_drone.room){
        if(damage->system>0&&g->enemy_drone.health>0)damaged="Drones";
        g->enemy_drone.health-=damage->system;if(g->enemy_drone.health<0)g->enemy_drone.health=0;
        g->enemy_drone.power=minimum(g->enemy_drone.power,g->enemy_drone.health);
    }
    if(ship==&g->enemy){for(i=0;i<g->artillery_count;i++){Artillery*a=&g->artillery[i];if(a->room!=room)continue;
            if(damage->system>0&&a->health>0)damaged="Artillery";
            a->health-=damage->system;if(a->health<0)a->health=0;
            a->power=minimum(a->power,a->health);}
        if(g->enemy_cloak.capacity>0&&g->enemy_cloak.room==room){EnemyCloak*k=&g->enemy_cloak;
            if(damage->system>0&&k->health>0)damaged="Cloaking";
            k->health-=damage->system;if(k->health<0)k->health=0;
            k->power=minimum(k->power,k->health);}}
    if(ship==&g->player&&g->player_artillery.capacity>0&&g->player_artillery.room==room){Artillery*a=&g->player_artillery;a->health-=damage->system;if(a->health<0)a->health=0;a->power=minimum(a->power,a->health);}
    if(ship==&g->player&&g->player_cloak.capacity>0&&g->player_cloak.room==room){EnemyCloak*k=&g->player_cloak;k->health-=damage->system;if(k->health<0)k->health=0;k->power=minimum(k->power,k->health);if(k->active_ticks&&minimum(k->power,k->health)<=k->ion_bars)k->active_ticks=0;}
    {Teleporter*t=&g->teleporter[ship==&g->enemy];if(t->installed&&t->room==room){if(ship==&g->enemy&&damage->system>0&&t->health>0)damaged="Teleporter";t->health-=damage->system;if(t->health<0)t->health=0;t->power=minimum(t->power,t->health);}}
    if(damage->system>0&&ship==&g->player && room==g->pilot_room)g->pilot_health=0;
    if(damage->system>0&&ship==&g->enemy&&room==g->enemy_pilot_room){if(g->enemy_pilot_health>0)damaged="Piloting";g->enemy_pilot_health=0;}
    {int f,side=ship==&g->enemy;for(f=0;f<2;f++)for(i=0;i<(f?g->enemy_crew_count:g->crew_count);i++){
        Crew*c=f?&g->enemy_crew[i]:&g->crew[i];if(c->ship==side&&c->room==room&&c->health>0){c->health-=damage->crew;if(c->health<0)c->health=0;if(c->health>species_max_health(c->species))c->health=species_max_health(c->species);}}}
    if(ship==&g->enemy){if(damaged)snprintf(g->message,sizeof g->message,"%s hit: system damaged.",damaged);else if(ship->hull<hull_before)say(g,"Enemy hull hit.");else say(g,"Enemy room hit.");}
    return EFFECT_HIT;
}
static int hit(Game *g,Ship *ship,int room,const GameDamage *damage,int piercing,int ion,int bomb,int friendly) {
    int evade=ship==&g->player?game_evasion(g):effective(ship,SYS_ENGINES)*5;
    if(ship==&g->enemy&&!ship->automated){if(!g->enemy_pilot_health||g->pilot_ion_ticks[1]>0||!enemy_manned(g,g->enemy_pilot_room))evade=0;else evade=minimum(45,evade+5+(enemy_manned(g,ship->systems[SYS_ENGINES].room)?5:0));}
    /* An AI-controlled flagship treats undamaged systems as manned. */
    else if(ship==&g->enemy&&game_is_flagship(g)&&g->enemy_pilot_health&&!g->pilot_ion_ticks[1])evade=minimum(45,evade+10);
    if(ship==&g->enemy&&game_enemy_cloaked(g))evade+=60;
    if(ship==&g->player&&game_player_cloaked(g))evade+=60;
    if(!friendly&&random100(g)<(unsigned int)evade) {say(g,ship==&g->enemy?"Enemy evaded the shot.":"Kestrel evaded the shot.");return EFFECT_MISS;}
    /* V55 player augments: Zoltan Shield points absorb first; Reverse Ion Field may cancel ions. */
    if(ship==&g->player&&!friendly&&g->player_super_shield>0){g->player_super_shield-=ion>0?ion:damage->hull>0?damage->hull:1;if(g->player_super_shield<0)g->player_super_shield=0;say(g,"Zoltan shield absorbed the shot.");return EFFECT_SHIELD;}
    if(ship==&g->player&&ion>0&&aug_roll(g,AUG_ION_ARMOR)){say(g,"Reverse Ion Field negated the ion blast.");return EFFECT_SHIELD;}
    /* V44 Zoltan super shield: absorbs every projectile, ion and bomb, losing its damage in points. */
    if(ship==&g->enemy&&g->super_shield>0&&!friendly){g->super_shield-=ion>0?ion:damage->hull>0?damage->hull:1;if(g->super_shield<0)g->super_shield=0;say(g,g->super_shield?"Flagship super shield absorbed the shot.":"Flagship super shield is down!");return EFFECT_SHIELD;}
    if(ion>0&&bomb){ion_room(g,ship,room,ion);return room_damage(g,ship,room,damage);}
    if(ion>0){if(ship->shields>0){ion_room(g,ship,ship->systems[SYS_SHIELDS].room,ion);return EFFECT_SHIELD;}ion_room(g,ship,room,ion);return EFFECT_HIT;}
    if(!bomb&&ship->shields>piercing) {ship->shields--;ship->shield_ticks=0;say(g,ship==&g->enemy?"Enemy shield absorbed the shot.":"Kestrel shield absorbed the shot.");return EFFECT_SHIELD;}
    return room_damage(g,ship,room,damage);
}
int game_launch_effect(Game *g,int side,int room,int kind,const GameDamage *damage,int piercing) {
    int i;
    if(!damage||side<0||side>1||piercing<0||(kind!=EFFECT_LASER&&kind!=EFFECT_MISSILE)||room_index(side?&g->player:&g->enemy,room)<0||damage->hull<0||damage->system<0||damage->crew<0||damage->fire_chance<0||damage->fire_chance>100||damage->breach_chance<0||damage->breach_chance>100)return 0;
    for(i=0;i<GAME_MAX_EFFECTS;i++) if(!g->effects[i].active) {
        GameEffect *e=&g->effects[i];
        memset(e,0,sizeof(*e));e->origin=side?0:g->launch_origin;e->mount=g->launch_mount;
        e->active=1;e->duration=12;e->side=side;e->room=room;
        e->kind=kind;e->damage=damage->hull;e->piercing=piercing;e->fire_chance=damage->fire_chance;
        e->system_damage=damage->system;e->crew_damage=damage->crew;e->breach_chance=damage->breach_chance;e->hull_bust=damage->hull_bust;e->lockdown=damage->lockdown;
        return 1;
    }
    /* Leave the shot queued when the bounded effect pool is full. */
    return 0;
}
static int launch_bomb(Game*g,int side,int room,const GameDamage*damage,int friendly){int i;
    if(side<0||side>1||!damage||damage->hull!=0||damage->system<0||damage->crew<-1000||damage->ion<0||damage->ion>GAME_ION_MAX_BARS||damage->fire_chance<0||damage->fire_chance>100||damage->breach_chance<0||damage->breach_chance>100||room_index(side?&g->player:&g->enemy,room)<0)return 0;
    for(i=0;i<GAME_MAX_EFFECTS;i++)if(!g->effects[i].active){GameEffect*e=&g->effects[i];memset(e,0,sizeof *e);e->origin=side?0:g->launch_origin;e->mount=g->launch_mount;e->active=1;e->duration=12;e->side=side;e->room=room;e->kind=EFFECT_BOMB;e->bomb_friendly=friendly;
        e->system_damage=damage->system;e->crew_damage=damage->crew;e->fire_chance=damage->fire_chance;e->breach_chance=damage->breach_chance;e->lockdown=damage->lockdown;e->ion_damage=damage->ion;return 1;}
    return 0;
}
int game_launch_bomb(Game*g,int side,int room,const GameDamage*damage){return launch_bomb(g,side,room,damage,0);}
int game_launch_ion(Game*g,int side,int room,int amount){int i;
    if(side<0||side>1||amount<1||amount>GAME_ION_MAX_BARS||room_index(side?&g->player:&g->enemy,room)<0)return 0;
    for(i=0;i<GAME_MAX_EFFECTS;i++)if(!g->effects[i].active){GameEffect*e=&g->effects[i];memset(e,0,sizeof *e);e->origin=side?0:g->launch_origin;e->mount=g->launch_mount;e->active=1;e->duration=12;e->side=side;e->room=room;e->kind=EFFECT_ION;e->ion_damage=amount;return 1;}
    return 0;
}
static void ion_recover(Game*g,int side){int i;Ship*s=side?&g->enemy:&g->player;
    for(i=0;i<GAME_SYSTEM_COUNT;i++){GameSystemState*d=&s->systems[i];if(d->ion_ticks>0&&--d->ion_ticks==0)d->ion_bars=0;}
    for(i=0;i<SUB_COUNT;i++){Subsystem*d=&g->subsystems[side][i];if(d->ion_ticks>0&&--d->ion_ticks==0)d->ion_bars=0;}
    {Teleporter*d=&g->teleporter[side];if(d->ion_ticks>0&&--d->ion_ticks==0)d->ion_bars=0;}
    if(!side&&g->drones.ion_ticks>0&&--g->drones.ion_ticks==0)g->drones.ion_bars=0;
    if(!side&&g->player_cloak.ion_ticks>0&&--g->player_cloak.ion_ticks==0)g->player_cloak.ion_bars=0;
    if(side&&g->enemy_drone.ion_ticks>0&&--g->enemy_drone.ion_ticks==0)g->enemy_drone.ion_bars=0;
    if(side){for(i=0;i<g->artillery_count;i++){Artillery*a=&g->artillery[i];if(a->ion_ticks>0&&--a->ion_ticks==0)a->ion_bars=0;}
        if(g->enemy_cloak.ion_ticks>0&&--g->enemy_cloak.ion_ticks==0)g->enemy_cloak.ion_bars=0;}
    if(g->pilot_ion_ticks[side]>0)g->pilot_ion_ticks[side]--;
}
int game_set_beam_target(Game*g,int weapon,int x0,int y0,int x1,int y1){WeaponState*w;
    if(weapon<0||weapon>=g->weapon_count)return 0;
    w=&g->weapons[weapon];
    if(!w->beam_length||!beam_valid_line(&g->enemy,x0,y0,x1,y1)||!beam_clamp_line(x0,y0,&x1,&y1,w->beam_length)||!beam_valid_line(&g->enemy,x0,y0,x1,y1)){say(g,"Choose a beam line crossing the ship.");return 0;}
    w->beam_x0=x0;w->beam_y0=y0;w->beam_x1=x1;w->beam_y1=y1;if(w->aim_room==GAME_AIM_HOLD){w->aim_room=-1;}w->beam_target_valid=1;say(g,"Beam line set. Fire when ready.");return 1;
}
int game_launch_beam(Game*g,int side,int x0,int y0,int x1,int y1,const GameDamage*damage){return game_launch_beam_piercing(g,side,x0,y0,x1,y1,damage,0);}
int game_launch_beam_piercing(Game*g,int side,int x0,int y0,int x1,int y1,const GameDamage*damage,int piercing){int i;
    if(side<0||side>1||!damage||damage->hull<0||damage->system<0||damage->crew<0||damage->fire_chance<0||damage->fire_chance>100||damage->breach_chance<0||damage->breach_chance>100||!beam_valid_line(side?&g->player:&g->enemy,x0,y0,x1,y1))return 0;
    for(i=0;i<GAME_MAX_EFFECTS;i++)if(!g->effects[i].active){GameEffect*e=&g->effects[i];memset(e,0,sizeof *e);e->origin=side?0:g->launch_origin;e->mount=g->launch_mount;e->active=1;e->duration=12;e->side=side;e->kind=EFFECT_BEAM;e->room=-1;
        e->beam_x0=x0;e->beam_y0=y0;e->beam_x1=x1;e->beam_y1=y1;e->damage=damage->hull;e->system_damage=damage->system;e->crew_damage=damage->crew;e->fire_chance=damage->fire_chance;e->breach_chance=damage->breach_chance;e->hull_bust=damage->hull_bust;e->piercing=piercing<0?0:piercing;return 1;}
    return 0;
}
static int beam_hit(Game*g,Ship*s,const GameEffect*e){int i,layers=s->shields>e->piercing?s->shields-e->piercing:0,hit=0; /* V58: artillery pierces shields. */
    if(s==&g->enemy&&g->super_shield>0){g->super_shield-=e->damage>0?e->damage:1;if(g->super_shield<0)g->super_shield=0;return EFFECT_SHIELD;}
    if(layers>0&&e->damage<=layers)return EFFECT_SHIELD; /* Original: a beam needs damage left after shields (fire/anti-bio beams need shields down). */
    GameDamage damage={e->damage-layers,e->system_damage>layers?e->system_damage-layers:0,e->crew_damage>layers*15?e->crew_damage-layers*15:0,e->fire_chance,e->breach_chance,e->hull_bust,0,0};
    if(!damage.hull&&!damage.system&&!damage.crew)return EFFECT_SHIELD;
    for(i=0;i<s->room_count;i++)if(beam_intersects_room(&s->rooms[i],e->beam_x0,e->beam_y0,e->beam_x1,e->beam_y1)){room_damage(g,s,s->rooms[i].id,&damage);hit=1;}
    return hit?EFFECT_HIT:EFFECT_MISS;
}
static void effects_step(Game *g) {
    int i;
    for(i=0;i<g->player.room_count;i++){if(g->player.rooms[i].hit_ticks>0)g->player.rooms[i].hit_ticks--;}
    for(i=0;i<g->enemy.room_count;i++){if(g->enemy.rooms[i].hit_ticks>0)g->enemy.rooms[i].hit_ticks--;}
    for(i=0;i<GAME_MAX_EFFECTS;i++) {
        GameEffect *e=&g->effects[i];
        if(!e->active) continue;
        e->age++;
        if(e->age>=e->duration && e->result==EFFECT_PENDING) {
            Ship *target=e->side?&g->player:&g->enemy;
            GameDamage damage={e->damage,e->system_damage,e->crew_damage,e->fire_chance,e->breach_chance,e->hull_bust,e->lockdown,0};
            int hull_before=g->enemy.hull,alive_before=achievements_enemy_alive(g);
            e->result=e->kind==EFFECT_BEAM?beam_hit(g,target,e):hit(g,target,e->room,&damage,e->piercing,e->ion_damage,e->kind==EFFECT_BOMB,e->bomb_friendly);
            achievements_effect_resolved(g,e,hull_before,alive_before);
        }
        if(e->age>=e->duration+GAME_EFFECT_IMPACT_TICKS) e->active=0;
    }
}
static void ship_step(Ship *s) {
    int cap=game_shield_capacity(s);
    if(s->shields>cap) s->shields=cap;
    if(s->shields<cap) {if(++s->shield_ticks>=GAME_SHIELD_RECHARGE_TICKS) {s->shields++;s->shield_ticks=0;}}
    else s->shield_ticks=0;
}
/* Provisional atmosphere: vacuum-connected rooms lose 12 units/tick; sealed
 * rooms gain 2 with O2 power or lose 1 without. Exchange conserves area-weighted
 * oxygen, up to integer rounding. Rates are chosen, not recovered FTL rules. */
static void update_exposure_side(Game *g,int side) {
    Ship *s=side?&g->enemy:&g->player;int count;GameDoor*doors=game_ship_doors(g,side,&count);
    int q[GAME_MAX_ROOMS],head=0,tail=0,i;
    for(i=0;i<s->room_count;i++){s->rooms[i].exposed=s->rooms[i].breaches>0;if(s->rooms[i].exposed)q[tail++]=i;}
    for(i=0;i<count;i++) {
        GameDoor *d=&doors[i];int a,b;
        if(!d->open)continue;
        a=room_index(s,d->room_a);b=room_index(s,d->room_b);
        if((a>=0) != (b>=0)) {
            int inside=a>=0?a:b;
            if(!s->rooms[inside].exposed){s->rooms[inside].exposed=1;q[tail++]=inside;}
        }
    }
    while(head<tail) {
        int index=q[head++],id=s->rooms[index].id;
        for(i=0;i<count;i++) {
            const GameDoor *d=&doors[i];int other=-1;
            if(!d->open)continue;
            if(d->room_a==id)other=room_index(s,d->room_b);
            else if(d->room_b==id)other=room_index(s,d->room_a);
            if(other>=0 && !s->rooms[other].exposed){s->rooms[other].exposed=1;q[tail++]=other;}
        }
    }
}
static void update_exposure(Game*g){update_exposure_side(g,0);}
static void atmosphere_ship_step(Game *g,int side) {
    Ship *s=side?&g->enemy:&g->player;int count;GameDoor*doors=game_ship_doors(g,side,&count);
    int delta[GAME_MAX_ROOMS]={0},i,area_sum=0,oxygen_sum=0;
    int powered=effective(s,SYS_OXYGEN)>0;
    if(s->automated)return;
    for(i=0;i<count;i++) {
        GameDoor *d=&doors[i];
        if(d->transit_ticks>0 && --d->transit_ticks==0)d->open=0;
    }
    update_exposure_side(g,side);
    for(i=0;i<count;i++) {
        const GameDoor *d=&doors[i];int a,b,aa,ab,exchange;
        if(!d->open)continue;
        a=room_index(s,d->room_a);b=room_index(s,d->room_b);
        if(a<0 || b<0)continue;
        aa=s->rooms[a].w*s->rooms[a].h;ab=s->rooms[b].w*s->rooms[b].h;
        exchange=(s->rooms[a].oxygen-s->rooms[b].oxygen)/(4*(aa+ab));
        delta[a]-=exchange*ab;delta[b]+=exchange*aa;
    }
    for(i=0;i<s->room_count;i++) {
        GameRoom *r=&s->rooms[i];int area=r->w*r->h;
        r->oxygen+=delta[i]+(r->breaches?-12*r->breaches:r->exposed?-12:(powered?2:-(g->ticks&1))); /* V55: unpowered rooms lose 1.5%/s. */
        if(r->oxygen<0)r->oxygen=0;
        if(r->oxygen>1000)r->oxygen=1000;
        area_sum+=area;oxygen_sum+=r->oxygen*area;
    }
    s->oxygen=area_sum?oxygen_sum/area_sum:0;
 }
static void atmosphere_step(Game*g,int service){atmosphere_ship_step(g,0);if(!service||(g->enemy.hull>0&&game_away_crew(g)))atmosphere_ship_step(g,1);}
/* FTL task priorities follow the xftl reverse-engineering notes. This bounded
 * once-per-second greedy scheduler is a DS approximation, not the original AI. */
static int enemy_room_safe(const GameRoom*r,const Crew*c){
    int max=species_max_health(c->species);
    return !r->exposed && !(c->health*4<max && r->oxygen<100) &&
        !(c->species!=SPEC_ROCK && (r->fires>=4 || (r->fires && c->health*5<max)));
}
static int enemy_system_priority(int system,int healing){
    return system==SYS_SHIELDS?0:system==SYS_WEAPONS?1:system==SYS_MEDBAY?(healing?1:2):system==SYS_OXYGEN?3:system==SYS_ENGINES?4:8;
}
/* CrewAI::CheckForProblems (FTL 1.6.14, 0x628918) starts with bCalmShip set
 * and clears it for any fire, hull breach, intruder or damaged system. Calm
 * crew seek the medbay below 99% health instead of 25% (CheckForHealing). */
int game_enemy_calm(const Game*g){const Ship*s=&g->enemy;int i;
    for(i=0;i<s->room_count;i++)if(s->rooms[i].fires||s->rooms[i].breaches)return 0;
    for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==1)return 0;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)if(s->systems[i].capacity>0&&s->systems[i].health<s->systems[i].capacity)return 0;
    for(i=0;i<SUB_COUNT;i++)if(g->subsystems[1][i].capacity>0&&g->subsystems[1][i].health<g->subsystems[1][i].capacity)return 0;
    if(g->teleporter[1].installed&&g->teleporter[1].health<g->teleporter[1].capacity)return 0;
    if(g->enemy_drone.capacity>0&&g->enemy_drone.health<g->enemy_drone.capacity)return 0;
    for(i=0;i<g->artillery_count;i++)if(g->artillery[i].health<g->artillery[i].capacity)return 0;
    if(g->enemy_cloak.capacity>0&&g->enemy_cloak.health<g->enemy_cloak.capacity)return 0;
    if(g->enemy_pilot_room>=0&&!g->enemy_pilot_health)return 0;
    return 1;
}
static int enemy_patient(const Game*g,const Crew*c,int calm){int max=species_max_health(c->species);
    return c->health*100<(calm?99:25)*max||(c->destination==g->enemy.systems[SYS_MEDBAY].room&&c->health*100<99*max);
}
/* CrewAI manning tasks (normal difficulty): piloting 64, shields 65, weapons
 * 66, engines 67, sensors 69, doors 70. A vacant task goes to the crewmember
 * with the least important current task, if that task is less important. */
static void enemy_assign_stations(Game*g,unsigned skip){const Ship*s=&g->enemy;int t,i,n=minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);
    /* V46 house rule: weapons fire only when manned, so gunners outrank shields. */
    int room[6]={g->enemy_pilot_room,s->systems[SYS_WEAPONS].capacity>0?s->systems[SYS_WEAPONS].room:-1,s->systems[SYS_SHIELDS].capacity>0?s->systems[SYS_SHIELDS].room:-1,
        s->systems[SYS_ENGINES].capacity>0?s->systems[SYS_ENGINES].room:-1,g->subsystems[1][SUB_SENSORS].capacity>0?g->subsystems[1][SUB_SENSORS].room:-1,g->subsystems[1][SUB_DOORS].capacity>0?g->subsystems[1][SUB_DOORS].room:-1};
    static const int value[6]={64,65,66,67,69,70};int current[GAME_MAX_ENEMY_CREW];
    for(i=0;i<n;i++){const Crew*c=&g->enemy_crew[i];current[i]=1000;
        if(c->health<=0||c->ship!=1||(skip&(1u<<i)))current[i]=-1;
        else for(t=0;t<6;t++)if(room[t]>=0&&room_index(s,room[t])>=0&&c->station==room[t]){int k,first=1;for(k=0;k<i;k++)if(current[k]>=0&&g->enemy_crew[k].station==room[t])first=0;if(first)current[i]=value[t];}}
    for(t=0;t<6;t++){int held=0,best=-1;if(room[t]<0||room_index(s,room[t])<0)continue;
        for(i=0;i<n;i++)held|=current[i]==value[t]&&g->enemy_crew[i].station==room[t];
        if(held)continue;
        for(i=0;i<n;i++)if(current[i]>value[t]&&game_next_room(g,1,g->enemy_crew[i].room,room[t])>=0&&(best<0||current[i]>current[best]))best=i;
        if(best>=0){g->enemy_crew[best].station=room[t];current[best]=value[t];}
    }
}
static int enemy_allocate(int wanted,int*remaining){int n=minimum(wanted>0?wanted:0,*remaining);*remaining-=n;return n;}
void game_enemy_crew_orders(Game*g){
    int i,j,pass,heal=0,n,remaining,assigned[GAME_MAX_ROOMS]={0},order[GAME_MAX_ENEMY_CREW],rank[GAME_MAX_ENEMY_CREW];unsigned party;Ship*s=&g->enemy;
    int med=room_index(s,s->systems[SYS_MEDBAY].room),calm;
    if(g->paused||g->outcome!=GAME_PLAYING||s->hull<=0||s->automated||g->ticks%GAME_HZ)return;
    n=minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);calm=game_enemy_calm(g);
    for(i=0;i<n;i++){Crew*c=&g->enemy_crew[i];if(c->health>0&&c->ship==1&&enemy_patient(g,c,calm))heal=1;}
    /* Reserve only legal manual bars; racial bonus power is already supplied. */
    remaining=s->reactor>0?s->reactor:0;
    for(i=0;i<GAME_SYSTEM_COUNT;i++)s->systems[i].power=0;
    g->enemy_drone.power=0;g->teleporter[1].power=0;
    if(heal)s->systems[SYS_MEDBAY].power=enemy_allocate(minimum(1,s->systems[SYS_MEDBAY].health-s->systems[SYS_MEDBAY].bonus_power),&remaining);
    s->systems[SYS_OXYGEN].power=enemy_allocate(minimum(1,s->systems[SYS_OXYGEN].health-s->systems[SYS_OXYGEN].bonus_power),&remaining);
    {static const int order[]={SYS_SHIELDS,SYS_WEAPONS};for(i=0;i<2;i++){GameSystemState*d=&s->systems[order[i]];d->power=enemy_allocate(d->health-d->bonus_power,&remaining);}}
    if(g->teleporter[1].installed)g->teleporter[1].power=enemy_allocate(g->teleporter[1].health-g->teleporter[1].bonus_power,&remaining);
    g->enemy_drone.power=enemy_allocate(g->enemy_drone.health,&remaining);
    s->systems[SYS_ENGINES].power=enemy_allocate(s->systems[SYS_ENGINES].health-s->systems[SYS_ENGINES].bonus_power,&remaining);
    if(heal)s->systems[SYS_MEDBAY].power+=enemy_allocate(s->systems[SYS_MEDBAY].health-s->systems[SYS_MEDBAY].bonus_power-s->systems[SYS_MEDBAY].power,&remaining);
    if(s->shields>game_shield_capacity(s))s->shields=game_shield_capacity(s);
    {unsigned patients=0;for(i=0;i<n;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==1&&enemy_patient(g,&g->enemy_crew[i],calm))patients|=1u<<i;enemy_assign_stations(g,patients);}
    /* Patients first, then workers already beside urgent jobs, then other
     * nonpilots. Preserve the pilot whenever another crew member can respond. */
    for(i=0;i<n;i++){
        Crew*c=&g->enemy_crew[i];int r=room_index(s,c->room),priority=64;
        if(r>=0){GameRoom*room=&s->rooms[r];int sys=room->system;
            if(room->fires||room->breaches)priority=16;
            if(sys>=0&&s->systems[sys].health<s->systems[sys].capacity)priority=minimum(priority,sys==SYS_SHIELDS?1:sys==SYS_OXYGEN&&s->oxygen<250?0:48+enemy_system_priority(sys,heal));
        }
        if(c->station==g->enemy_pilot_room)priority=100;
        if(enemy_patient(g,c,calm))priority=-1;
        order[i]=i;rank[i]=priority;
        for(j=i;j>0&&rank[order[j]]<rank[order[j-1]];j--){int swap=order[j];order[j]=order[j-1];order[j-1]=swap;}
    }
    {int current[GAME_MAX_ENEMY_CREW];for(i=0;i<n;i++)current[i]=rank[i]<64?rank[i]:boarding_manning_priority(g,&g->enemy_crew[i]);
     party=boarding_enemy_party(g,current,n);}
    for(pass=0;pass<n;pass++){
        Crew*c;int patient,target=-1,best=1001;i=order[pass];c=&g->enemy_crew[i];
        if(c->health<=0||c->ship!=1)continue;
        patient=enemy_patient(g,c,calm);
        if(!patient&&(party&(1u<<i))){target=g->teleporter[1].room;j=room_index(s,target);if(j>=0)assigned[j]++;
            if(target!=c->destination){c->destination=target;c->move_ticks=c->repair_ticks=c->heal_accumulator=0;}continue;}
        if(patient&&med>=0&&effective(s,SYS_MEDBAY)>0&&enemy_room_safe(&s->rooms[med],c)&&
            assigned[med]<s->rooms[med].w*s->rooms[med].h&&game_next_room(g,1,c->room,s->rooms[med].id)>=0){target=s->rooms[med].id;best=0;}
        for(j=0;j<s->room_count&&best>0;j++){
            GameRoom*r=&s->rooms[j];int priority=1000,slots=1,k,intruders=0,sys=enemy_system_priority(r->system,heal);
            if(!enemy_room_safe(r,c)||game_next_room(g,1,c->room,r->id)<0)continue;
            for(k=0;k<g->crew_count;k++)intruders+=g->crew[k].health>0&&g->crew[k].ship==1&&g->crew[k].room==r->id;
            if(intruders){priority=16+sys;slots=intruders;}
            if(r->fires&&r->oxygen>100){priority=minimum(priority,16+sys);slots=minimum(2,r->w*r->h);}
            else if(r->breaches)priority=minimum(priority,32+sys);
            else if(r->system>=0&&s->systems[r->system].health<s->systems[r->system].capacity){
                int repair=r->system==SYS_SHIELDS?1:r->system==SYS_OXYGEN&&s->oxygen<250?0:48+sys;
                priority=minimum(priority,repair);
            }
            if(r->id==g->enemy_pilot_room&&!g->enemy_pilot_health)priority=minimum(priority,52);
            if(r->id==g->enemy_drone.room&&g->enemy_drone.health<g->enemy_drone.capacity)priority=minimum(priority,52);
            if(g->teleporter[1].installed&&r->id==g->teleporter[1].room&&g->teleporter[1].health<g->teleporter[1].capacity)priority=minimum(priority,51);
            {int k;for(k=0;k<g->artillery_count;k++)if(r->id==g->artillery[k].room&&g->artillery[k].health<g->artillery[k].capacity)priority=minimum(priority,50);}
            if(g->enemy_cloak.capacity>0&&r->id==g->enemy_cloak.room&&g->enemy_cloak.health<g->enemy_cloak.capacity)priority=minimum(priority,53);
            for(k=0;k<SUB_COUNT;k++)if(g->subsystems[1][k].room==r->id&&g->subsystems[1][k].health<g->subsystems[1][k].capacity)priority=minimum(priority,55);
            if(priority==1000&&r->id==c->station)priority=64;
            if(assigned[j]>=minimum(slots,r->w*r->h))continue;
            if(priority<best||(priority==best&&r->id==c->destination)){best=priority;target=r->id;}
        }
        if(target<0)target=c->room;
        j=room_index(s,target);if(j>=0)assigned[j]++;
        if(target!=c->destination){c->destination=target;c->move_ticks=c->repair_ticks=c->heal_accumulator=0;}
    }
}
static int crew_room_available(const Game*g,int faction,int index,int side,int dest){const Crew*all=faction?g->enemy_crew:g->crew;const Ship*s=side?&g->enemy:&g->player;int i,n=0,r=room_index(s,dest);
    if(r<0)return 0;
    for(i=0;i<(faction?g->enemy_crew_count:g->crew_count);i++)if(i!=index&&all[i].health>0&&all[i].ship==side&&all[i].room==dest)n++;
    return n<s->rooms[r].w*s->rooms[r].h;
}
static void crew_step(Game *g,int service) {
    int faction,i;if(!service)game_enemy_crew_orders(g);
    for(faction=0;faction<2;faction++)for(i=0;i<(faction?g->enemy_crew_count:g->crew_count);i++) {
        Crew*c=faction?&g->enemy_crew[i]:&g->crew[i];Ship*ship=c->ship?&g->enemy:&g->player;int r;
        int pilot=c->ship?g->enemy_pilot_room:g->pilot_room;int*pilothealth=c->ship?&g->enemy_pilot_health:&g->pilot_health;
        if(service&&c->ship!=0&&(faction||g->enemy.hull<=0))continue;
        if(c->health<=0){c->heal_accumulator=0;continue;}
        r=room_index(ship,c->room);
        if(r>=0&&ship->rooms[r].oxygen<100&&g->ticks%GAME_HZ==0){c->health-=species_suffocation_damage(c->species,g->ticks);if(c->health<0)c->health=0;}
        if(c->health<=0){c->heal_accumulator=0;continue;}
        if(!crew_healing(g,c,faction))c->heal_accumulator=0;
        if(c->room!=c->destination){
            if(++c->move_ticks>=species_move_ticks(c->species)){int next=game_next_room(g,c->ship,c->room,c->destination);if(next>=0&&crew_room_available(g,faction,i,c->ship,next)&&crew_pass_door(g,faction,c->ship,c->room,next)){c->room=next;c->move_ticks=0;}else c->move_ticks=species_move_ticks(c->species)-1;}
            c->repair_ticks=0;continue;
        }
        /* A stationary patient receives one medical tick independent of work or
         * melee. Work interruptions below must not suppress or duplicate it. */
        if(crew_healing(g,c,faction)){c->heal_accumulator+=6*effective(ship,SYS_MEDBAY);
            while(c->heal_accumulator>=GAME_HZ&&c->health<species_max_health(c->species)){c->health++;c->heal_accumulator-=GAME_HZ;}if(c->health==species_max_health(c->species))c->heal_accumulator=0;}
        if(r>=0&&ship->rooms[r].fires>0){c->repair_ticks=0;continue;}
        if(boarding_contested(g,c,faction)){c->repair_ticks=0;continue;}
        if(r>=0&&ship->rooms[r].breaches){c->repair_ticks=0;continue;}
        if(c->ship==faction&&g->teleporter[c->ship].installed&&c->room==g->teleporter[c->ship].room&&g->teleporter[c->ship].health<g->teleporter[c->ship].capacity){
            if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){g->teleporter[c->ship].health++;c->repair_ticks=0;}
        }
        if(!c->ship&&!faction&&g->player_artillery.capacity>0&&c->room==g->player_artillery.room&&g->player_artillery.health<g->player_artillery.capacity){
            if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){g->player_artillery.health++;c->repair_ticks=0;}
        }
        if(!c->ship&&!faction&&g->player_cloak.capacity>0&&c->room==g->player_cloak.room&&g->player_cloak.health<g->player_cloak.capacity){
            if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){g->player_cloak.health++;c->repair_ticks=0;}
        }
        if(!c->ship&&!faction&&g->drones.installed&&c->room==g->drones.room&&g->drones.health<g->drones.capacity){
            if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){g->drones.health++;c->repair_ticks=0;}
        }
        if(c->ship&&faction&&c->room==g->enemy_drone.room&&g->enemy_drone.health<g->enemy_drone.capacity){
            if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){int k,used=g->teleporter[1].power+g->enemy_drone.power;g->enemy_drone.health++;c->repair_ticks=0;for(k=0;k<GAME_SYSTEM_COUNT;k++)used+=g->enemy.systems[k].power;if(used<g->enemy.reactor)g->enemy_drone.power++;}
        }
        if(c->ship&&faction){int k;for(k=0;k<g->artillery_count;k++){Artillery*art=&g->artillery[k];
                if(c->room==art->room&&art->health<art->capacity&&(c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){art->health++;art->power=art->health;c->repair_ticks=0;}}
            if(g->enemy_cloak.capacity>0&&c->room==g->enemy_cloak.room&&g->enemy_cloak.health<g->enemy_cloak.capacity&&(c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){g->enemy_cloak.health++;g->enemy_cloak.power=g->enemy_cloak.health;c->repair_ticks=0;}}
        if(c->ship!=faction)continue; /* Enemy systems cannot be repaired by boarders. */
        {int sub;for(sub=0;sub<SUB_COUNT;sub++){Subsystem*s=&g->subsystems[c->ship][sub];if(s->room==c->room&&s->health<s->capacity){if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){s->health++;c->repair_ticks=0;}}}}
        if(c->room==pilot&&*pilothealth<1){if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){*pilothealth=1;c->repair_ticks=0;}}
        {int k;for(k=0;k<4;k++){int room=k==0?pilot:ship->systems[k==1?SYS_ENGINES:k==2?SYS_WEAPONS:SYS_SHIELDS].room;
            if(c->room==room&&c->skill[k]<120*GAME_HZ){c->skill[k]+=1+(c->species==SPEC_HUMAN&&g->ticks%10==0);if(c->skill[k]>120*GAME_HZ)c->skill[k]=120*GAME_HZ;}}}
        if(r>=0&&ship->rooms[r].system>=0){GameSystemState*sys=&ship->systems[ship->rooms[r].system];
            if(sys->health<sys->capacity){if((c->repair_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){sys->health++;c->repair_ticks=0;if(faction){int k,used=g->teleporter[c->ship].power+(c->ship?g->enemy_drone.power:g->drones.power);for(k=0;k<GAME_SYSTEM_COUNT;k++)used+=ship->systems[k].power;if(used<ship->reactor)sys->power++;}else say(g,"System repaired: restore its power.");}}
            else c->repair_ticks=0;
        }
    }
}
static int enemy_manned(const Game*g,int room){int i;for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++){const Crew*c=&g->enemy_crew[i];if(c->ship==1&&c->health>0&&c->room==room&&c->destination==room)return 1;}return 0;}
/* V46 house rule (Blite): a weapon volley starts only while a crewmember of the
 * ship's own side stands in the weapons room. Automated ships have no crew. */
int game_weapons_manned(const Game*g,int side){
    if(side)return g->enemy.automated||enemy_manned(g,g->enemy.systems[SYS_WEAPONS].room);
    return station_crew(g,g->player.systems[SYS_WEAPONS].room)>=0;
}
int game_artillery_manned(const Game*g,int index){
    if(index<0||index>=g->artillery_count)return 0;
    return g->enemy.automated||!game_is_flagship(g)||enemy_manned(g,g->artillery[index].room); /* V67: crewless outside the flagship (half-built flagship). */
}
int game_crew_blocked(const Game*g,int faction,int index){const Crew*c;int next,i,n;const GameDoor*doors;
    if(index<0||index>=(faction?g->enemy_crew_count:g->crew_count))return 0;
    c=faction?&g->enemy_crew[index]:&g->crew[index];if(c->health<=0||c->room==c->destination)return 0;
    next=game_next_room(g,c->ship,c->room,c->destination);if(next<0)return 1;
    if(!crew_room_available(g,faction,index,c->ship,next))return 1;
    doors=c->ship?g->enemy_doors:g->doors;n=c->ship?g->enemy_door_count:g->door_count;
    for(i=0;i<n;i++){const GameDoor*d=&doors[i];if((d->room_a==c->room&&d->room_b==next)||(d->room_a==next&&d->room_b==c->room)){
        if(species_door_locked(g,c->ship,d))return 1;
        return !d->open&&faction!=c->ship&&subsystem_level(g,c->ship,SUB_DOORS)>=2;}}
    return 1;
}
int game_player_ftl_operational(const Game*g){
    return g&&g->outcome==GAME_PLAYING&&g->player.hull>0&&effective(&g->player,SYS_ENGINES)>0&&
        g->pilot_health>0&&g->pilot_ion_ticks[0]==0&&g->pilot_room>=0&&
        room_index(&g->player,g->pilot_room)>=0&&station_crew(g,g->pilot_room)>=0;
}
int game_player_ftl_ready(const Game*g){return game_player_ftl_operational(g)&&g->player_ftl_charge>=GAME_PLAYER_FTL_CHARGE;}
int game_enemy_escape_start(Game*g,int delay_ticks,int hull_min,int hull_max){
    if(!g||g->outcome!=GAME_PLAYING||g->enemy.hull<=0||g->enemy_departed||g->enemy_escape_total||delay_ticks<1||delay_ticks>3600*GAME_HZ||hull_min<0||hull_max<hull_min||hull_max>1000)return 0;
    g->enemy_escape_threshold=hull_min;
    if(hull_max>hull_min){g->rng=g->rng*1664525u+1013904223u;g->enemy_escape_threshold+=(int)(g->rng%(unsigned)(hull_max-hull_min+1));}
    if(game_augment_count(g,AUG_FTL_JAMMER)&&delay_ticks<=1800*GAME_HZ)delay_ticks*=2; /* V55 FTL Jammer: enemy FTL takes twice as long. */
    g->enemy_escape_ticks=g->enemy_escape_total=delay_ticks;
    g->enemy_escape_active=g->enemy.hull<=g->enemy_escape_threshold;
    return 1;
}
int game_enemy_escape_ready(const Game*g){
    if(!g||g->enemy_departed||g->enemy.hull<=0||effective(&g->enemy,SYS_ENGINES)<=0)return 0;
    if(g->enemy_pilot_room>=0&&(g->enemy_pilot_health<=0||g->pilot_ion_ticks[1]>0))return 0;
    return g->enemy.automated||(g->enemy_pilot_room>=0&&enemy_manned(g,g->enemy_pilot_room));
}
static int enemy_boarders(const Game*g){int i,n=0;for(i=0;i<minimum(g->enemy_crew_count,GAME_MAX_ENEMY_CREW);i++)n+=g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==0;return n;}
static void enemy_escape_depart(Game*g){int i;
    g->enemy_departed=1;g->enemy_escape_active=0;g->enemy.hull=0;
    /* Departure removes the remote ship; this is not a damage/death event. */
    for(i=0;i<g->crew_count;i++)if(g->crew[i].ship==1){g->crew[i].health=0;g->crew[i].death_processed=1;}
    for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].ship==1){g->enemy_crew[i].health=0;g->enemy_crew[i].death_processed=1;}
    enemy_loadout_stop(g);g->enemy_pending_shots=g->enemy_fire_ticks=g->enemy_weapon.pending_shots=g->enemy_weapon.charge_ticks=0;
    g->enemy_drone.power=g->enemy_drone.charge_ticks=0;g->teleporter[1].power=0;
    memset(g->effects,0,sizeof g->effects);memset(g->autofire,0,sizeof g->autofire);
    for(i=0;i<g->weapon_count;i++)g->weapons[i].pending_shots=0;
    species_refresh_power(g);g->result_reason=RESULT_ENEMY_ESCAPE;
    if(!game_alive_crew(g)){g->outcome=GAME_DEFEAT;g->result_reason=RESULT_PLAYER_CREW;say(g,"All crew lost with the escaping ship.");}
    else if(enemy_boarders(g))say(g,"Enemy escaped! Defeat the remaining boarders.");
    else {g->outcome=GAME_ESCAPE;say(g,"Enemy escaped.");}
}
void game_service_step(Game *g) {
    if(g->paused || g->outcome==GAME_DEFEAT)return;
    g->ticks++;ion_recover(g,0);if(g->enemy.hull>0&&game_away_crew(g))ion_recover(g,1);species_refresh_power(g);subsystem_step(g,1);ship_step(&g->player);atmosphere_step(g,1);crew_step(g,1);fire_step(g,1);breach_step(g,1);boarding_step(g,1);species_step(g,1);augment_upkeep(g);drones_service_step(g);
    if(g->player.hull<=0 || !game_alive_crew(g)){g->outcome=GAME_DEFEAT;g->result_reason=g->player.hull<=0?RESULT_PLAYER_HULL:RESULT_PLAYER_CREW;say(g,"All crew lost.");}
}
/* Independent authored enemy slots. Main slot wins power/ammo ties; a queued
 * volley retains its payload/room and finishes after later power disruption. */
static void enemy_loadout_step(Game*g){
    if(g->enemy.hull<=0||g->player.hull<=0||!game_alive_crew(g)||(!g->enemy.automated&&!game_alive_enemy_crew(g))){enemy_loadout_stop(g);return;}
    for(int i=0;i<g->enemy_weapon_count;i++){
        WeaponState*w=i?&g->enemy_weapon_secondary:&g->enemy_weapon;
        if(game_enemy_weapon_powered(g,i)){
            if(!w->pending_shots&&w->charge_ticks<w->cooldown_ticks&&!game_player_cloaked(g))w->charge_ticks=minimum(w->cooldown_ticks,w->charge_ticks+1+(!g->enemy.automated&&g->ticks%20==0&&enemy_manned(g,g->enemy.systems[SYS_WEAPONS].room)));
            if(!w->pending_shots&&w->charge_ticks>=w->cooldown_ticks&&g->enemy_missiles>=w->missiles&&game_weapons_manned(g,1)){
                g->enemy_missiles-=w->missiles;w->charge_ticks=0;w->shot_ticks=0;
                w->queued_beam_length=0;w->queued_bomb=w->bomb;
                /* V51: enemy loadouts may carry beams and bombs like the original's ships. */
                if(w->beam_length){w->queued_target_count=0;w->pending_shots=game_enemy_plan_beam(g,w->beam_length,&w->queued_beam_x0,&w->queued_beam_y0,&w->queued_beam_x1,&w->queued_beam_y1)?1:0;if(w->pending_shots)w->queued_beam_length=w->beam_length;}
                else w->queued_target_count=w->pending_shots=game_enemy_plan_volley(g,w->queued_targets,w->shots);
                w->queued_ion_damage=w->ion_damage;w->queued_damage=weapon_damage(w);
            }
        }else w->charge_ticks=0;
        if(w->pending_shots>0&&--w->shot_ticks<=0){
            int launched;g->launch_mount=i+1;
            if(w->queued_beam_length)launched=game_launch_beam(g,1,w->queued_beam_x0,w->queued_beam_y0,w->queued_beam_x1,w->queued_beam_y1,&w->queued_damage);
            else{w->target_room=w->queued_targets[w->queued_target_count-w->pending_shots];
            launched=w->queued_bomb?game_launch_bomb(g,1,w->target_room,&w->queued_damage):w->queued_ion_damage?game_launch_ion(g,1,w->target_room,w->queued_ion_damage):game_launch_effect(g,1,w->target_room,w->missiles?EFFECT_MISSILE:EFFECT_LASER,&w->queued_damage,w->shield_piercing);}
            g->launch_mount=0;if(launched){w->pending_shots--;w->shot_ticks=6;}
        }
    }
    g->enemy_fire_ticks=g->enemy_weapon.charge_ticks;g->enemy_pending_shots=g->enemy_weapon.pending_shots;g->enemy_shot_ticks=g->enemy_weapon.shot_ticks;
}
/* V55 crew/repair augments, also active between fights. */
static void augment_upkeep(Game*g){int i;
    if(game_augment_count(g,AUG_NANO_MEDBAY)&&g->ticks%(2*GAME_HZ)==0)for(i=0;i<g->crew_count;i++){Crew*c=&g->crew[i];if(c->health>0&&c->ship==0&&c->health<species_max_health(c->species))c->health++;}
    if(game_augment_count(g,AUG_SLUG_GEL)&&g->ticks%(20*GAME_HZ)==0)for(i=0;i<GAME_SYSTEM_COUNT;i++){GameSystemState*y=&g->player.systems[i];if(y->health<y->capacity){y->health++;break;}}
    if(game_augment_count(g,AUG_CREW_STIMS)&&g->ticks%4==0)for(i=0;i<g->crew_count;i++){Crew*c=&g->crew[i];if(c->health>0&&c->room!=c->destination&&c->move_ticks+1<species_move_ticks(c->species))c->move_ticks++;}
}
void game_step(Game *g) {
    int i;
    if(g->paused || g->outcome) return;
    g->ticks++;ion_recover(g,0);ion_recover(g,1);species_refresh_power(g);subsystem_step(g,0);ship_step(&g->player);ship_step(&g->enemy);
    if(g->player.shields<effective(&g->player,SYS_SHIELDS)/2 && g->ticks%5==0 && game_station_bonus(g,SYS_SHIELDS)>0)g->player.shield_ticks++;
    atmosphere_step(g,0);
    drones_intercept(g);
    effects_step(g);
    crew_step(g,0);fire_step(g,0);breach_step(g,0);boarding_step(g,0);boarding_destroyed_ships(g);species_step(g,0);augment_upkeep(g);
    drones_step(g);
    for(i=0;i<g->weapon_count;i++) {
        WeaponState *w=&g->weapons[i];
        if(game_weapon_powered(g,i)) {if(w->charge_ticks<w->cooldown_ticks&&!game_enemy_cloaked(g)) {w->charge_accumulator+=100+game_station_bonus(g,SYS_WEAPONS)+10*game_augment_count(g,AUG_AUTO_COOLDOWN);w->charge_ticks+=w->charge_accumulator/100;w->charge_accumulator%=100;w->charge_ticks=minimum(w->charge_ticks,w->cooldown_ticks);}}
        else w->charge_ticks=0;
        /* A queued volley keeps its target; the next uses the current selection.
         * Check readiness first to avoid repeated charging/no-ammo messages. */
        if(g->autofire[i] && g->player.hull>0 && (g->enemy.hull>0||(g->enemy_departed&&w->bomb&&w->bomb_side==1)) && !(game_enemy_cloaked(g)&&!(w->bomb&&w->bomb_side)) &&
           game_alive_crew(g) &&
           game_weapon_powered(g,i) && game_weapons_manned(g,0) && w->charge_ticks>=w->cooldown_ticks &&
           !w->pending_shots && g->missiles>=w->missiles && w->aim_room!=GAME_AIM_HOLD)
            game_fire(g,i,game_weapon_aim(g,i));
        if(w->pending_shots>0 && --w->shot_ticks<=0) {
            g->launch_origin=w->missiles?ORIGIN_MISSILE:!strcmp(game_weapon_short(w->catalog_id),"ANTIBIO")?ORIGIN_BIO:ORIGIN_WEAPON;
            g->ach.used|=w->missiles?USED_MISSILE:USED_WEAPON;g->launch_mount=i+1;
            if(w->queued_bomb?launch_bomb(g,w->queued_bomb_side,w->target_room,&w->queued_damage,w->queued_bomb_side==1):w->queued_beam_length?game_launch_beam(g,0,w->queued_beam_x0,w->queued_beam_y0,w->queued_beam_x1,w->queued_beam_y1,&w->queued_damage):w->queued_ion_damage?game_launch_ion(g,0,w->target_room,w->queued_ion_damage):game_launch_effect(g,0,w->target_room,w->missiles?EFFECT_MISSILE:EFFECT_LASER,&w->queued_damage,w->shield_piercing)) {
                w->pending_shots--;w->shot_ticks=6; }
            g->launch_origin=ORIGIN_NONE;g->launch_mount=0;
        }
    }
    if(g->enemy_weapon_count)enemy_loadout_step(g);else {
    /* One explicit basic laser selected from the original randomized pool. */
    if(g->enemy.hull>0 && !g->enemy_pending_shots && effective(&g->enemy,SYS_WEAPONS)>=g->enemy_weapon.power && (!g->enemy_weapon.bomb||g->enemy_missiles>=g->enemy_weapon.missiles)) {
        if(!game_player_cloaked(g))g->enemy_fire_ticks=minimum(g->enemy_weapon.cooldown_ticks,g->enemy_fire_ticks+1+(!g->enemy.automated&&g->ticks%20==0&&enemy_manned(g,g->enemy.systems[SYS_WEAPONS].room)));
        if(g->enemy_fire_ticks>=g->enemy_weapon.cooldown_ticks&&game_weapons_manned(g,1)) {
            g->enemy_fire_ticks=0;
            if(g->enemy_weapon.bomb)g->enemy_missiles-=g->enemy_weapon.missiles;
            g->enemy_pending_shots=g->enemy_weapon.beam_length?1:g->enemy_weapon.shots;g->enemy_shot_ticks=0;
            {WeaponState*w=&g->enemy_weapon;w->queued_bomb=w->bomb;w->queued_beam_length=0;
             w->queued_target_count=0;
             if(w->beam_length){if(game_enemy_plan_beam(g,w->beam_length,&w->queued_beam_x0,&w->queued_beam_y0,&w->queued_beam_x1,&w->queued_beam_y1))w->queued_beam_length=w->beam_length;}
             else g->enemy_pending_shots=w->queued_target_count=game_enemy_plan_volley(g,w->queued_targets,g->enemy_pending_shots);
             w->queued_ion_damage=w->ion_damage;w->queued_damage=weapon_damage(w);}
        }
    } else g->enemy_fire_ticks=0;
    if(g->enemy.hull>0 && g->enemy_pending_shots>0 && --g->enemy_shot_ticks<=0) {
        WeaponState *w=&g->enemy_weapon;g->launch_mount=1;
        if(w->queued_target_count>=g->enemy_pending_shots)w->target_room=w->queued_targets[w->queued_target_count-g->enemy_pending_shots];
        if(w->queued_bomb?game_launch_bomb(g,1,w->target_room,&w->queued_damage):w->queued_beam_length?game_launch_beam(g,1,w->queued_beam_x0,w->queued_beam_y0,w->queued_beam_x1,w->queued_beam_y1,&w->queued_damage):w->queued_ion_damage?game_launch_ion(g,1,w->target_room,w->queued_ion_damage):game_launch_effect(g,1,w->target_room,w->missiles?EFFECT_MISSILE:EFFECT_LASER,&w->queued_damage,w->shield_piercing)) {
            g->enemy_pending_shots--;g->enemy_shot_ticks=6; }
        g->launch_mount=0;
    }
    }
    cloak_step(g);artillery_step(g);player_artillery_step(g);flagship_drones_step(g);surge_step(g);
    {EnemyCloak*k=&g->player_cloak;if(k->active_ticks>0){if(minimum(k->power,k->health)-k->ion_bars<=0)k->active_ticks=0;else if(--k->active_ticks==0)ion_stack(&k->ion_bars,&k->ion_ticks,4);}}
    if(game_augment_count(g,AUG_SHIELD_RECHARGE)&&g->player.shields<game_shield_capacity(&g->player)&&g->ticks%7==0)g->player.shield_ticks+=game_augment_count(g,AUG_SHIELD_RECHARGE);
    /* Imported combat drone: bounded 10-second firing interval is a demake rule. */
    if(g->enemy_drone.capacity>0&&!game_is_flagship(g)) {
        EnemyDrone *d=&g->enemy_drone;
        if(d->power-d->ion_bars>=2 && d->health>=2 && g->enemy.hull>0 && g->player.hull>0) {
            if(d->charge_ticks<d->cooldown_ticks)d->charge_ticks++;
            if(d->charge_ticks>=d->cooldown_ticks) {
                int room=g->player.rooms[random100(g)%(unsigned)g->player.room_count].id;
                GameDamage damage={d->damage,d->damage,d->damage*15,d->fire_chance,0,0,0,0};
                if(game_launch_effect(g,1,room,EFFECT_LASER,&damage,0))d->charge_ticks=0;
            }
        } else d->charge_ticks=0;
        if(g->enemy.hull>0 && g->enemy.automated && g->ticks%(12*GAME_HZ)==0 && d->health<d->capacity){d->health++;d->power=d->health;}
    }
    g->enemy_weapon.charge_ticks=g->enemy_fire_ticks;
    g->enemy_weapon.pending_shots=g->enemy_pending_shots;
    /* Provisional automated repair: one installed system bar every 12 seconds.
     * The actual engine's repair timing has not been recovered. */
    if(g->enemy.hull>0 && g->enemy.automated && g->ticks%(12*GAME_HZ)==0) {int done=0;
        for(i=0;i<GAME_SYSTEM_COUNT&&!done;i++) {
            GameSystemState *s=&g->enemy.systems[i];int r=room_index(&g->enemy,s->room);
            if(game_is_flagship(g)&&r>=0&&(g->enemy.rooms[r].fires||g->enemy.rooms[r].breaches))continue;
            if(s->health<s->capacity) {s->health++;s->power=s->health;done=1;}
        }
        /* V42 flagship AI also restores artillery, cloaking, doors and piloting. */
        if(game_is_flagship(g))for(i=0;i<g->artillery_count&&!done;i++){Artillery*a=&g->artillery[i];int r=room_index(&g->enemy,a->room);
            if(r>=0&&!g->enemy.rooms[r].fires&&!g->enemy.rooms[r].breaches&&a->health<a->capacity){a->health++;a->power=a->health;done=1;}}
        if(game_is_flagship(g)&&!done&&g->enemy_cloak.health<g->enemy_cloak.capacity){int r=room_index(&g->enemy,g->enemy_cloak.room);
            if(r>=0&&!g->enemy.rooms[r].fires&&!g->enemy.rooms[r].breaches){g->enemy_cloak.health++;g->enemy_cloak.power=g->enemy_cloak.health;done=1;}}
        if(game_is_flagship(g)&&!done&&g->subsystems[1][SUB_DOORS].health<g->subsystems[1][SUB_DOORS].capacity){g->subsystems[1][SUB_DOORS].health++;done=1;}
        if(game_is_flagship(g)&&!done&&g->enemy_pilot_room>=0&&!g->enemy_pilot_health){g->enemy_pilot_health=1;done=1;}
    }
    boarding_destroyed_ships(g);
    if(g->player.hull<=0) {g->outcome=GAME_DEFEAT;g->result_reason=RESULT_PLAYER_HULL;say(g,"Kestrel destroyed. Restart to try again.");}
    else if(!game_alive_crew(g)){g->outcome=GAME_DEFEAT;g->result_reason=RESULT_PLAYER_CREW;say(g,"All crew lost. Restart to try again.");}
    else {int boarders=enemy_boarders(g);
        if(g->enemy_departed){if(!boarders&&g->result_reason!=RESULT_PLAYER_ESCAPE){g->outcome=GAME_ESCAPE;g->result_reason=RESULT_ENEMY_ESCAPE;say(g,"Enemy escaped. Remaining boarders defeated.");}}
        else if(g->enemy.hull<=0&&!boarders){g->outcome=GAME_VICTORY;g->result_reason=RESULT_ENEMY_HULL;say(g,"Enemy destroyed. Encounter complete!");}
        else if(!g->enemy.automated&&!game_alive_enemy_crew(g)&&game_is_flagship(g)){g->enemy.automated=1;say(g,"Flagship crew lost. Its AI has taken control.");}
        else if(!g->enemy.automated&&!game_alive_enemy_crew(g)){g->outcome=GAME_VICTORY;g->result_reason=RESULT_ENEMY_CREW;say(g,"Enemy crew defeated. Ship captured.");}
        else if(g->enemy.hull>0&&g->enemy_escape_total>0){
            /* DS interpretation: raw hull threshold starts a latched attempt;
             * timer is operational charge time. These semantics are authored. */
            if(g->enemy.hull<=g->enemy_escape_threshold)g->enemy_escape_active=1;
            if(g->enemy_escape_active&&game_enemy_escape_ready(g)&&g->enemy_escape_ticks>0&&--g->enemy_escape_ticks==0)enemy_escape_depart(g);
        }
    }
    if(g->outcome&&g->enemy_weapon_count){enemy_loadout_stop(g);for(i=0;i<GAME_MAX_EFFECTS;i++)if(g->effects[i].side==1)memset(&g->effects[i],0,sizeof g->effects[i]);}
    /* Provisional DS model: each effective engine bar adds one charge unit per
     * combat tick, capped at eight. Damage/ions/unmanning hold stored charge;
     * this runs after impacts and terminal resolution, never in service time. */
    if(g->result_reason!=RESULT_PLAYER_ESCAPE&&game_player_ftl_operational(g)&&g->player_ftl_charge<GAME_PLAYER_FTL_CHARGE)
        g->player_ftl_charge=minimum(GAME_PLAYER_FTL_CHARGE,g->player_ftl_charge+minimum(8,effective(&g->player,SYS_ENGINES))*(4+game_augment_count(g,AUG_FTL_BOOSTER))/4);
    environment_step(g);
    achievements_step(g);
}
