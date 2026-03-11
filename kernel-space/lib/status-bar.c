#include "../pch.h"

#define STATUS_BAR_ROW 0
void update_status_bar() {
    char buf[10];

    print_at("uptime: ", VGA_WIDTH - 13, STATUS_BAR_ROW, WHITE);
    print_at("00000s", VGA_WIDTH - 6, STATUS_BAR_ROW, BRIGHT_GREEN);

    itos(buf, get_uptime_seconds());
    print_at(buf, VGA_WIDTH - (strlen(buf) + 1), STATUS_BAR_ROW, BRIGHT_GREEN);

    print_at("pids: ", VGA_WIDTH - 20, STATUS_BAR_ROW, WHITE);
    print_at(itos(buf, process_queue_size), VGA_WIDTH - 15, STATUS_BAR_ROW, BRIGHT_GREEN);
}
