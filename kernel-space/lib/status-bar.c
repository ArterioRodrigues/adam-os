#include "../pch.h"
void update_status_bar() {
    char buf[20];
    heap_block_header_t *curr = heap_head_ptr;
    uint32_t size = 0;
    uint32_t free = 0;
    while (curr) {
        size += curr->size + sizeof(heap_block_header_t);
        if (curr->is_free)
            free += curr->size + sizeof(heap_block_header_t);
        curr = curr->next;
    }
    terminal_print_at("used: ",   STATUS_USED_LABEL,   STATUS_BAR_ROW, WHITE);
    terminal_print_at(itos(buf, size - free), STATUS_USED_VALUE, STATUS_BAR_ROW, BRIGHT_GREEN);
    terminal_print_at("free: ",   STATUS_FREE_LABEL,   STATUS_BAR_ROW, WHITE);
    terminal_print_at(itos(buf, free), STATUS_FREE_VALUE, STATUS_BAR_ROW, BRIGHT_GREEN);
    terminal_print_at("pids: ",   STATUS_PIDS_LABEL,   STATUS_BAR_ROW, WHITE);
    terminal_print_at(itos(buf, process_queue_size), STATUS_PIDS_VALUE, STATUS_BAR_ROW, BRIGHT_GREEN);
    terminal_print_at("uptime: ", STATUS_UPTIME_LABEL, STATUS_BAR_ROW, WHITE);
    terminal_print_at(itos(buf, get_uptime_seconds()), STATUS_UPTIME_VALUE, STATUS_BAR_ROW, BRIGHT_GREEN);
}
