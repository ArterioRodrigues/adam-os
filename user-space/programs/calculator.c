#include "../lib/lib.h"
#include "../lib/string.h"

void calculator() {
    create_window_t window;
    window.x = 50;
    window.y = 15;
    window.height = 172;
    window.width = 160;
    strcpy(window.title, "CALCULATOR");
    uint32_t id = sys_create_window(&window);

    create_rect_t rect;
    rect.x = 0;
    rect.y = 50;
    rect.width = 160;
    rect.height = 160;
    rect.color = 0xF;
    rect.window_id = id;
    sys_create_rect(&rect);

    rect.width = 38;
    rect.height = 28;
    rect.color = 0x0;

    char c[2];
    char str[16] = "789/456*123-C0=+";
    create_text_t text;
    text.color = 0xF;
    text.window_id = id;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            rect.y = (i * 30) + 52;
            rect.x = (j * 40) + 1;
            sys_create_rect(&rect);

            c[0] = str[(i * 4) + j];
            c[1] = '\0';
            text.y = (i * 30) + 62;
            text.x = (j * 40) + 18;
            text.str = c;
            sys_create_text(&text);
        }
    }

    int index = 0;
    while (1) {
        event_t event;
        int is_event = sys_get_event(id, &event);

        if (!is_event) {
            sys_sleep(1);
            continue;
        }
        if (is_event && event.type != EVENT_MOUSE_CLICK)
            continue;

        int x = event.mouse_x;
        int y = event.mouse_y;
        bool clicked = false;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int col = j * 40 + 1;
                int row = i * 30 + 50;
                if (col <= x && x <= col + 40 && row <= y && y <= row + 30) {
                    c[0] = str[(i * 4) + j];
                    c[1] = '\0';
                    clicked = true;
                    break;
                }
            }
        }

        if (!clicked)
            continue;

        text.x = 20 + index;
        text.y = 30;
        text.str = c;
        sys_create_text(&text);

        index += 6;
    }
}
