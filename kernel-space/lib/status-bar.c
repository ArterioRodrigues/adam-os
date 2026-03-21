#include "../pch.h"

#define STATUS_BAR_MID 12
#define STATUS_BAR_PADDING 12
#define DUMP_START 100
#define DUMP_SPACE 150
#define STATUS_DATA_OFFSET 55
#define STATUS_BAR_START 30

char *status[8];

int get_heap_status() {
    heap_block_header_t *curr = heap_head_ptr;
    uint32_t not_free_size = 0;

    while (curr) {

        not_free_size += curr->is_free ? 0 : curr->size;
        curr = curr->next;
    }

    return not_free_size;
}

int get_frame_status() {
    int index = 0;

    while (test_frame(index) == 1) {
        index++;
    }
    return index;
}

int get_processes_status() {
    pcb_t *p = scheduler_head_ptr;
    uint32_t result = 0;
    while (p) {
        result++;
        p = p->next;
    }

    return result;
}


void draw_status_bar() {
    vga_draw_rect(0, 0, screen_width, STATUS_BAR_HEIGHT, 0x11);
    vga_draw_string(STATUS_BAR_START, STATUS_BAR_MID, "ADAMOS", BRIGHT_BLUE);

    status[0] = "HEAP";
    status[1] = "FRMS";
    status[2] = "PIDS";
    status[3] = "WINS";

    char buf[10];

    int data = get_heap_status();
    int size = 0;
    itos(buf, data);
    size = strlen(buf);
    buf[size - 3] = 'K';
    buf[size - 2] = '\0';
    strcpy(status[4], buf);

    itos(buf, HEAP_SIZE - data);
    size = strlen(buf);
    buf[size - 3] = 'K';
    buf[size - 2] = '\0';
    strcat(status[4], "/");
    strcat(status[4], buf);

    data = get_frame_status();
    itos(buf, data);
    strcpy(status[5], buf);

    itos(buf, NUM_FRAMES - data);
    strcat(status[5], "/");
    strcat(status[5], buf);

    itos(buf, get_processes_status());
    strcpy(status[6], buf);
    itos(buf, window_count());
    strcpy(status[7], buf);

    for (int i = 0; i < 4; i++) {
        int x = DUMP_START + (DUMP_SPACE * i);
        x -= i == 3 ? 80 : 0;
        vga_draw_string(x, STATUS_BAR_MID, "|", LIGHT_GRAY);
        vga_draw_string(x + STATUS_BAR_PADDING, STATUS_BAR_MID, status[i], LIGHT_GRAY);
        vga_draw_string(x + STATUS_DATA_OFFSET, STATUS_BAR_MID, status[i + 4], BRIGHT_GREEN);
    }

    itos(buf, get_uptime_seconds());
    strcat(buf, "s");
    vga_draw_string(screen_pitch - STATUS_BAR_START - (strlen(buf) * font_col), STATUS_BAR_MID, buf, BRIGHT_GREEN);
}

void init_status_bar() {
    for (int i = 0; i < 8; i++)
        status[i] = kmalloc(20);
}
