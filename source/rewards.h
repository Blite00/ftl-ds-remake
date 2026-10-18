#ifndef FTL_DS_REWARDS_H
#define FTL_DS_REWARDS_H
#include "game.h"
#define REWARD_MAX_CARGO 4
#define REWARD_CREW_ID_LIMIT 1000000u
enum RewardKind { REWARD_NONE, REWARD_WEAPON, REWARD_CREW };
/* Encounter-local offer. The campaign must block save/travel while kind!=NONE.
 * A recruit's identity is reserved once, even if the offer is later declined. */
typedef struct {int kind,weapon;Crew recruit;} PendingReward;
int rewards_offer_weapon(PendingReward *offer,int catalog);
int rewards_offer_crew(PendingReward *offer,Game *game,int species);
/* slot=-1 selects a free slot only; replace=1 requires an explicit slot.
 * Occupied cargo/live home crew require explicit replacement confirmation.
 * Passing NULL cargo and count0 is allowed for crew offers. */
int rewards_can_accept(const PendingReward *offer,const Game *game,const int *cargo,int cargo_count,int slot,int replace);
int rewards_accept(PendingReward *offer,Game *game,int *cargo,int cargo_count,int slot,int replace);
/* UI obtains confirmation before committing a decline. */
int rewards_decline(PendingReward *offer);
#endif
