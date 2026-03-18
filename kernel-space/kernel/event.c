#include "../pch.h"

void event_queue_push(event_queue_t *queue, event_t event) {
    if (queue->size >= 64)
        return;

    queue->events[queue->head_index++] = event;
    queue->head_index = queue->head_index % 64;
    queue->size++;
}

event_t *event_queue_pop(event_queue_t *queue) {
    if (queue->size == 0)
        return NULL;

    event_t *event = &queue->events[queue->tail_index];
    queue->tail_index = (queue->tail_index + 1) % 64;
    queue->size--;
    return event;
}
bool event_queue_is_empty(event_queue_t *queue) { return queue->size == 0; }
