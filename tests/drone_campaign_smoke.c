#include "campaign.h"
#include "drones.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;
static void unchanged(void){assert(!memcmp(&c,&before,sizeof c));}
static void store(void){campaign_init_legacy_seed(&c,210021);c.mode=CAM_STORE;c.current=5;c.scrap=500;}
static int count(void){int n=0;for(int i=0;i<CAM_CARGO_COUNT;i++)n+=c.drone_cargo[i]>=0;for(int i=0;i<c.game.drones.slot_count;i++)n+=c.game.drones.slots[i].catalog_id>=0;return n;}
int main(void){
 store();assert(CAM_OFFER_COUNT==17);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++){assert(c.stock[i][11]==1&&c.stock[i][12]==1&&c.stock[i][13]==3);}
 for(int i=0;i<CAM_CARGO_COUNT;i++)assert(c.drone_cargo[i]==-1);
 assert(campaign_offer_price(&c,11)==85&&campaign_offer_price(&c,12)==50&&campaign_offer_price(&c,13)==24);
 assert(campaign_purchase(&c,11));assert(c.scrap==415&&c.game.drone_parts==4&&c.stock[5][11]==0);
 assert(c.game.drones.installed&&c.game.drones.capacity==2&&c.game.drones.health==2&&c.game.drones.power==0&&c.game.drones.slots[0].catalog_id==0);
 before=c;assert(!campaign_purchase(&c,11));unchanged();
 c.current=6;before=c;assert(!campaign_purchase(&c,11));unchanged();c.current=5;
 assert(campaign_purchase(&c,12)&&count()==2&&c.drone_cargo[0]==0&&c.scrap==365);
 assert(campaign_drone_swap(&c,0,1)&&count()==2&&c.drone_cargo[0]==-1);
 c.game.drones.slots[0].charge_ticks=19;c.game.drones.slots[1].charge_ticks=37;
 assert(campaign_drone_reorder(&c,0,1)&&c.game.drones.slots[0].charge_ticks==37&&c.game.drones.slots[1].charge_ticks==19);
 assert(campaign_drone_unequip(&c,1)&&count()==2&&c.drone_cargo[0]==0&&c.game.drones.slots[1].catalog_id==-1);
 assert(campaign_drone_swap(&c,0,0)&&count()==2&&c.drone_cargo[0]==0&&c.game.drones.slots[0].charge_ticks==0);
 int parts=c.game.drone_parts;assert(campaign_purchase(&c,13)&&c.game.drone_parts==parts+3&&c.stock[5][13]==2);
 assert(campaign_purchase(&c,13)&&campaign_purchase(&c,13));before=c;assert(!campaign_purchase(&c,13));unchanged();
 assert(campaign_drone_sell_price(&c,0,0)==25&&campaign_drone_sell_price(&c,1,0)==25);
 int scrap=c.scrap;assert(campaign_sell_drone(&c,0,0)&&count()==1&&c.scrap==scrap+25);
 before=c;assert(!campaign_sell_drone(&c,0,0));unchanged();
 scrap=c.scrap;assert(campaign_sell_drone(&c,1,0)&&count()==0&&c.scrap==scrap+25);
 assert(c.game.drones.installed&&c.game.drone_parts==13);
 /* Full cargo cannot erase an owned installed schematic or spend stock. */
 store();assert(campaign_purchase(&c,11));for(int i=0;i<CAM_CARGO_COUNT;i++)c.drone_cargo[i]=0;
 before=c;assert(!campaign_purchase(&c,12));unchanged();assert(!campaign_drone_unequip(&c,0));unchanged();
 assert(campaign_drone_swap(&c,3,0)&&count()==5);
 /* Every mutator obeys its management/store boundary and fails atomically. */
 for(int mode=CAM_MAP;mode<=CAM_STORE;mode++){
  c.mode=mode;c.event_resolved=0;
  if(mode==CAM_MAP||mode==CAM_STORE)continue;
  before=c;assert(!campaign_purchase(&c,13));unchanged();assert(!campaign_drone_swap(&c,0,0));unchanged();
  assert(!campaign_drone_unequip(&c,0));unchanged();assert(!campaign_drone_reorder(&c,0,1));unchanged();
  assert(!campaign_upgrade_drones(&c));unchanged();assert(!campaign_sell_drone(&c,0,0));unchanged();
 }
 c.mode=CAM_EVENT;c.event_resolved=1;c.drone_cargo[0]=-1;assert(campaign_drone_unequip(&c,0));assert(campaign_drone_swap(&c,0,0));
 before=c;assert(!campaign_sell_drone(&c,1,0));unchanged();
 c.mode=CAM_MAP;c.scrap=1000;c.game.drones.health=1;
 for(int cap=2;cap<8;cap++){int price=20+10*cap;assert(campaign_drone_upgrade_price(&c)==price);scrap=c.scrap;assert(campaign_upgrade_drones(&c)&&c.scrap==scrap-price&&c.game.drones.capacity==cap+1&&c.game.drones.health==cap);}
 before=c;assert(campaign_drone_upgrade_price(&c)==-1&&!campaign_upgrade_drones(&c));unchanged();
 store();c.scrap=84;before=c;assert(!campaign_purchase(&c,11));unchanged();
 c.scrap=500;c.game.drone_parts=999999;before=c;assert(!campaign_purchase(&c,11));unchanged();assert(!campaign_purchase(&c,13));unchanged();
 c.game.drone_parts=999998;assert(campaign_purchase(&c,11)&&c.game.drone_parts==1000000);
 c.scrap=999976;before=c;assert(!campaign_sell_drone(&c,1,0));unchanged();
 c.scrap=999975;assert(campaign_sell_drone(&c,1,0)&&c.scrap==1000000);
 before=c;assert(!campaign_drone_swap(&c,-1,0));unchanged();assert(!campaign_drone_swap(&c,0,3));unchanged();
 assert(!campaign_drone_unequip(&c,3));unchanged();assert(!campaign_drone_reorder(&c,-1,0));unchanged();
 assert(campaign_drone_sell_price(&c,2,0)==-1&&campaign_drone_sell_price(&c,0,4)==-1);
 /* Separate cargo families: drone purchases work with full weapon cargo. */
 store();for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=1;
 assert(campaign_purchase(&c,12)&&c.drone_cargo[0]==0&&!c.game.drones.installed);
 before=c;assert(!campaign_drone_swap(&c,0,0));unchanged();
 assert(campaign_purchase(&c,11)&&c.drone_cargo[0]==0);
 assert(campaign_purchase(&c,13));for(int i=0;i<CAM_CARGO_COUNT;i++)assert(c.cargo[i]==1);
 /* Refused travel retains deployment; any successful jump retires it, even
    a noncombat beacon, without refunding its previously spent part. */
 c.mode=CAM_MAP;c.current=0;c.game.drones.slots[0].enabled=1;c.game.drones.slots[0].deployed=1;
 c.game.drones.slots[0].charge_ticks=27;c.game.drones.slots[0].target_room=0;
 DroneState paid=c.game.drones.slots[0];int paid_parts=c.game.drone_parts;
 assert(!campaign_jump(&c,0)&&!memcmp(&paid,&c.game.drones.slots[0],sizeof paid)&&c.game.drone_parts==paid_parts);
 int fuel=c.fuel;c.fuel=0;assert(!campaign_jump(&c,2)&&!memcmp(&paid,&c.game.drones.slots[0],sizeof paid));c.fuel=fuel;
 assert(campaign_jump(&c,2)&&c.mode==CAM_EVENT&&!c.game.drones.slots[0].deployed&&!c.game.drones.slots[0].enabled);
 assert(!c.game.drones.slots[0].charge_ticks&&c.game.drones.slots[0].target_room==-1&&c.game.drone_parts==paid_parts&&c.game.drones.slots[0].catalog_id==0);
 /* Sector generation refreshes supply stock, preserving owned equipment. */
 c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;
 c.game.drones.slots[0]=paid;int prior_parts=c.game.drone_parts;
 assert(!campaign_choice(&c,1)&&!memcmp(&paid,&c.game.drones.slots[0],sizeof paid));
 assert(campaign_choice(&c,0)&&c.sector==1);
 assert(c.drone_cargo[0]==0&&c.game.drones.slots[0].catalog_id==0&&c.game.drone_parts==prior_parts&&!c.game.drones.slots[0].deployed&&!c.game.drones.slots[0].enabled&&!c.game.drones.slots[0].charge_ticks&&c.game.drones.slots[0].target_room==-1);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)assert(c.stock[i][11]==1&&c.stock[i][12]==1&&c.stock[i][13]==3);
 c.mode=CAM_STORE;c.current=5;c.scrap=100;c.game.drone_parts=999997;
 assert(campaign_purchase(&c,13)&&c.game.drone_parts==1000000);
 puts("drone campaign: package, supply bounds, separate inventory, exact sales, upgrades and atomic refusals passed");return 0;
}
