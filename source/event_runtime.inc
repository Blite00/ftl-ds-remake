/* Bounded compiled original event execution. Source closure is validated by
 * tools/event_compile.py. Runtime queries never consume campaign randomness. */
#include "event_catalog.h"
#include "fire.h"
#include "breach.h"
#include "subsystems.h"
#define EVENT_RESOURCE_LIMIT 1000000
#define EVENT_MAX_DEPTH 16
/* V53: policy 6 enables the original sector beacon lists; policies 1-5 keep their
 * historical root ranges (6/8/9/11/12 roots) so older saves replay unchanged. */
static int imported_root_limit(const Campaign*c){return c->event_policy==1?6:c->event_policy==2?8:c->event_policy==3?9:c->event_policy==4?11:c->event_policy==5?12:IMPORTED_EVENT_ROOT_COUNT;}
static int imported_seen(const Campaign*c,int root){if(root<31)return (c->event_seen>>root)&1;root-=31;return root<256&&((c->event_seen_ext[root>>5]>>(root&31))&1u);}
static void imported_mark(Campaign*c,int root){if(root<31)c->event_seen|=1<<root;else{root-=31;if(root<256)c->event_seen_ext[root>>5]|=1u<<(root&31);}}
static int imported_active(const Campaign*c){return c->event_id==EVENT_IMPORTED&&c->mode==CAM_EVENT&&c->imported_node>=0&&c->imported_node<IMPORTED_EVENT_NODE_COUNT;}
int campaign_surrender_pending(const Campaign*c){return c&&imported_active(c)&&c->imported_battle==5&&!c->event_resolved;}
const char*campaign_text(const Campaign*c){return c->text;}
int campaign_imported_count(void){return IMPORTED_EVENT_ROOT_COUNT;}
const char*campaign_imported_id(int root){return root>=0&&root<IMPORTED_EVENT_ROOT_COUNT?imported_event_roots[root].id:"";}
static int imported_resource(const Campaign*c,int kind){
 switch(kind){case IE_FUEL:return c->fuel;case IE_SCRAP:return c->scrap;case IE_MISSILES:return c->game.missiles;case IE_DRONEPARTS:return c->game.drone_parts;case IE_HULL:return c->game.player.hull;default:return -1;}
}
/* Compute minimum/maximum immediate deltas across a random closure. Future
 * choices are separate transactions and do not gate their parent branch. */
