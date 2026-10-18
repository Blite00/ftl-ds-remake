#ifndef FTL_DS_SHOT_PATH_H
#define FTL_DS_SHOT_PATH_H
#include "game.h"
/* V68 projectile geometry, shared by render.c and host tests (no drawing).
 * Shots leave the firing ship's original weapon mount (slot i uses mount i,
 * artillery 4+i) and travel toward the other ship: the player faces right,
 * the enemy is on the right of the overview, so player shots move right and
 * enemy shots move left in every view. A hit ends in the centre of the
 * targeted room, a shield block on the target's shield ellipse, a miss beside
 * the ship outside every room, and an interception in open space. */
enum ShotView { SHOT_VIEW_OVERVIEW, SHOT_VIEW_PLAYER, SHOT_VIEW_ENEMY }; /* Top battle view; bottom schematic of the player / enemy ship. */
enum ShotTerminal { SHOT_FLIGHT, SHOT_ROOM, SHOT_SHIELD, SHOT_MISS, SHOT_SPACE, SHOT_EXIT };
/* x0,y0 launch point; gx,gy position when the shot resolves (age == duration);
 * x1,y1 terminal point for the resolved result; cx,cy aimed room centre. */
typedef struct { int x0,y0,gx,gy,x1,y1,cx,cy,terminal,mounted; } ShotPath;
/* 0 for inactive effects, beams and bombs (teleported) or a missing target room. */
int shot_path(const Game *g,int effect,int view,ShotPath *path);
/* Projectile position at age (flight, then a short run to its terminal point). */
void shot_point(const ShotPath *path,int age,int duration,int *x,int *y);
/* Vector of about len pixels along the current direction of travel. */
void shot_heading(const ShotPath *path,int age,int duration,int len,int *dx,int *dy);
/* Shared view geometry. side 0 is the player ship, 1 the enemy ship. Points are
 * room-grid pixels (35 per tile, the original layout units). */
void shot_geometry(const Ship *s,int enemy,int *ox,int *oy,int *t);
void shot_ship_point(const Game *g,int side,int view,int px,int py,int *x,int *y);
void shot_room_rect(const Game *g,int side,int view,const GameRoom *r,int *x,int *y,int *w,int *h);
void shot_hull_rect(const Game *g,int side,int view,int *x,int *y,int *w,int *h);
void shot_shield(const Game *g,int side,int view,int *cx,int *cy,int *rx,int *ry);
/* Screen point of a weapon mount (slot -1: no mount). Returns 1 for an original
 * mount, 0 for the fallback (forward edge of the weapons room). */
int shot_mount(const Game *g,int side,int slot,int view,int *x,int *y);
#endif
