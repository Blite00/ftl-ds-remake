#include "diagnostics.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef __NDS__
#include <fat.h>
#include <nds/debug.h>
#endif

typedef struct {
    char lines[DIAGNOSTICS_LINES][DIAGNOSTICS_LINE_LENGTH];
    char path[256];
    unsigned next,count,pending,dropped,last_flush;
    int active,seen,defer_automatic;
    const char *status;
    Game previous;
} Diagnostics;
static Diagnostics log_state;

static void record(unsigned frame,const char *format,...) {
    char *line=log_state.lines[log_state.next];
    int used;
    va_list args;
    used=snprintf(line,DIAGNOSTICS_LINE_LENGTH,"f=%lu ",(unsigned long)frame);
    va_start(args,format);
    vsnprintf(line+used,DIAGNOSTICS_LINE_LENGTH-(unsigned)used,format,args);
    va_end(args);
    /* Keep externally supplied status/action text on one physical log line. */
    for(used=0;line[used];used++) if(line[used]=='\n' || line[used]=='\r')line[used]=' ';
#ifdef __NDS__
    /* Compatible emulator debug consoles only; not persistent storage. */
    {
        char debug_line[121];
        snprintf(debug_line,sizeof debug_line,"%.120s",line);
        nocashMessage(debug_line);
    }
#endif
    log_state.next=(log_state.next+1)%DIAGNOSTICS_LINES;
    if(log_state.count<DIAGNOSTICS_LINES)log_state.count++;
    if(log_state.pending<DIAGNOSTICS_LINES)log_state.pending++;
    else log_state.dropped++;
}
static void disable_file(const char *reason) {
    log_state.active=0;log_state.status=reason;
}
void diagnostics_defer_automatic_flush(int defer){log_state.defer_automatic=defer!=0;}
void diagnostics_flush(void) {
    FILE *file;
    long bytes;
    unsigned first,i;
    if(!log_state.active || !log_state.pending)return;
    file=fopen(log_state.path,"ab");
    if(!file){disable_file("Memory log: file unavailable");return;}
    if(fseek(file,0,SEEK_END)!=0 || (bytes=ftell(file))<0) {
        fclose(file);disable_file("Memory log: file seek failed");return;
    }
    if(log_state.dropped) {
        char notice[80];
        int size=snprintf(notice,sizeof notice,"BUFFER dropped=%lu older unflushed lines\n",(unsigned long)log_state.dropped);
        if(bytes+size<=DIAGNOSTICS_MAX_BYTES) {
            if(fwrite(notice,1,(size_t)size,file)!=(size_t)size) {
                fclose(file);disable_file("Memory log: file write failed");return;
            }
            bytes+=size;
        }
    }
    first=(log_state.next+DIAGNOSTICS_LINES-log_state.pending)%DIAGNOSTICS_LINES;
    for(i=0;i<log_state.pending;i++) {
        const char *line=log_state.lines[(first+i)%DIAGNOSTICS_LINES];
        size_t size=strlen(line);
        if(bytes+(long)size+1>DIAGNOSTICS_MAX_BYTES) {
            disable_file("Memory log: file reached 256 KiB");break;
        }
        if(fwrite(line,1,size,file)!=size || fputc('\n',file)==EOF) {
            disable_file("Memory log: file write failed");break;
        }
        bytes+=(long)size+1;
    }
    if(fclose(file)!=0)disable_file("Memory log: file close failed");
    log_state.pending=0;log_state.dropped=0;
}
void diagnostics_init(const char *path) {
    memset(&log_state,0,sizeof log_state);
    log_state.status="File log active";
#ifdef __NDS__
    if(!path)path="/ftl-ds.log";
    log_state.active=fatInitDefault()?1:0;
    if(!log_state.active)log_state.status="Memory log: no writable filesystem";
#else
    if(!path)path="ftl-ds.log";
    log_state.active=1;
#endif
    if(strlen(path)>=sizeof log_state.path)disable_file("Memory log: path too long");
    else snprintf(log_state.path,sizeof log_state.path,"%s",path);
    record(0,"SESSION start version=campaign-v10 frame_hz=60 simulation_hz=%d log_cap=%ld",GAME_HZ,DIAGNOSTICS_MAX_BYTES);
    diagnostics_flush();
    record(0,"LOGGER %s",log_state.status);
}
void diagnostics_input(unsigned frame,unsigned keys,int x,int y) {
    if(keys)record(frame,"INPUT keys=0x%lx touch=%d,%d",(unsigned long)keys,x,y);
}
void diagnostics_action(unsigned frame,const char *action) {
    if(action && *action)record(frame,"ACTION %s",action);
}
/* Pack state into a few records so an initial snapshot fits the 64-line ring.
   Room fields are id:oxygen:exposed; door fields are index:manual-open. */