static int imported_bounds(int index,int depth,int lo[5],int hi[5]){
 if(index<0||index>=IMPORTED_EVENT_NODE_COUNT||depth>=EVENT_MAX_DEPTH)return 0;
 const ImportedEventNode*n=&imported_event_nodes[index];
 for(int k=0;k<5;k++)lo[k]=hi[k]=0;
 for(int i=0;i<n->effect_count;i++){
  const ImportedEventEffect*e=&imported_event_effects[n->first_effect+i];
  if(e->kind<0||e->kind>IE_SECRET_SECTOR||(e->kind<=IE_CREW&&e->min>e->max))return 0;
  if(e->kind>=IE_WEAPON)continue;
  lo[e->kind]+=e->min;hi[e->kind]+=e->max;
 }
 if(n->alt_count){
  int low[5]={0},high[5]={0};
  for(int i=0;i<n->alt_count;i++){
   int a[5],b[5];if(!imported_bounds(imported_event_alternatives[n->first_alt+i],depth+1,a,b))return 0;
   for(int k=0;k<5;k++){if(!i||a[k]<low[k])low[k]=a[k];if(!i||b[k]>high[k])high[k]=b[k];}
  }
  for(int k=0;k<5;k++){lo[k]+=low[k];hi[k]+=high[k];}
 }
 return 1;
}
static int imported_affordable_mode(const Campaign*c,int node,int mandatory){
 int lo[5],hi[5];if(!imported_bounds(node,0,lo,hi))return 0;
 for(int k=0;k<5;k++){
  int amount=imported_resource(c,k);
  if(amount+lo[k]<(k==IE_HULL?1:0))return 0;
  if(!mandatory&&k!=IE_HULL&&amount+hi[k]>EVENT_RESOURCE_LIMIT)return 0;
 }
 return 1;
}
static int imported_affordable(const Campaign*c,int node){return imported_affordable_mode(c,node,(c->imported_battle==3||c->imported_battle==5));}
static int imported_requirement(const Campaign*c,const ImportedEventChoice*q){
 if(q->req_kind==IE_REQ_NONE)return 1;
 if(q->req_kind==IE_REQ_SPECIES){int count=0;for(int i=0;i<c->game.crew_count;i++){const Crew*r=&c->game.crew[i];if(r->health>0&&r->ship==0&&r->species==q->req_value)count++;}return count>=q->req_level;}
 if(q->req_kind==IE_REQ_SYSTEM){const Game*g=&c->game;int level=0;
  if(q->req_value>=0&&q->req_value<GAME_SYSTEM_COUNT)level=g->player.systems[q->req_value].capacity;
  else if(q->req_value==5)level=g->subsystems[0][SUB_SENSORS].capacity;
  else if(q->req_value==6)level=g->subsystems[0][SUB_DOORS].capacity;
  else if(q->req_value==7)level=g->teleporter[0].installed?g->teleporter[0].capacity:0;
  else if(q->req_value==8)level=g->drones.installed?g->drones.capacity:0;
  else if(q->req_value==9)level=1; /* The Kestrel cockpit is always installed. */
  return level>=q->req_level;}
 if(q->req_kind==IE_REQ_WEAPON){int count=0;for(int i=0;i<c->game.weapon_count;i++)if(c->game.weapons[i].catalog_id==q->req_value)count++;return count>=q->req_level;}
 if(q->req_kind==IE_REQ_NEVER)return 0;
 if(q->req_kind==IE_REQ_AUGMENT)return game_augment_count(&c->game,q->req_value)>0;
 /* V56 drone blue options: a schematic installed or carried in drone cargo. */
 if(q->req_kind==IE_REQ_DRONE||q->req_kind==IE_REQ_DRONE_ANY){int count=0;
  if(q->req_kind==IE_REQ_DRONE_ANY&&(q->req_value<0||q->req_value>=(int)(sizeof imported_event_req_lists/sizeof imported_event_req_lists[0])))return 0;
  for(int i=0;i<GAME_MAX_DRONES+CAM_CARGO_COUNT;i++){int id=i<GAME_MAX_DRONES?(c->game.drones.installed&&i<c->game.drones.slot_count?c->game.drones.slots[i].catalog_id:-1):c->drone_cargo[i-GAME_MAX_DRONES];
   if(id<0)continue;
   if(q->req_kind==IE_REQ_DRONE)count+=id==q->req_value;
   else for(int k=0;k<IMPORTED_EVENT_REQ_LIST_WIDTH&&imported_event_req_lists[q->req_value][k]>=0;k++)count+=id==imported_event_req_lists[q->req_value][k];}
  return count>=q->req_level;}
 if(q->req_kind==IE_REQ_WEAPON_ANY){int count=0;if(q->req_value<0||q->req_value>=(int)(sizeof imported_event_req_lists/sizeof imported_event_req_lists[0]))return 0;
  for(int i=0;i<c->game.weapon_count;i++)for(int k=0;k<IMPORTED_EVENT_REQ_LIST_WIDTH&&imported_event_req_lists[q->req_value][k]>=0;k++)if(c->game.weapons[i].catalog_id==imported_event_req_lists[q->req_value][k])count++;
  return count>=q->req_level;}
 return 0;
}
static const ImportedEventChoice*imported_choice(const Campaign*c,int choice){
 if(!imported_active(c)||c->event_resolved||(c->imported_battle==1||c->imported_battle==4))return 0;
 const ImportedEventNode*n=&imported_event_nodes[c->imported_node];
 return choice>=0&&choice<n->choice_count?&imported_event_choices[n->first_choice+choice]:0;
}
int campaign_choice_blue(const Campaign*c,int choice){const ImportedEventChoice*q=imported_choice(c,choice);return q&&q->req_kind!=IE_REQ_NONE;}
/* Queries must match execution guards without drawing random alternatives. */
static int imported_needs_home(int node,int depth){
 if(node<0||node>=IMPORTED_EVENT_NODE_COUNT||depth>=EVENT_MAX_DEPTH)return 1;
 const ImportedEventNode*n=&imported_event_nodes[node];
 if(n->hostile==1)return 1;
 for(int i=0;i<n->alt_count;i++)if(imported_needs_home(imported_event_alternatives[n->first_alt+i],depth+1))return 1;
 return 0;
}
static int imported_accepts(int node,int depth){
 if(node<0||node>=IMPORTED_EVENT_NODE_COUNT||depth>=EVENT_MAX_DEPTH)return 0;
 const ImportedEventNode*n=&imported_event_nodes[node];
 if(n->hostile==0)return 1;
 for(int i=0;i<n->alt_count;i++)if(imported_accepts(imported_event_alternatives[n->first_alt+i],depth+1))return 1;
 return 0;
}
static int imported_enabled(const Campaign*c,int choice){
 if(imported_active(c)&&!c->event_resolved&&c->imported_battle==4)return choice==0;
 if(imported_active(c)&&!c->event_resolved&&c->imported_battle==1)return choice==0&&!game_away_crew(&c->game)&&c->imported_ship>=0&&c->imported_ship<IMPORTED_EVENT_SHIP_COUNT;
 const ImportedEventChoice*q=imported_choice(c,choice);
 if(!q||!imported_requirement(c,q)||!imported_affordable(c,q->next))return 0;
 if(c->imported_battle==5)return !imported_accepts(q->next,0)||game_can_accept_surrender(&c->game);
 return !game_away_crew(&c->game)||!imported_needs_home(q->next,0);
}
/* V63: the hidden Crystal sector replaces the next sector, so it needs a following
 * sector before the Last Stand (and the route metadata of current runs). */
