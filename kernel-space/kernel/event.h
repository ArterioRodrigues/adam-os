#ifndef EVENT_H
#define EVENT_H
#include "types.h"

typedef enum { EVENT_KEYPRESS, EVENT_KEYRELEASE, EVENT_MOUSE_CLICK, EVENT_MOUSE_MOVE } event_type_t;

typedef struct {
    event_type_t type;

    uint8_t keyboard_scancode;
    int mouse_x;
    int mouse_y;
    uint8_t mouse_button;

    char c;
} event_t;

typedef struct {
    event_t events[64];
    uint32_t head_index;
    uint32_t tail_index;
    uint32_t size;
} event_queue_t;

void event_queue_push(event_queue_t *queue, event_t event);
event_t *event_queue_pop(event_queue_t *queue);
bool event_queue_is_empty(event_queue_t *queue);

#endif
