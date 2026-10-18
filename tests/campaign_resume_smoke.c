/* Real campaigns with cold initialization + disk resume after every map stop.
 * Reuse the public-API pilot/economy helpers from the campaign behavior check. */
#define main campaign_behavior_check_main
#include "campaign_v10_smoke.c"
#undef main
#include "save.h"
static const char *resume_paths[]={"tests/campaign-resume-check.sav","tests/campaign-resume-check.sav.bak","tests/campaign-resume-check.sav.tmp","tests/campaign-resume-check.sav.bad"};
static int file_exists(const char *path){FILE*f=fopen(path,"rb");if(!f)return 0;assert(!fclose(f));return 1;}
static void resume_checkpoint(Campaign *c) {
    Campaign loaded;
    assert(c->mode==CAM_MAP);assert(campaign_save(c,resume_paths[0]));
    campaign_init_legacy_seed(&loaded,123456u);assert(campaign_load(&loaded,resume_paths[0]));
    assert(loaded.seed==c->seed&&loaded.rng==c->rng&&loaded.sector==c->sector&&loaded.current==c->current&&loaded.selected==c->selected);
    assert(loaded.scrap==c->scrap&&loaded.fuel==c->fuel&&loaded.jumps==c->jumps&&loaded.sector_jumps==c->sector_jumps&&loaded.quest==c->quest);
    assert(!memcmp(loaded.links,c->links,sizeof c->links));assert(!memcmp(loaded.node_kind,c->node_kind,sizeof c->node_kind));assert(!memcmp(loaded.node_y,c->node_y,sizeof c->node_y));
    assert(!memcmp(loaded.visited,c->visited,sizeof c->visited));assert(!memcmp(loaded.stock,c->stock,sizeof c->stock));assert(!memcmp(loaded.cargo,c->cargo,sizeof c->cargo));
    assert(!memcmp(&loaded.game.player,&c->game.player,sizeof c->game.player));assert(!memcmp(loaded.game.doors,c->game.doors,sizeof c->game.doors));
    assert(loaded.game.crew_count==c->game.crew_count&&loaded.game.weapon_count==c->game.weapon_count);
    assert(!memcmp(loaded.game.crew,c->game.crew,sizeof(Crew)*(size_t)c->game.crew_count));
    assert(loaded.game.missiles==c->game.missiles&&loaded.game.rng==c->game.rng&&loaded.game.ticks==c->game.ticks);
    for(int i=0;i<c->game.weapon_count;i++){assert(loaded.game.weapons[i].catalog_id==c->game.weapons[i].catalog_id);assert(loaded.game.weapons[i].enabled==c->game.weapons[i].enabled);}
    *c=loaded;
}
int main(void) {
    static const unsigned seeds[]={1,42,8675309};static const int route[]={1,3,5,7,9,11};
    int checkpoints=0;
    for(int i=0;i<4;i++)assert(!file_exists(resume_paths[i]));
    for(unsigned seed=0;seed<sizeof seeds/sizeof seeds[0];seed++) {
        Campaign c;campaign_init_legacy_seed(&c,seeds[seed]);resume_checkpoint(&c);checkpoints++;
        for(int sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++) {
            assert(c.sector==sector);
            for(int node=0;node<6;node++) {
                assert(campaign_jump(&c,route[node]));settle(&c);
                if(c.mode==CAM_COMPLETE)break;
                resume_checkpoint(&c);checkpoints++;
            }
        }
        assert(c.mode==CAM_COMPLETE&&c.boss_phase==2&&c.jumps==36&&c.game.player.hull>0);
        printf("Resumed campaign seed %u: victory, hull %d, scrap %d\n",seeds[seed],c.game.player.hull,c.scrap);
        for(int i=0;i<4;i++)if(file_exists(resume_paths[i]))assert(!remove(resume_paths[i]));
    }
    assert(checkpoints==108&&battles>=27);
    puts("PASS: three actual six-sector campaigns, 108 disk save/cold-load checkpoints, preserved progression, and all final boss phases.");
    return 0;
}