static int secret_sector_open(const Campaign*c){return c->route_policy==1&&c->sector>=0&&c->sector<c->sector_count-2&&!campaign_finale_active(c);}
static int imported_random(unsigned int*rng,int n){*rng=*rng*1664525u+1013904223u;return (int)((*rng>>8)%(unsigned)n);}
static int imported_enter(Campaign*c,int node){
 unsigned int rng=c->rng;int delta[5]={0},object_kind=-1,object_value=-1,depth=0,ship=c->imported_ship,hostile=-1,capped=0,pursuit=0,surrender=c->imported_battle==5;const ImportedEventNode*n=0;const char*story="";
 int harm[8][3],harms=0; /* V53 deferred ship damage: kind, a, b. */
 int env=0; /* V60 beacon hazard named by the event. */
 int named=-1,named_skill=0; /* V62 named crew reward. */
 if(c->reward.kind!=REWARD_NONE||!imported_affordable(c,node))return 0;
 for(;depth<EVENT_MAX_DEPTH;depth++){
  if(node<0||node>=IMPORTED_EVENT_NODE_COUNT)return 0;
  n=&imported_event_nodes[node];if(n->text[0])story=n->text;
  if(n->ship>=0){if(n->ship>=IMPORTED_EVENT_SHIP_COUNT)return 0;ship=n->ship;}
  if(n->hostile>=0)hostile=n->hostile;
  if(n->env)env=n->env;
  pursuit+=n->pursuit;
  for(int i=0;i<n->effect_count;i++){
   const ImportedEventEffect*e=&imported_event_effects[n->first_effect+i];
   if(e->kind==IE_NAMED_CREW){if(object_kind>=0)return 0;object_kind=IE_CREW;object_value=e->min%10;named=e->max;named_skill=e->min/10;continue;}
   if(e->kind>=IE_AUGMENT&&e->kind!=IE_BOARDERS&&e->kind!=IE_STATUS){if(harms<8){harm[harms][0]=e->kind;harm[harms][1]=e->min;harm[harms][2]=0;harms++;}continue;}
   if(e->kind==IE_SYSDAMAGE||e->kind==IE_SHIPHAZARD||e->kind==IE_BOARDERS||e->kind==IE_STATUS){if(harms<8){harm[harms][0]=e->kind;harm[harms][1]=e->min;harm[harms][2]=e->max;harms++;}continue;}
   int amount=e->min+(e->min==e->max?0:imported_random(&rng,e->max-e->min+1));
   /* Random weapon rewards draw an ordinary store-rarity (1-3) player weapon. */
   if(e->kind==IE_WEAPON&&e->min<0){int pool[GAME_CATALOG_COUNT],count=0;for(int id=0;id<GAME_CATALOG_COUNT;id++)if(game_weapon_player(id)&&game_weapon_rarity(id)>=1&&game_weapon_rarity(id)<=3)pool[count++]=id;amount=count?pool[imported_random(&rng,count)]:2;}
   if(e->kind>=IE_WEAPON){if(object_kind>=0||e->kind>IE_CREW)return 0;object_kind=e->kind;object_value=amount;}
   else delta[e->kind]+=amount;
  }
  if(!n->alt_count)break;
  node=imported_event_alternatives[n->first_alt+imported_random(&rng,n->alt_count)];
 }
 if(depth==EVENT_MAX_DEPTH||!n)return 0;
 if(!surrender&&hostile==1&&(ship<0||ship>=IMPORTED_EVENT_SHIP_COUNT||game_away_crew(&c->game)))return 0;
 if(c->imported_battle==3||surrender)for(int k=0;k<IE_HULL;k++)if(imported_resource(c,k)+delta[k]>EVENT_RESOURCE_LIMIT){delta[k]=EVENT_RESOURCE_LIMIT-imported_resource(c,k);capped=1;}
 if(c->game.player.hull+delta[IE_HULL]>c->game.player.max_hull)delta[IE_HULL]=c->game.player.max_hull-c->game.player.hull;
 if(surrender&&hostile==0&&!game_can_accept_surrender(&c->game))return 0;
 PendingReward pending={0};
 if(object_kind==IE_WEAPON&&!rewards_offer_weapon(&pending,object_value))return 0;
 if(object_kind==IE_CREW&&!rewards_offer_crew(&pending,&c->game,object_value))return 0;
 if(object_kind==IE_CREW&&named>=0&&named<IMPORTED_EVENT_CREW_NAME_COUNT){if(*imported_event_crew_names[named])snprintf(pending.recruit.name,sizeof pending.recruit.name,"%s",imported_event_crew_names[named]);
  for(int k=0;k<4;k++){pending.recruit.skill[k]=named_skill*60*GAME_HZ;}}
 /* Acceptance and its resources commit together, after all bounded effects
  * and reward preflight have succeeded. Refusal never rebuilds the ship. */
 if(surrender&&hostile==0&&!game_accept_surrender(&c->game))return 0;
 c->reward=pending;
 c->fuel+=delta[IE_FUEL];c->scrap+=delta[IE_SCRAP];c->game.missiles+=delta[IE_MISSILES];c->game.drone_parts+=delta[IE_DRONEPARTS];c->game.player.hull+=delta[IE_HULL];
 char augment_note[64]="";
 for(int h=0;h<harms;h++){Game*g=&c->game;int target=harm[h][2];
  /* V55 augment rewards: fit into a free slot, else sold at once for half price. */
  if(harm[h][0]==IE_AUGMENT){int id=harm[h][1];if(id<0){int pool[AUG_COUNT],n=0;for(int a=0;a<AUG_COUNT;a++)if(game_augment_rarity(a)>=1)pool[n++]=a;id=n?pool[imported_random(&rng,n)]:AUG_SCRAP_COLLECTOR;}
   if(game_add_augment(g,id)>=0)snprintf(augment_note,sizeof augment_note,"\n\nAugment installed: %s.",game_augment_name(id));
   else{c->scrap+=game_augment_cost(id)/2;snprintf(augment_note,sizeof augment_note,"\n\n%s sold: augment slots full.",game_augment_name(id));}continue;}
  /* V56 drone rewards: a schematic into drone cargo, else sold for half price. */
  /* V62 store / crew loss / boarders / system status. */
  if(harm[h][0]==IE_STORE){c->node_kind[c->current]=NODE_STORE;c->store_after_event=1;continue;}
  if(harm[h][0]==IE_REMOVECREW){int alive[GAME_MAX_CREW],n=0;for(int i=0;i<g->crew_count;i++){if(g->crew[i].health>0&&g->crew[i].ship==0)alive[n++]=i;}
   if(n>1){int k=alive[imported_random(&rng,n)];char name[GAME_CREW_NAME_SIZE];snprintf(name,sizeof name,"%s",g->crew[k].name);
    for(int i=k;i<g->crew_count-1;i++){g->crew[i]=g->crew[i+1];}memset(&g->crew[--g->crew_count],0,sizeof g->crew[0]);species_refresh_power(g);
    snprintf(augment_note,sizeof augment_note,"%c%c%.20s is gone.",10,10,name);}continue;}
  if(harm[h][0]==IE_BOARDERS){int lo=harm[h][1]%100,hi=harm[h][2]%100,species=harm[h][1]/100,count=lo+(hi>lo?imported_random(&rng,hi-lo+1):0);
   if(species>=SPEC_COUNT)species=-1;
   if(hostile==1){c->pending_boarders=count;c->pending_boarder_species=species;}else game_spawn_intruders(g,count,species);continue;}
  if(harm[h][0]==IE_STATUS){int code=harm[h][1];game_apply_status(g,code/1000,(code/100)%10,code%100,harm[h][2]);continue;}
  /* V61 quests: mark a beacon (or the next sector) for the quest event. */
  if(harm[h][0]==IE_QUEST){int placed=campaign_quest_add(c,harm[h][1]);snprintf(augment_note,sizeof augment_note,"%c%c%s",10,10,placed==1?"A quest beacon is marked on the map.":placed==2?"The quest beacon will appear in the next sector.":"Too many open quests; this one is lost.");continue;}
  /* V63 <secretSector/>: the jump happens when this (choice-free) event is closed. */
  if(harm[h][0]==IE_SECRET_SECTOR){c->secret_jump=secret_sector_open(c);snprintf(augment_note,sizeof augment_note,"%c%c%s",10,10,c->secret_jump?"Course set for the hidden Crystal sector.":"The Crystal sector is out of reach from here.");continue;}
  /* V57 hangar unlocks are merged into the SD profile by the caller. */
  if(harm[h][0]==IE_UNLOCK){int layout=harm[h][1]*2;if(layout>=0&&layout<game_layout_count()){c->unlock_pending|=1u<<layout;snprintf(augment_note,sizeof augment_note,"%c%cShip unlocked: %.40s.",10,10,game_layout_name(layout));}continue;}
  if(harm[h][0]==IE_DRONE){int id=harm[h][1],slot=-1;if(id<0){int pool[GAME_DRONE_CATALOG_COUNT],n=0;for(int a=0;a<GAME_DRONE_CATALOG_COUNT;a++)if(drones_rarity(a)>=1)pool[n++]=a;id=n?pool[imported_random(&rng,n)]:0;}
   for(int k=0;k<CAM_CARGO_COUNT&&slot<0;k++)if(c->drone_cargo[k]<0)slot=k;
   if(slot>=0){c->drone_cargo[slot]=id;snprintf(augment_note,sizeof augment_note,"\n\nDrone schematic received: %s.",drones_name(id));}
   else{c->scrap+=drones_price(id)/2;snprintf(augment_note,sizeof augment_note,"\n\n%s sold: drone cargo full.",drones_name(id));}continue;}
  if(harm[h][0]==IE_SYSDAMAGE){int sys=target<GAME_SYSTEM_COUNT?target:imported_random(&rng,GAME_SYSTEM_COUNT);GameSystemState*y=&g->player.systems[sys];if(y->capacity>0){y->health-=harm[h][1];if(y->health<0)y->health=0;if(y->power>y->health)y->power=y->health;}}
  else{int room;if(target<GAME_SYSTEM_COUNT&&g->player.systems[target].room>=0)room=g->player.systems[target].room;else room=g->player.rooms[imported_random(&rng,g->player.room_count)].id;
   int kind=harm[h][1]==3?1+imported_random(&rng,2):harm[h][1];if(kind==1)fire_ignite(&g->player,room,1);else breach_add(&g->player,room,1);}}
 int fleet_advance=pursuit;
 if(fleet_advance>1000000-c->sector_jumps)fleet_advance=1000000-c->sector_jumps;
 if(fleet_advance<-c->sector_jumps)fleet_advance=-c->sector_jumps; /* V61: fleet delays cannot go below zero. */
 c->sector_jumps+=fleet_advance;
 if(env)c->beacon_env=env;
 c->rng=rng;c->imported_ship=ship;if(surrender&&hostile==0)c->imported_battle=3;else if(!surrender&&hostile==1)c->imported_battle=1;c->imported_node=node;c->mode=CAM_EVENT;c->event_id=EVENT_IMPORTED;c->event_resolved=n->choice_count==0&&c->imported_battle!=1;
 snprintf(c->text,sizeof c->text,"%s",*story?story:(surrender&&hostile==0?"Surrender accepted. Both ships cease fire.":c->event_resolved?"You prepare to continue.":"Choose your response."));
 int changed=0;for(int k=0;k<5;k++)changed|=delta[k]!=0;
 if(changed||capped){size_t len=strlen(c->text);snprintf(c->text+len,sizeof c->text-len,"\n\nReceived / spent: fuel %+d, scrap %+d, missiles %+d, drone parts %+d, hull %+d.",delta[0],delta[1],delta[2],delta[3],delta[4]);}
 if(augment_note[0]){size_t len=strlen(c->text);snprintf(c->text+len,sizeof c->text-len,"%s",augment_note);}
 if(pursuit){size_t len=strlen(c->text);snprintf(c->text+len,sizeof c->text-len,fleet_advance<0?"\n\nFleet delayed %d turn(s).":"\n\nFleet advanced %d turn(s).",fleet_advance<0?-fleet_advance:fleet_advance);}
 if(surrender&&hostile!=0&&!n->choice_count){c->imported_battle=2;c->mode=CAM_COMBAT;c->event_resolved=1;}
 if(capped){size_t len=strlen(c->text);snprintf(c->text+len,sizeof c->text-len,"\nStorage limit reached; excess supplies were left behind.");}
 return 1;
}
static int imported_escape_warning(Campaign*c){
 if(c->imported_escape_warned)return 0;
 if(c->imported_ship<0||c->imported_ship>=IMPORTED_EVENT_SHIP_COUNT)return 0;
 int node=imported_event_ships[c->imported_ship].escape_node;
 if(node<0||node>=IMPORTED_EVENT_NODE_COUNT)return 0;
 /* Compiler permits only warning text and redundant hostility, no choices/effects. */
 c->imported_escape_warned=1;c->imported_node=node;c->imported_battle=4;c->mode=CAM_EVENT;c->event_resolved=0;
 snprintf(c->text,sizeof c->text,"%s",imported_event_nodes[node].text);
 return 1;
}
/* XML supplies chance/threshold bounds but not draw timing. Sample once at
 * encounter start; certainty consumes no extra RNG, preserving older roots. */
