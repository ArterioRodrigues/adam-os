#include "../lib/lib.h"
#include "../lib/string.h"

void render_calculator(uint32_t id, char *display) {
    create_rect_t rect;
    rect.window_id = id;

    create_text_t text;
    text.window_id = id;

    rect.x = 0;
    rect.y = 12;
    rect.width = 160;
    rect.height = 38;
    rect.color = 0x0;
    sys_create_rect(&rect);

    rect.x = 0;
    rect.y = 50;
    rect.width = 162;
    rect.height = 150;
    rect.color = 0xF;
    sys_create_rect(&rect);

    text.x = 10;
    text.y = 30;
    text.str = display;
    text.color = 0xF;
    sys_create_text(&text);

    char buttons[4][4] = {{'7', '8', '9', '/'}, {'4', '5', '6', '*'}, {'1', '2', '3', '-'}, {'C', '0', '=', '+'}};
    char c[2];
    c[1] = '\0';

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            rect.x = (j * 40) + 2;
            rect.y = (i * 30) + 52;
            rect.width = 38;
            rect.height = 28;
            rect.color = 0x0;
            sys_create_rect(&rect);

            c[0] = buttons[i][j];
            text.x = (j * 40) + 18;
            text.y = (i * 30) + 62;
            text.str = c;
            text.color = 0xF;
            sys_create_text(&text);
        }
    }

    sys_flush();
}

void calculate(char *display) {
    int len = strlen(display);
    if (len == 0)
        return;

    if (display[len - 1] == '=')
        display[--len] = '\0';
    int result = 0;
    int current = 0;
    char op = '+';
    int i = 0;

    while (i <= len) {
        char c = display[i];

        if (i < len && c >= '0' && c <= '9') {
            current = current * 10 + (c - '0');
        } else {
            if (op == '+')
                result += current;
            else if (op == '-')
                result -= current;
            else if (op == '*')
                result *= current;
            else if (op == '/' && current != 0)
                result /= current;

            current = 0;
            op = c;
        }
        i++;
    }

    char buf[100];
    itos(buf, result);
    strcat(display, "=");
    strcat(display, buf);
}
void calculator() {
    create_window_t window;
    window.x = 50;
    window.y = 15;
    window.height = 172;
    window.width = 162;
    strcpy(window.title, "CALCULATOR");
    uint32_t id = sys_create_window(&window);

    char display[100];
    int index = 0;
    display[0] = '\0';

    char buttons[4][4] = {{'7', '8', '9', '/'}, {'4', '5', '6', '*'}, {'1', '2', '3', '-'}, {'C', '0', '=', '+'}};

    render_calculator(id, display);

    char c;
    int x;
    int y;
    while (1) {
        event_t event;
        int is_event = sys_get_event(id, &event);
        if (!is_event) {
            sys_sleep(1);
            continue;
        }
        if (event.type != EVENT_MOUSE_CLICK)
            continue;
        if (c == '=') {
            index = 0;
            display[index] = '\0';
        }

        x = event.mouse_x;
        y = event.mouse_y;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int col = j * 40 + 1;
                int row = i * 30 + 50;
                if (col <= x && x <= col + 40 && row <= y && y <= row + 30) {
                    display[index] = buttons[i][j];
                    display[index + 1] = '\0';
                    index++;
                    break;
                }
            }
        }

        c = display[index - 1];
        if (c == 'C') {
            index = 0;
            display[index] = '\0';
        } else if (c == '=') {
            calculate(display);
        }

        render_calculator(id, display);
    }
}
