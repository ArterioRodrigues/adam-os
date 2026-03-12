#include "../pch.h"

#define STATUS_BAR_ROW 0
void update_status_bar() {
    char buf[20];

    heap_block_header_t *curr = heap_head_ptr;
    uint32_t size = 0;
    uint32_t free = 0;

    while (curr) {
        size += curr->size + sizeof(heap_block_header_t);
        if (curr->is_free) {
            free += curr->size + sizeof(heap_block_header_t);
        }
        curr = curr->next;
    }

    print_at("uptime: ", VGA_WIDTH - 13, STATUS_BAR_ROW, WHITE);
    print_at("00000s", VGA_WIDTH - 6, STATUS_BAR_ROW, BRIGHT_GREEN);

    itos(buf, get_uptime_seconds());
    print_at(buf, VGA_WIDTH - (strlen(buf) + 1), STATUS_BAR_ROW, BRIGHT_GREEN);

    print_at("pids: ", VGA_WIDTH - 21, STATUS_BAR_ROW, WHITE);
    print_at(itos(buf, process_queue_size), VGA_WIDTH - 16, STATUS_BAR_ROW, BRIGHT_GREEN);

    print_at("free: ", VGA_WIDTH - 45, STATUS_BAR_ROW, WHITE);
    print_at(itos(buf, free), VGA_WIDTH - 40, STATUS_BAR_ROW, BRIGHT_GREEN);

    print_at("used: ", VGA_WIDTH - 32, STATUS_BAR_ROW, WHITE);
    print_at(itos(buf, size - free), VGA_WIDTH - 27, STATUS_BAR_ROW, BRIGHT_GREEN);
}