static int imported_chance(Campaign*c,int chance){return chance>=10000||(chance>0&&bounded(c,10000)<chance);}
static int imported_surrender_offer(Campaign*c){
 if(c->imported_battle!=2||c->imported_surrender_threshold<0||c->game.outcome!=GAME_PLAYING||c->game.paused||c->game.enemy_departed||c->game.enemy.hull<=0||c->game.enemy.hull>c->imported_surrender_threshold)return 0;
 if(c->imported_ship<0||c->imported_ship>=IMPORTED_EVENT_SHIP_COUNT)return 0;
 int node=imported_event_ships[c->imported_ship].surrender_node;
 c->imported_surrender_threshold=-1;c->imported_battle=5;
 if(node>=0&&imported_enter(c,node))return 1;
 c->imported_battle=2;return 0;
}
static int imported_engage(Campaign*c){
 if(!imported_enabled(c,0)||c->imported_battle!=1)return 0;
 const ImportedEventShip*s=&imported_event_ships[c->imported_ship];
 if(s->enemy_count<=0)return 0;
 unsigned old_rng=c->rng;int kind=imported_event_enemies[s->first_enemy+bounded(c,s->enemy_count)];
 if(kind>=ENEMY_SHIP_BASE){pending_ship=kind-ENEMY_SHIP_BASE;kind=ENEMY_SHIP;}
 if(!campaign_begin_enemy(c,kind,0)){c->rng=old_rng;return 0;}
 c->mode=CAM_COMBAT;c->event_resolved=1;c->result_ticks=0;c->imported_battle=2;
 /* V61 crew mix (<crew>): each enemy crewmember's species is drawn by the given shares. */
 if(s->crew0>=0){int total=s->crew0_percent+s->crew1_percent+s->crew2_percent;unsigned rng=c->rng;
  for(int i=0;i<c->game.enemy_crew_count&&i<GAME_MAX_ENEMY_CREW&&total>0;i++){int roll=imported_random(&rng,total),species=s->crew0;
   if(roll>=s->crew0_percent){species=s->crew1;if(roll>=s->crew0_percent+s->crew1_percent)species=s->crew2;}
   if(species>=0&&c->game.enemy_crew[i].health>0&&c->game.enemy_crew[i].ship==1)game_set_crew_species(&c->game,1,i,species);}
  c->rng=rng;}
 c->imported_escape_warned=0;c->imported_surrender_threshold=-1;
 if(s->surrender_node>=0&&imported_chance(c,s->surrender_chance))
  c->imported_surrender_threshold=s->surrender_min+(s->surrender_max>s->surrender_min?bounded(c,s->surrender_max-s->surrender_min+1):0);
 if(s->escape_timer>0&&imported_chance(c,s->escape_chance)){
  game_enemy_escape_start(&c->game,s->escape_timer*GAME_HZ,s->escape_min,s->escape_max);
  if(c->game.enemy_escape_active)imported_escape_warning(c);
 }
 return 1;
}
/* Callback ownership stays with this event through its explicit reward choices.
 * Forced reward delivery clamps positive supplies at the DS storage bound. */
