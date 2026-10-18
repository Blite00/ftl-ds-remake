/* Run with a fresh disposable filename under build/; refuses existing files.
 * gcc -std=c99 -Wall -Wextra -Werror -Isource -Igenerated tests/logging_smoke.c
 *     source/diagnostics.c source/game.c -o build/logging_smoke.exe
 */
#include "diagnostics.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
    Game g;
    FILE *file;
    char text[16384],badpath[512];
    size_t count;
    long length;
    unsigned i;
    assert(argc==2);
    file=fopen(argv[1],"rb");
    if(file){fclose(file);fprintf(stderr,"Refusing existing smoke file: %s\n",argv[1]);return 2;}
    /* File belongs to this smoke check; verify append preserves older sessions. */
    file=fopen(argv[1],"wb");assert(file);
    assert(fputs("PRIOR SESSION\n",file)>=0);assert(fclose(file)==0);
    game_init(&g);diagnostics_init(argv[1]);diagnostics_poll(&g,0);
    diagnostics_input(2,1u,42,17);diagnostics_action(2,"fire laser");
    g.player.hull--;g.player.systems[SYS_SHIELDS].health--;
    diagnostics_poll(&g,2);diagnostics_poll(&g,600);
    g.outcome=GAME_VICTORY;diagnostics_poll(&g,602);
    file=fopen(argv[1],"rb");assert(file);
    count=fread(text,1,sizeof text-1,file);text[count]=0;assert(fclose(file)==0);
    assert(strstr(text,"PRIOR SESSION\n"));
    assert(strstr(text,"SESSION start"));assert(strstr(text,"INPUT keys="));
    assert(strstr(text,"ACTION fire laser"));assert(strstr(text,"COMBAT player_hull=29"));
    assert(strstr(text,"SYSTEM_CHANGE Shields"));
    assert(strstr(text,"SNAPSHOT periodic"));assert(strstr(text,"SNAPSHOT outcome"));
    assert(diagnostics_line_count()>0);
    /* Realtime mode never writes on periodic/outcome polling. Explicit Save,
       log-view or menu flush persists its bounded memory ring. */
    file=fopen(argv[1],"rb");assert(file);assert(fseek(file,0,SEEK_END)==0);length=ftell(file);fclose(file);
    diagnostics_defer_automatic_flush(1);diagnostics_action(700,"deferred marker");
    g.outcome=GAME_PLAYING;diagnostics_poll(&g,1200);g.outcome=GAME_DEFEAT;diagnostics_poll(&g,1202);
    file=fopen(argv[1],"rb");assert(file);assert(fseek(file,0,SEEK_END)==0);assert(ftell(file)==length);fclose(file);
    diagnostics_flush();file=fopen(argv[1],"rb");assert(file);count=fread(text,1,sizeof text-1,file);text[count]=0;fclose(file);
    assert(strstr(text,"deferred marker"));diagnostics_defer_automatic_flush(0);
    /* Exercise the hard cap without exceeding it or truncating prior content. */
    file=fopen(argv[1],"ab");assert(file);assert(fseek(file,0,SEEK_END)==0);
    length=ftell(file);assert(length>=0);
    while(length++<DIAGNOSTICS_MAX_BYTES)assert(fputc('x',file)!=EOF);
    assert(fclose(file)==0);
    diagnostics_action(604,"cap check");diagnostics_flush();
    assert(strstr(diagnostics_status(),"256 KiB"));
    file=fopen(argv[1],"rb");assert(file);assert(fseek(file,0,SEEK_END)==0);
    assert(ftell(file)==DIAGNOSTICS_MAX_BYTES);assert(fclose(file)==0);
    /* A regular file cannot be a parent directory: guaranteed unavailable sink. */
    snprintf(badpath,sizeof badpath,"%s/no-log-here",argv[1]);
    diagnostics_init(badpath);
    assert(strstr(diagnostics_status(),"Memory log"));
    for(i=0;i<100;i++)diagnostics_action(i,"ring test");
    assert(diagnostics_line_count()==DIAGNOSTICS_LINES);
    assert(strstr(diagnostics_line(0),"f=99 ACTION ring test"));
    assert(!*diagnostics_line(DIAGNOSTICS_LINES));
    assert(remove(argv[1])==0);
    puts("PASS: append, events, snapshots, outcome flush, file cap and memory fallback.");
    return 0;
}
