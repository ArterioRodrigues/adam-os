#ifndef EVENT_H
#define EVENT_H
#include "../../shared/abi-types.h"

typedef struct {
    event_t events[64];
    uint32_t head_index;
    uint32_t tail_index;
    uint32_t size;
} event_queue_t;

void event_queue_push(event_queue_t *queue, event_t event);
void event_queue_pop(event_queue_t *queue, event_t *event);
bool event_queue_is_empty(event_queue_t *queue);

#endif
