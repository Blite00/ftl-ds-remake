#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    Game g; Ship enemy; GameEffect effects[GAME_MAX_EFFECTS]; int i,target;
    game_init(&g);
    assert(g.crew_count==3 && g.weapon_count==2 && game_alive_crew(&g)==3);
    assert(game_evasion(&g)>0);g.crew[0].destination=1;assert(game_evasion(&g)==0);
    g.crew[0].destination=g.pilot_room;g.pilot_health=0;assert(game_evasion(&g)==0);
    g.player.systems[SYS_WEAPONS].power=1;
    assert(!game_weapon_powered(&g,1) && game_weapon_powered(&g,0));
    g.player.systems[SYS_WEAPONS].power=2;
    assert(game_weapon_powered(&g,0) && !game_weapon_powered(&g,1));
    assert(game_toggle_weapon(&g,1));assert(game_weapon_powered(&g,0));
    assert(game_equip(&g,2,2));assert(game_equip(&g,3,3));assert(!game_equip(&g,4,1));
    assert(g.weapon_count==4 && g.weapons[3].damage==2);
    g.crew_count=6;for(i=3;i<6;i++){g.crew[i]=g.crew[0];g.crew[i].health=100;}
    assert(game_alive_crew(&g)==6 && game_order_crew(&g,5,g.pilot_room));
    g.outcome=GAME_VICTORY;g.crew[5].health=40;
    g.crew[5].room=g.crew[5].destination=g.player.systems[SYS_MEDBAY].room;
    g.player.systems[SYS_MEDBAY].power=1;
    g.enemy_weapon.charge_ticks=7;g.enemy_fire_ticks=7;
    g.effects[0].active=1;g.effects[0].duration=12;
    enemy=g.enemy;memcpy(effects,g.effects,sizeof effects);
    for(i=0;i<6*GAME_HZ;i++)game_service_step(&g);
    assert(g.crew[5].health>40 && g.pilot_health==1);
    assert(!memcmp(&enemy,&g.enemy,sizeof enemy) && !memcmp(effects,g.effects,sizeof effects));
    assert(g.enemy_fire_ticks==7 && g.outcome==GAME_VICTORY);
    for(i=0;i<3;i++)g.crew[i].health=0;
    assert(game_alive_crew(&g)==3);
    g.player.hull=19;g.weapons[1].enabled=0;
    g.interim_flagship=1; /* Legacy authored platform; the V42 flagship is covered elsewhere. */
    game_begin_encounter(&g,ENEMY_FLAGSHIP,5,2);
    assert(g.player.hull==19 && g.crew_count==6 && g.weapon_count==4 && !g.weapons[1].enabled);
    assert(g.enemy.room_count==8 && g.enemy.shields==2 && !g.effects[0].active);
    assert(g.outcome==GAME_PLAYING && !g.enemy_fire_ticks);
    target=g.enemy.systems[SYS_ENGINES].room;
    assert(game_set_weapon_target(&g,0,target));assert(!game_set_weapon_target(&g,0,999));
    g.player.systems[SYS_WEAPONS].power=3;g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
    /* V46: autofire waits while the weapons room is unmanned (crew 0-2 are dead). */
    g.autofire[0]=1;game_step(&g);assert(g.weapons[0].target_room!=target&&!g.weapons[0].pending_shots);
    g.crew[3].room=g.crew[3].destination=g.player.systems[SYS_WEAPONS].room;
    game_step(&g);assert(g.weapons[0].target_room==target);
    g.target_room=g.enemy.systems[SYS_SHIELDS].room;assert(g.weapons[0].target_room==target);
    printf("PASS: bounded roster/equipment, pilot, activation, service isolation, repair/heal, encounter preservation and independent targeting.\n");
    return 0;
}

