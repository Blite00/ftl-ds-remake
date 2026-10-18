#ifndef FTL_DS_SAVE_H
#define FTL_DS_SAVE_H
#include "campaign.h"
/* Map-only campaign format21; formats2-20 migrate on load.
 * Carried intruders persist; current-encounter home crew require promotion.
 * Primary then validated .bak on load.
 * New /ftl-ds-campaign.sav path preserves v9 /ftl-ds.sav.
 * Explicit bounded integer fields, static single-threaded staging.
 * Returns 1 on success; 0 leaves load
 * destination unchanged. Caller initializes the DS filesystem first. */
int campaign_save(const Campaign *campaign,const char *path);
int campaign_load(Campaign *campaign,const char *path);
#endif