static void imported_outcome(Campaign*c){
 int node=-1;
 if(c->imported_ship>=0&&c->imported_ship<IMPORTED_EVENT_SHIP_COUNT){
  const ImportedEventShip*s=&imported_event_ships[c->imported_ship];
  if(c->game.result_reason==RESULT_ENEMY_ESCAPE)node=s->gotaway;
  else if(c->game.result_reason==RESULT_ENEMY_HULL)node=s->destroyed;
  else if(c->game.result_reason==RESULT_ENEMY_CREW)node=s->dead_crew;
 }
 c->imported_battle=3;
 if(node>=0&&imported_enter(c,node))return;
 c->mode=CAM_EVENT;c->event_id=EVENT_IMPORTED;c->event_resolved=1;
 snprintf(c->text,sizeof c->text,"The encounter is over. No further reward is available.");
}
static int imported_begin(Campaign*c,int root,int arrival){
 if((!arrival&&c->mode!=CAM_MAP)||(c->event_policy<1||c->event_policy>6)||c->route_policy!=1||root<0||root>=imported_root_limit(c)||game_away_crew(&c->game))return 0;
 const ImportedEventRoot*r=&imported_event_roots[root];
 if((r->fuel_only&&c->fuel!=0)||(r->unique&&imported_seen(c,root)))return 0;
 int old_ship=c->imported_ship,old_battle=c->imported_battle;
 c->imported_ship=-1;c->imported_battle=0;
 if(!imported_enter(c,r->node)){c->imported_ship=old_ship;c->imported_battle=old_battle;return 0;}
 imported_mark(c,root);return 1;
}
/* V61 quest beacons: an unvisited beacon of this sector (not the start, exit or a
 * store, not already a quest) becomes a QUEST beacon for the event. */
