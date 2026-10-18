#ifndef FTL_DS_PROFILE_H
#define FTL_DS_PROFILE_H
/* V57 persistent profile (hangar unlocks), kept apart from the campaign save in
 * its own SD file with a .bak copy of the previous good version. A bad or missing
 * file falls back to the backup, then to the defaults (only the Kestrel A).
 * Bits are ship classes in the original order: 0 Kestrel, 1 Stealth, 2 Mantis,
 * 3 Engi, 4 Federation, 5 Slug, 6 Rock, 7 Zoltan, 8 Crystal. */
#define PROFILE_PATH "/ftl-ds-profile.dat"
#define PROFILE_CLASSES 9
typedef struct { unsigned unlocked_a,unlocked_b,achievements[2],runs,victories; } Profile;
void profile_defaults(Profile *profile);
/* 1 primary valid, 2 restored from the backup, 0 defaults. */
int profile_load(Profile *profile,const char *path);
int profile_save(const Profile *profile,const char *path);
/* Layout = class*2 + variant (0 Type A, 1 Type B). */
int profile_layout_unlocked(const Profile *profile,int layout);
/* Returns 1 when the layout was newly unlocked. */
int profile_unlock_layout(Profile *profile,int layout);
int profile_valid(const Profile *profile);
/* V59: add earned achievements (bit = id, 27 total); 2 of a class's 3 unlock its
 * Type B layout. Returns the bits that were new. */
#define PROFILE_ACHIEVEMENTS 27
unsigned profile_add_achievements(Profile *profile,unsigned earned);
int profile_class_achievements(const Profile *profile,int cls);
#endif
