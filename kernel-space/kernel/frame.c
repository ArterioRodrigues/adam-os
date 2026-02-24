#include "../pch.h"

unsigned int frames_bitmap[NUM_FRAMES / 32];
static inline void clear_frame(uint32_t frame) { frames_bitmap[frame / 32] &= ~(1 << (frame % 32)); }
static inline void set_frame(uint32_t frame) { frames_bitmap[frame / 32] |= (1 << (frame % 32)); }
static inline int test_frame(uint32_t frame) {
    if (frames_bitmap[frame / 32] & (1 << (frame % 32)))
        return 1;
    return 0;
}

void init_frames() {
    for (uint32_t i = 0; i < NUM_FRAMES; i++)
        clear_frame(i);

}

uint32_t allocate_frame() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (!test_frame(i)) {
            set_frame(i);
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void mark_frames_used(uint32_t start, uint32_t end) {
    start /= PAGE_SIZE;
    end /= PAGE_SIZE;

    for (int i = start; i < end; i++) {
        set_frame(i);
    }
}

void dump_frame() {
    char *result;

    for (int i = 0; i < NUM_FRAMES; i++) {
        print(itos(result, test_frame(i)));
    }

    print("\n");
}