/* Hidden (homeworld) slots draw from their own seed-derived stream so they never
 * shift the run's random sequence. */
static int quest_place(Campaign*c,int slot){int pool[CAM_NODE_COUNT],n=0;int hidden=!!(c->quest_hidden&(1u<<slot));unsigned rng=hidden?(c->seed*2654435761u)^((unsigned)c->sector*40503u)^((unsigned)slot*97u):c->rng;
 for(int i=1;i<c->node_count-1;i++){int k=c->node_kind[i],taken=0;
  if(c->visited[i]||i==c->current||k==NODE_STORE||k==NODE_EXIT||k==NODE_START)continue;
  if(hidden&&(i==1||i==2||i==20))continue; /* Scripted first fight, supply and repair beacons keep their roles. */
  for(int q=0;q<CAM_QUESTS;q++){taken|=q!=slot&&c->quest_event[q]>=0&&c->quest_node[q]==i;}
  if(!taken)pool[n++]=i;}
 if(!n){c->quest_node[slot]=-1;return 0;}
 c->quest_node[slot]=pool[imported_random(&rng,n)];if(!hidden){c->rng=rng;c->node_kind[c->quest_node[slot]]=NODE_QUEST;}return 1;}
int campaign_quest_add(Campaign*c,int event){
 if(event<0||event>=IMPORTED_EVENT_NODE_COUNT)return 0;
 for(int q=0;q<CAM_QUESTS;q++)if(c->quest_event[q]<0){c->quest_event[q]=event;c->quest_hidden&=~(1u<<q);return quest_place(c,q)?1:2;}
 return 0;}
