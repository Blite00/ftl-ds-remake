#include "breach.h"
#include "boarding.h"
#include "species.h"
void breach_add(Ship*s,int room,int amount){int i;if(amount<=0)return;
 for(i=0;i<s->room_count;i++)if(s->rooms[i].id==room){GameRoom*r=&s->rooms[i];int cap=r->w*r->h;if(cap>4)cap=4;
  if(amount>cap-r->breaches)amount=cap-r->breaches;
  r->breaches+=amount;return;}
}
void breach_step(Game*g,int service){int side,i,k;
 if(g->paused||g->outcome==GAME_DEFEAT||(!service&&g->outcome!=GAME_PLAYING))return;
 for(side=0;side<(service?1:2);side++){Ship*s=side?&g->enemy:&g->player;Crew*crew=side?g->enemy_crew:g->crew;int count=side?g->enemy_crew_count:g->crew_count;
  if(s->hull<=0)continue;
  for(i=0;i<s->room_count;i++){GameRoom*r=&s->rooms[i];int workers=0,present=0;
   if(!r->breaches){r->breach_repair_ticks=0;continue;}
   if(!r->fires)for(k=0;k<count;k++){Crew*c=&crew[k];if(c->health>0&&c->ship==side&&c->room==r->id&&c->destination==r->id&&!boarding_contested(g,c,side)){workers+=species_work_rate(c->species,g->ticks);present++;}}
   if(!present){r->breach_repair_ticks=0;continue;}
   r->breach_repair_ticks+=workers;
   if(r->breach_repair_ticks>=BREACH_SEAL_TICKS){r->breaches--;r->breach_repair_ticks-=BREACH_SEAL_TICKS;if(!r->breaches)r->breach_repair_ticks=0;}
  }
 }
}
