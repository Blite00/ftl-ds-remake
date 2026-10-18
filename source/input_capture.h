#ifndef FTL_DS_INPUT_CAPTURE_H
#define FTL_DS_INPUT_CAPTURE_H
#include <stdint.h>
#define INPUT_CAPTURE_CAPACITY 8
#define INPUT_CAPTURE_MAX_AGE 30u
/* Portable producer/consumer logic; caller serializes IRQ/main access. */
typedef struct { unsigned keys, sample; int x,y; } CapturedInput;
typedef struct {
 CapturedInput events[INPUT_CAPTURE_CAPACITY];
 unsigned held,sample,first,count,dropped;
 int blocked;
} InputCapture;
void input_capture_sample(volatile InputCapture*,unsigned held,int x,int y);
int input_capture_take(volatile InputCapture*,CapturedInput*);
void input_capture_block(volatile InputCapture*);
void input_capture_resume(volatile InputCapture*);
/* DS integration. View boundaries discard stale presses before a new modal. */
void ds_input_init(void);
uint32_t ds_input_vblank_count(void);
int ds_input_take(CapturedInput*);
void ds_input_view_begin(const void*context,unsigned size);
void ds_input_view_end(void);
#endif