static void room_snapshot(const Ship *ship,unsigned frame,const char *side) {
    int i,j;
    for(i=0;i<ship->room_count;i+=4) {
        char fields[160]="";
        for(j=i;j<ship->room_count && j<i+4;j++) {
            char entry[40];
            const GameRoom *r=&ship->rooms[j];
            snprintf(entry,sizeof entry," %d:%d:%d",r->id,r->oxygen,r->exposed);
            strncat(fields,entry,sizeof fields-strlen(fields)-1);
        }
        record(frame,"ROOMS %s id:oxygen:exposed%s",side,fields);
    }
}
static void door_snapshot(const Game *g,unsigned frame) {
    int i,j;
    for(i=0;i<g->door_count;i+=8) {
        char fields[144]="";
        for(j=i;j<g->door_count && j<i+8;j++) {
            char entry[28];
            snprintf(entry,sizeof entry," %d:%d",j,g->doors[j].open && g->doors[j].transit_ticks==0);
            strncat(fields,entry,sizeof fields-strlen(fields)-1);
        }
        record(frame,"DOORS index:manual-open%s",fields);
    }
}
static void room_changes(const Ship *ship,const Ship *old,unsigned frame,const char *side) {
    int i;
    for(i=0;i<ship->room_count && i<old->room_count;i++) {
        const GameRoom *r=&ship->rooms[i],*before=&old->rooms[i];
        if(r->exposed!=before->exposed || r->oxygen/250!=before->oxygen/250)
            record(frame,"ROOM_CHANGE %s room=%d oxygen=%d band=%d exposed=%d",side,r->id,r->oxygen,r->oxygen/250,r->exposed);
    }
}
static void snapshot(const Game *g,unsigned frame,const char *reason) {
    int i;
    record(frame,"SNAPSHOT %s ticks=%d pause=%d outcome=%d target=%d missiles=%d rng=%lu",reason,g->ticks,g->paused,g->outcome,g->target_room,g->missiles,(unsigned long)g->rng);
    record(frame,"PLAYER hull=%d/%d shield=%d oxygen=%d reactor=%d ENEMY hull=%d/%d shield=%d oxygen=%d",g->player.hull,g->player.max_hull,g->player.shields,g->player.oxygen,g->player.reactor,g->enemy.hull,g->enemy.max_hull,g->enemy.shields,g->enemy.oxygen);
    for(i=0;i<GAME_SYSTEM_COUNT;i++) {
        const GameSystemState *p=&g->player.systems[i],*e=&g->enemy.systems[i];
        record(frame,"SYSTEM %s player=%d/%d power=%d enemy=%d/%d power=%d",game_system_name(i),p->health,p->capacity,p->power,e->health,e->capacity,e->power);
    }
    for(i=0;i<g->crew_count;i++)record(frame,"CREW %d hp=%d room=%d destination=%d",i,g->crew[i].health,g->crew[i].room,g->crew[i].destination);
    for(i=0;i<g->weapon_count;i++)record(frame,"WEAPON %d charge=%d/%d pending=%d target=%d",i,g->weapons[i].charge_ticks,g->weapons[i].cooldown_ticks,g->weapons[i].pending_shots,g->weapons[i].target_room);
    room_snapshot(&g->player,frame,"player");
    room_snapshot(&g->enemy,frame,"enemy");
    door_snapshot(g,frame);
}
void diagnostics_poll(const Game *g,unsigned frame) {
    Game *old=&log_state.previous;
    int i,important=0,restart;
    if(!g)return;
    restart=log_state.seen && g->ticks<old->ticks;
    if(!log_state.seen || restart) {
        snapshot(g,frame,restart?"restart":"initial");
        important=1;
    } else {
        if(g->player.hull!=old->player.hull || g->player.shields!=old->player.shields || g->enemy.hull!=old->enemy.hull || g->enemy.shields!=old->enemy.shields)
            record(frame,"COMBAT player_hull=%d shield=%d enemy_hull=%d shield=%d",g->player.hull,g->player.shields,g->enemy.hull,g->enemy.shields);
        for(i=0;i<GAME_SYSTEM_COUNT;i++) {
            const GameSystemState *p=&g->player.systems[i],*e=&g->enemy.systems[i];
            if(p->health!=old->player.systems[i].health || p->power!=old->player.systems[i].power || e->health!=old->enemy.systems[i].health || e->power!=old->enemy.systems[i].power)
                record(frame,"SYSTEM_CHANGE %s player_health=%d power=%d enemy_health=%d power=%d",game_system_name(i),p->health,p->power,e->health,e->power);
        }
        for(i=0;i<g->crew_count;i++) {
            const Crew *c=&g->crew[i],*before=&old->crew[i];
            if(c->health!=before->health || c->room!=before->room || c->destination!=before->destination)
                record(frame,"CREW_CHANGE %d hp=%d room=%d destination=%d",i,c->health,c->room,c->destination);
        }
        for(i=0;i<g->door_count && i<old->door_count;i++) {
            const GameDoor *d=&g->doors[i];
            /* Transit animations deliberately do not produce door events. */
            if(d->open!=old->doors[i].open && d->transit_ticks==0 && old->doors[i].transit_ticks==0)
                record(frame,"DOOR_CHANGE %d rooms=%d,%d open=%d",i,d->room_a,d->room_b,d->open);
        }
        room_changes(&g->player,&old->player,frame,"player");
        room_changes(&g->enemy,&old->enemy,frame,"enemy");
        if(g->paused!=old->paused || g->target_room!=old->target_room || g->missiles!=old->missiles)
            record(frame,"CONTROL pause=%d target=%d missiles=%d",g->paused,g->target_room,g->missiles);
        if(strcmp(g->message,old->message))record(frame,"MESSAGE %s",g->message);
        for(i=0;i<g->weapon_count;i++)if(g->autofire[i]!=old->autofire[i])
            record(frame,"AUTOFIRE weapon=%d enabled=%d",i,g->autofire[i]);
        if(g->outcome!=old->outcome) {snapshot(g,frame,"outcome");important=1;}
    }
    if(!important && frame-log_state.last_flush>=600u) snapshot(g,frame,"periodic");
    if(important || frame-log_state.last_flush>=600u) {
        if(!log_state.defer_automatic)diagnostics_flush();
        log_state.last_flush=frame;
    }
    log_state.previous=*g;log_state.seen=1;
}
const char *diagnostics_status(void) {
    return log_state.status?log_state.status:"Logger not initialized";
}
unsigned diagnostics_line_count(void) {return log_state.count;}
const char *diagnostics_line(unsigned newest_offset) {
    if(newest_offset>=log_state.count)return "";
    return log_state.lines[(log_state.next+DIAGNOSTICS_LINES-1-newest_offset)%DIAGNOSTICS_LINES];
}