/* V62 homeworld specials: on reaching a sector, each special of its type not yet
 * placed this run has a 1 in 3 chance (the original homeworld sectors are one
 * variant of their people's sectors) to sit, unmarked, at one of its beacons. */
static void quests_place_specials(Campaign*c){unsigned rng=(c->seed^0x484f4d45u)*2246822519u+(unsigned)c->sector*3266489917u;int bit=0,type;
 if(c->event_policy!=6||c->route_policy!=1||c->sector<1)return; /* Never the starting sector, as in the original. */
 type=c->sector_types[c->sector];
 for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++){const ImportedEventPool*e=&imported_event_pools[i];if(e->category!=7)continue;
  if(e->sector==type&&!(c->specials_done&(1u<<bit))&&imported_random(&rng,3)==0){
   for(int q=0;q<CAM_QUESTS;q++)if(c->quest_event[q]<0){c->quest_event[q]=imported_event_roots[e->root].node;c->quest_hidden|=1u<<q;quest_place(c,q);c->specials_done|=1u<<bit;break;}}
  bit++;}}
int campaign_event_node_count(void){return IMPORTED_EVENT_NODE_COUNT;}
unsigned campaign_event_catalog_hash(void){return IMPORTED_EVENT_CATALOG_HASH;}
int campaign_quest_at(const Campaign*c,int node){for(int q=0;q<CAM_QUESTS;q++)if(c->quest_event[q]>=0&&c->quest_node[q]==node&&node>=0)return q;return -1;}
static void quests_new_sector(Campaign*c){
 for(int q=0;q<CAM_QUESTS;q++)if(c->quest_event[q]>=0){if(c->quest_node[q]>=0){c->quest_event[q]=c->quest_node[q]=-1;c->quest_hidden&=~(1u<<q);}else quest_place(c,q);}
 quests_place_specials(c);}
static int quest_arrive(Campaign*c,int node){int q=campaign_quest_at(c,node),event,old_ship=c->imported_ship,old_battle=c->imported_battle;
 if(q<0)return 0;
 event=c->quest_event[q];c->quest_event[q]=c->quest_node[q]=-1;c->quest_hidden&=~(1u<<q);c->imported_ship=-1;c->imported_battle=0;
 if(!imported_enter(c,event)){c->imported_ship=old_ship;c->imported_battle=old_battle;return 0;}
 return 1;}
