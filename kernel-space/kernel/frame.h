#ifndef FRAME_H
#define FRAME_H
#include "../../shared/types.h"
#include "../../shared/config.h"

extern unsigned int frames_bitmap[NUM_FRAMES / 32];

void dump_frame();
void init_frames();

uint32_t allocate_frame();
static inline void clear_frame(uint32_t frame);
static inline void set_frame(uint32_t frame);
int test_frame(uint32_t frame);

void mark_frames_used(uint32_t start, uint32_t end);
void free_frame(uint32_t frame);

#endif
