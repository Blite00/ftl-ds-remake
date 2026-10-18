#include "rewards.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;static PendingReward offer,offered;static int cargo[4],oldcargo[4];
static void setup(void){game_init(&g);memset(&offer,0,sizeof offer);for(int i=0;i<4;i++)cargo[i]=-1;}
static void rejects(int slot,int replace){before=g;offered=offer;memcpy(oldcargo,cargo,sizeof cargo);assert(!rewards_can_accept(&offer,&g,cargo,4,slot,replace));assert(!rewards_accept(&offer,&g,cargo,4,slot,replace));assert(!memcmp(&before,&g,sizeof g)&&!memcmp(&offer,&offered,sizeof offer)&&!memcmp(cargo,oldcargo,sizeof cargo));}
static void full(void){for(int i=g.crew_count;i<GAME_MAX_CREW;i++)game_init_crew(&g,0,i,g.player.rooms[i].id);g.crew_count=GAME_MAX_CREW;}
int main(void){
 setup();assert(!rewards_offer_weapon(&offer,-1)&&!rewards_offer_weapon(&offer,GAME_CATALOG_COUNT)&&!offer.kind);assert(rewards_offer_weapon(&offer,4));offered=offer;assert(!rewards_offer_weapon(&offer,1)&&!memcmp(&offer,&offered,sizeof offer));
 before=g;for(int i=0;i<10;i++)assert(rewards_can_accept(&offer,&g,cargo,4,-1,0));assert(!memcmp(&before,&g,sizeof g)&&!memcmp(&offer,&offered,sizeof offer));assert(rewards_accept(&offer,&g,cargo,4,-1,0)&&cargo[0]==4&&!offer.kind&&!memcmp(&before,&g,sizeof g));assert(!rewards_accept(&offer,&g,cargo,4,-1,0));
 for(int i=0;i<4;i++)cargo[i]=i;
 assert(rewards_offer_weapon(&offer,6));rejects(-1,0);rejects(-1,1);rejects(2,0);rejects(4,1);before=g;assert(rewards_accept(&offer,&g,cargo,4,2,1));assert(cargo[0]==0&&cargo[1]==1&&cargo[2]==6&&cargo[3]==3&&!memcmp(&before,&g,sizeof g));
 setup();before=g;assert(!rewards_offer_crew(&offer,&g,SPEC_COUNT)&&!memcmp(&before,&g,sizeof g));unsigned reserved=g.next_crew_id;assert(rewards_offer_crew(&offer,&g,SPEC_ROCK));assert(offer.recruit.id==reserved&&g.next_crew_id==reserved+1&&offer.recruit.health==150);offered=offer;before=g;assert(!rewards_offer_crew(&offer,&g,SPEC_HUMAN)&&!memcmp(&before,&g,sizeof g));
 for(int i=0;i<20;i++)assert(rewards_can_accept(&offer,&g,NULL,0,-1,0));
 assert(!memcmp(&offer,&offered,sizeof offer)&&!memcmp(&before,&g,sizeof g));assert(rewards_accept(&offer,&g,NULL,0,-1,0)&&g.crew_count==4&&g.crew[3].id==reserved&&g.crew[3].species==SPEC_ROCK&&g.crew[3].health==150&&!strcmp(g.crew[3].name,offered.recruit.name));
 setup();full();assert(rewards_offer_crew(&offer,&g,SPEC_ENGI));reserved=offer.recruit.id;rejects(-1,0);rejects(3,0);g.crew[3].ship=1;rejects(3,1);g.crew[3].ship=0;g.crew[3].species=SPEC_ZOLTAN;g.crew[3].health=0;g.crew[3].death_processed=0;rejects(-1,0);rejects(3,1);g.crew[3].death_processed=1;assert(rewards_accept(&offer,&g,cargo,4,-1,0)&&g.crew_count==8&&g.crew[3].id==reserved);
 /* Explicit replacement cannot erase a defender engaged with a hostile. */
 setup();full();assert(rewards_offer_crew(&offer,&g,SPEC_ZOLTAN));game_init_crew(&g,1,55,g.crew[4].room);g.enemy_crew[55].ship=0;g.enemy_crew_count=56;rejects(4,1);g.enemy_crew[55].health=0;g.enemy_crew[55].death_processed=1;before=g;offered=offer;assert(rewards_accept(&offer,&g,cargo,4,4,1));assert(g.crew[4].id==offered.recruit.id&&g.crew[4].species==SPEC_ZOLTAN);for(int i=0;i<8;i++)if(i!=4)assert(!memcmp(&before.crew[i],&g.crew[i],sizeof(Crew)));assert(!memcmp(before.enemy_crew,g.enemy_crew,sizeof g.enemy_crew));
 /* Explicit replacement of a last living crew is safe because joining is atomic. */
 setup();g.crew_count=1;assert(rewards_offer_crew(&offer,&g,SPEC_HUMAN));assert(rewards_accept(&offer,&g,NULL,0,0,1)&&g.crew_count==1&&game_alive_crew(&g)==1);
 /* No physical room means no insertion; a replaced occupant frees one cell. */
 setup();g.crew_count=1;g.player.room_count=1;g.player.rooms[0].w=g.player.rooms[0].h=1;assert(rewards_offer_crew(&offer,&g,SPEC_HUMAN));rejects(-1,0);assert(rewards_accept(&offer,&g,NULL,0,0,1)&&g.crew_count==1);
 setup();g.next_crew_id=REWARD_CREW_ID_LIMIT;before=g;assert(!rewards_offer_crew(&offer,&g,SPEC_HUMAN)&&!memcmp(&before,&g,sizeof g));g.next_crew_id=REWARD_CREW_ID_LIMIT-1;assert(rewards_offer_crew(&offer,&g,SPEC_HUMAN)&&g.next_crew_id==REWARD_CREW_ID_LIMIT);before=g;assert(rewards_decline(&offer)&&!offer.kind&&!memcmp(&before,&g,sizeof g));assert(!rewards_decline(&offer));
 puts("Rewards: bounded pending offer, no autoequip, exact confirmed replacement, atomic refusal, stable recruit ID/name/species, pending death/away/contested guards, physical placement and explicit decline passed.");
}