int campaign_imported_begin(Campaign*c,int root){if(c->reward.kind!=REWARD_NONE)return 0;return imported_begin(c,root,0);}
static int imported_pool(Campaign*c,int kind){
 int pool[IMPORTED_EVENT_ROOT_COUNT],count=0;
 if(c->event_policy<1||c->event_policy>6)return 0;
 /* V53: current runs draw beacon events from the original sector lists:
  * fights from HOSTILE_*, supply beacons from NEUTRAL/ITEMS/NOTHING_*, distress
  * from DISTRESS_BEACON_*, quest beacons from QUESTS_* (else neutral). */
 if(c->event_policy==6&&kind!=NODE_REPAIR){int type=c->sector_types[c->sector],want[3]={-1,-1,-1};
  if(type==SECTOR_FINAL||campaign_finale_active(c))return 0; /* The Last Stand keeps its own beacons. */
  if(type<0||(type>SECTOR_PIRATE&&type!=SECTOR_SECRET))type=SECTOR_UNCHARTED;
  if(kind==NODE_FIGHT){want[0]=0;}else if(kind==NODE_HAZARD){want[0]=6;}else if(kind==NODE_SUPPLY){want[0]=1;want[1]=4;want[2]=5;}else if(kind==NODE_DISTRESS){want[0]=2;}else if(kind==NODE_QUEST){want[0]=3;want[1]=1;}else return 0;
  for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++){const ImportedEventPool*e=&imported_event_pools[i];
   if(e->sector!=type||(e->category!=want[0]&&e->category!=want[1]&&e->category!=want[2]))continue;
   const ImportedEventRoot*r=&imported_event_roots[e->root];if(r->fuel_only||(r->unique&&imported_seen(c,e->root)))continue;
   if(count<IMPORTED_EVENT_ROOT_COUNT)pool[count++]=e->root;}
  /* Supply beacons also keep the reviewed V29 station/trade roots (0-5, 9). */
  if(kind==NODE_SUPPLY)for(int i=0;i<12&&count<IMPORTED_EVENT_ROOT_COUNT;i++){const ImportedEventRoot*r=&imported_event_roots[i];
   if((i>=6&&i!=9)||r->fuel_only||(r->unique&&imported_seen(c,i))||!strcmp(r->id,"REPAIR_STATION")){continue;}pool[count++]=i;}
  if(!count)return 0;
  return imported_begin(c,pool[bounded(c,count)],1);
 }
 int limit=imported_root_limit(c);
 for(int i=0;i<limit;i++){
  const ImportedEventRoot*r=&imported_event_roots[i];
  if(r->fuel_only||(r->unique&&imported_seen(c,i)))continue;
  /* Authored placement: original repair at optional repair beacons; trade
   * events at optional supply beacons. Guaranteed service anchors stay intact. */
  if(kind==NODE_FIGHT){if(c->event_policy<2||c->sector<1||i<6||i==9)continue;}
  else {if((i>=6&&i!=9)||(kind!=NODE_REPAIR&&kind!=NODE_SUPPLY))continue;
   if((kind==NODE_REPAIR)!=(!strcmp(r->id,"REPAIR_STATION")))continue;
  }
  pool[count++]=i;
 }
 if(!count)return 0;
 return imported_begin(c,pool[bounded(c,count)],1);
}

void campaign_choice_hint(const Campaign*c,int choice,char*buffer,int size){
 static const char*const names[]={"FUEL","SCRAP","AMMO","PARTS","HULL"};
 const ImportedEventChoice*q=imported_choice(c,choice);int lo[5],hi[5],used=0,capped=0;
 if(size<=0)return;
 buffer[0]=0;if(!q||!imported_bounds(q->next,0,lo,hi))return;
 if(c->imported_battle==5&&imported_accepts(q->next,0)&&!game_can_accept_surrender(&c->game)){snprintf(buffer,(size_t)size,"Cannot safely exchange boarding parties");return;}
 if(c->imported_battle==3||c->imported_battle==5)for(int k=0;k<5;k++){
  int room=(k==IE_HULL?c->game.player.max_hull:EVENT_RESOURCE_LIMIT)-imported_resource(c,k);
  if(lo[k]>room){lo[k]=room;capped=1;}
  if(hi[k]>room){hi[k]=room;capped=1;}
 }
 for(int k=0;k<5;k++)if(lo[k]||hi[k]){
  if(used>=size-1)break;
  int n=lo[k]==hi[k]?snprintf(buffer+used,(size_t)(size-used),"%s%s %+d",used?" / ":"",names[k],lo[k]):snprintf(buffer+used,(size_t)(size-used),"%s%s %+d..%+d",used?" / ":"",names[k],lo[k],hi[k]);
  if(n>0)used+=n;
 }
 if(!used&&capped)snprintf(buffer,(size_t)size,"Storage full: no supplies gained");
}
