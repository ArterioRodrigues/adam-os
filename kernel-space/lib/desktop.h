#ifndef DESKTOP_ICONS_H
#define DESKTOP_ICONS_H

#include "../../shared/types.h"

#define ICON_BITMAP_SIZE 16
#define ICON_SCALE 3
#define ICON_RENDER_SIZE (ICON_BITMAP_SIZE * ICON_SCALE)
#define ICON_SPACING_Y 64 
#define ICON_START_X 730
#define ICON_START_Y 50
#define ICON_LABEL_GAP 3
#define MAX_DESKTOP_ICONS 8

typedef struct {
    int x, y;
    char label[12];
    char program[12];
    uint8_t color;
    uint16_t bitmap[16];
} desktop_icon_t;

extern desktop_icon_t desktop_icons[];
extern int desktop_icon_count;

void init_desktop_icons(void);
void draw_desktop_icons(void);
bool handle_icon_click(void);
void process_desktop_launch(void);

#endif
