#include "fire.h"
#include "species.h"

static int room_index(const Ship *s,int id) {
    int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return i;return -1;
}
static int capacity(const GameRoom *r) {int n=r->w*r->h;return n<4?n:4;}
void fire_ignite(Ship *s,int room,int amount) {
    int i=room_index(s,room),limit;
    GameRoom *r;
    if(i<0||amount<=0)return;
    r=&s->rooms[i];limit=capacity(r);
    if(limit<=0||r->oxygen<100)return;
    if(amount>limit-r->fires)amount=limit-r->fires;
    r->fires+=amount;
}
static unsigned roll(Game *g) {
    g->rng=g->rng*1664525u+1013904223u;return (g->rng>>16)%100u;
}
static void damage_system(Game *g,Ship *s,GameRoom *r,int enemy) {
    int sub;for(sub=0;sub<SUB_COUNT;sub++){Subsystem*d=&g->subsystems[enemy][sub];if(d->room==r->id&&d->health>0)d->health--;}
    if(r->system>=0&&r->system<GAME_SYSTEM_COUNT) {
        GameSystemState *sys=&s->systems[r->system];
        if(sys->health>0)sys->health--;
        if(sys->power>sys->health)sys->power=sys->health;
        if(r->system==SYS_SHIELDS){int cap=game_shield_capacity(s);if(s->shields>cap)s->shields=cap;if(!cap)s->shield_ticks=0;}
    } else if(!enemy&&r->id==g->pilot_room)g->pilot_health=0;
    else if(enemy&&r->id==g->enemy_pilot_room)g->enemy_pilot_health=0;
    if(g->teleporter[enemy].installed&&r->id==g->teleporter[enemy].room){Teleporter*t=&g->teleporter[enemy];if(t->health>0)t->health--;if(t->power>t->health)t->power=t->health;}
    if(!enemy&&g->drones.installed&&r->id==g->drones.room){
        if(g->drones.health>0)g->drones.health--;
        if(g->drones.power>g->drones.health)g->drones.power=g->drones.health;
    }
    if(enemy&&g->enemy_drone.capacity>0&&r->id==g->enemy_drone.room) {
        if(g->enemy_drone.health>0)g->enemy_drone.health--;
        if(g->enemy_drone.power>g->enemy_drone.health)g->enemy_drone.power=g->enemy_drone.health;
    }
}
static int contested(const Game*g,const Crew*c,int faction){int i;const Crew*op=faction?g->crew:g->enemy_crew;int count=faction?g->crew_count:g->enemy_crew_count;
    for(i=0;i<count;i++)if(op[i].health>0&&op[i].ship==c->ship&&op[i].room==c->room)return 1;
    return 0;
}
static void ship_fire_step(Game *g,Ship *s,int enemy) {
    int i,k,f;int count=enemy?g->enemy_door_count:g->door_count;GameDoor*doors=enemy?g->enemy_doors:g->doors;unsigned char spread[GAME_MAX_ROOMS]={0};
    for(i=0;i<s->room_count;i++) {
        GameRoom *r=&s->rooms[i];int fighters=0,present=0;
        if(r->fires<=0){r->fire_ticks=0;r->firefight_ticks=0;continue;}
        for(f=0;f<2;f++)for(k=0;k<(f?g->enemy_crew_count:g->crew_count);k++) {
            const Crew *c=f?&g->enemy_crew[k]:&g->crew[k];
            if(f==enemy&&c->ship==enemy&&c->health>0&&c->room==r->id&&c->destination==r->id&&!contested(g,c,f)){fighters+=species_work_rate(c->species,g->ticks);present++;}
        }
        if(fighters) {
            r->firefight_ticks+=fighters;
            if(r->firefight_ticks>=3*GAME_HZ) {
                r->firefight_ticks-=3*GAME_HZ;r->fires--;
                if(r->fires==0){r->fire_ticks=0;r->firefight_ticks=0;continue;}
            }
        } else if(!present)r->firefight_ticks=0;
        if(r->oxygen>=100&&g->ticks%(3*GAME_HZ)==0)damage_system(g,s,r,enemy);
        if(++r->fire_ticks<GAME_HZ)continue;
        r->fire_ticks=0;
        if(r->oxygen<100) {
            r->fires--;
            if(!r->fires)r->firefight_ticks=0;
            continue;
        }
        r->oxygen-=25*r->fires;if(r->oxygen<0)r->oxygen=0;
        /* V46: fire also spreads between tiles of its own room (10% per second). */
#ifndef NO_FIRE_GROWTH
        if(r->fires<capacity(r)&&roll(g)<10u)spread[i]=1;
#endif
        for(f=0;f<2;f++)for(k=0;k<(f?g->enemy_crew_count:g->crew_count);k++) {
            Crew *c=f?&g->enemy_crew[k]:&g->crew[k];
            if(c->ship==enemy&&c->health>0&&c->room==r->id){c->health-=species_fire_damage(c->species,r->fires);if(c->health<0)c->health=0;}
        }
        /* Both sides use actual door topology, never touching rectangles. */
        for(k=0;k<count;k++) {
            const GameDoor *d=&doors[k];int dest=-1;
            if(species_door_locked(g,enemy,d))continue;
            const Subsystem*control=&g->subsystems[enemy][SUB_DOORS];
            int level=control->health-control->ion_bars;if(level<0)level=0;
            /* Authored spread probabilities: open 10%, closed basic 3%,
             * reinforced 1%, strongest intact doors 0% per spread check. */
            unsigned chance=d->open?10u:level>=3?0u:level==2?1u:3u;
            if(!chance)continue;
            if(d->room_a==r->id)dest=room_index(s,d->room_b);
            else if(d->room_b==r->id)dest=room_index(s,d->room_a);
            if(dest>=0&&s->rooms[dest].oxygen>=100&&
               s->rooms[dest].fires<capacity(&s->rooms[dest])&&roll(g)<chance)spread[dest]=1;
        }
    }
    /* Apply after scanning to avoid multi-room propagation within one tick. */
    for(i=0;i<s->room_count;i++)if(spread[i])fire_ignite(s,s->rooms[i].id,1);
    {int area=0,total=0;for(i=0;i<s->room_count;i++){int a=s->rooms[i].w*s->rooms[i].h;area+=a;total+=a*s->rooms[i].oxygen;}s->oxygen=area?total/area:0;}
}
void fire_step(Game *g,int service) {
    if(g->paused||g->outcome==GAME_DEFEAT||(!service&&g->outcome!=GAME_PLAYING))return;
    ship_fire_step(g,&g->player,0);
    {int away=0,i;for(i=0;i<g->crew_count;i++)away+=g->crew[i].health>0&&g->crew[i].ship==1;
     if(!service||(g->enemy.hull>0&&away))ship_fire_step(g,&g->enemy,1); }
}
