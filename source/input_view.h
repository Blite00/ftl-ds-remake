#ifndef FTL_DS_INPUT_VIEW_H
#define FTL_DS_INPUT_VIEW_H
#include "render.h"
typedef struct { int fields[48]; } InputView;
void input_view_context(const Campaign*,const Ui*,const MainMenu*,InputView*);
#endif
