#include "profile.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define PROFILE_VERSION 1u
#define PROFILE_WORDS 6
#define PROFILE_BYTES (8+4+4*PROFILE_WORDS+4)
static const unsigned char magic[8]={'F','T','L','D','S','P','R','F'};
static uint32_t checksum(const unsigned char*p,int n){uint32_t h=2166136261u;for(int i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static void put(unsigned char*p,uint32_t v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static uint32_t get(const unsigned char*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
void profile_defaults(Profile*p){memset(p,0,sizeof *p);p->unlocked_a=1u;}
int profile_valid(const Profile*p){unsigned mask=(1u<<PROFILE_CLASSES)-1u;
 return p&&(p->unlocked_a&1u)&&!(p->unlocked_a&~mask)&&!(p->unlocked_b&~mask)&&!(p->achievements[0]>>PROFILE_ACHIEVEMENTS)&&!p->achievements[1]&&p->victories<=p->runs&&p->runs<=1000000u;}
int profile_class_achievements(const Profile*p,int cls){int n=0;for(int k=0;k<3;k++){n+=!!(p->achievements[0]&(1u<<(cls*3+k)));}return n;}
unsigned profile_add_achievements(Profile*p,unsigned earned){unsigned fresh=earned&((1u<<PROFILE_ACHIEVEMENTS)-1u)&~p->achievements[0];
 p->achievements[0]|=fresh;
 for(int cls=0;cls<PROFILE_CLASSES;cls++)if(profile_class_achievements(p,cls)>=2)profile_unlock_layout(p,cls*2+1);
 return fresh;}
static void encode(const Profile*p,unsigned char*b){uint32_t w[PROFILE_WORDS]={p->unlocked_a,p->unlocked_b,p->achievements[0],p->achievements[1],p->runs,p->victories};
 memcpy(b,magic,8);put(b+8,PROFILE_VERSION);for(int i=0;i<PROFILE_WORDS;i++)put(b+12+4*i,w[i]);put(b+12+4*PROFILE_WORDS,checksum(b,12+4*PROFILE_WORDS));}
static int read_file(const char*path,Profile*out){unsigned char b[PROFILE_BYTES+1];Profile p;FILE*f=fopen(path,"rb");size_t n;
 if(!f)return 0;
 n=fread(b,1,sizeof b,f);fclose(f);
 if(n!=PROFILE_BYTES||memcmp(b,magic,8)||get(b+8)!=PROFILE_VERSION||get(b+12+4*PROFILE_WORDS)!=checksum(b,12+4*PROFILE_WORDS))return 0;
 p.unlocked_a=get(b+12);p.unlocked_b=get(b+16);p.achievements[0]=get(b+20);p.achievements[1]=get(b+24);p.runs=get(b+28);p.victories=get(b+32);
 if(!profile_valid(&p))return 0;
 *out=p;return 1;}
int profile_load(Profile*p,const char*path){char backup[300];
 profile_defaults(p);
 if(!path||strlen(path)>290)return 0;
 if(read_file(path,p))return 1;
 snprintf(backup,sizeof backup,"%s.bak",path);
 if(read_file(backup,p))return 2;
 profile_defaults(p);
 return 0;}
/* Write a temp file, keep the current good file as .bak, then move the temp in. */
int profile_save(const Profile*p,const char*path){char temp[300],backup[300];unsigned char b[PROFILE_BYTES];Profile check;FILE*f;int ok,primary;
 if(!profile_valid(p)||!path||!*path||strlen(path)>290)return 0;
 snprintf(temp,sizeof temp,"%s.tmp",path);snprintf(backup,sizeof backup,"%s.bak",path);
 encode(p,b);f=fopen(temp,"wb");
 if(!f)return 0;
 ok=fwrite(b,1,sizeof b,f)==sizeof b;
 if(fflush(f)!=0)ok=0;
 if(fclose(f)!=0)ok=0;
 if(!ok||!read_file(temp,&check)||memcmp(&check,p,sizeof check)){remove(temp);return 0;}
 primary=read_file(path,&check);
 if(primary){remove(backup);if(rename(path,backup)!=0){remove(temp);return 0;}}
 else remove(path); /* A damaged primary is replaced; the backup stays as it was. */
 if(rename(temp,path)!=0){if(primary)rename(backup,path);remove(temp);return 0;}
 return 1;}
int profile_layout_unlocked(const Profile*p,int layout){int cls=layout/2;
 if(!p||layout<0||cls>=PROFILE_CLASSES)return 0;
 return layout%2?!!(p->unlocked_b&(1u<<cls)):!!(p->unlocked_a&(1u<<cls));}
int profile_unlock_layout(Profile*p,int layout){int cls=layout/2;unsigned*mask;
 if(!p||layout<0||cls>=PROFILE_CLASSES||profile_layout_unlocked(p,layout))return 0;
 mask=layout%2?&p->unlocked_b:&p->unlocked_a;
 *mask|=1u<<cls;return 1;}
