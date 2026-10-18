#ifndef FTL_DS_BEAM_H
#define FTL_DS_BEAM_H
#include "game.h"
int beam_intersects_room(const GameRoom *room,int x0,int y0,int x1,int y1);
int beam_valid_line(const Ship *ship,int x0,int y0,int x1,int y1);
int beam_clamp_line(int x0,int y0,int *x1,int *y1,int length);
#endif
