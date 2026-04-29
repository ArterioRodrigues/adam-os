#include "../lib/lib.h"

#define WINDOW_W 400
#define WINDOW_H 300
#define FOCAL_LENGTH 300

#define BACKGROUND_COLOR 0x66
#define POINT_COLOR 0x00

static create_rect_t RECT;
static create_window_t WINDOW;

static obj_face_t FACES[MAX_FACES];
static obj_vertex_t VERTICES[MAX_VERTICES];

static int VERTEX_COUNT = 0;
static int FACE_COUNT = 0;
static int WINDOW_ID = 0;

static float YAW = 0.0f;
static float PITCH = 0.0f;
static float CAMERA_DISTANCE = 3;

static void draw_rect(int x, int y, int w, int h, uint8_t color) {
    RECT.x = x;
    RECT.y = y;
    RECT.width = w;
    RECT.height = h;
    RECT.color = color;
    sys_create_rect(&RECT);
}

static void draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = x1 - x0;
    if (dx < 0)
        dx = -dx;
    int dy = y1 - y0;
    if (dy < 0)
        dy = -dy;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_rect(x0, y0, 1, 1, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void load_obj(char *file_name) {
    char buf[1024];
    int fd = sys_open(file_name);
    if (fd == -1) {
        print("FILE NOT FOUND!");
        sys_exit();
    }
    int size = sys_read(fd, buf, 1000);
    while (size > 0) {
        parse_obj(buf, VERTICES, &VERTEX_COUNT, FACES, &FACE_COUNT);
        size = sys_read(fd, buf, 1000);
    }
}
static void init() {
    WINDOW.height = WINDOW_H;
    WINDOW.width = WINDOW_W;
    WINDOW.x = 100;
    WINDOW.y = 100;
    strcpy(WINDOW.title, "OBJ_VIEWER");

    WINDOW_ID = sys_create_window(&WINDOW);
    RECT.window_id = WINDOW_ID;
}
static void render_inital() {
    draw_rect(0, 0, WINDOW_W, WINDOW_H, BACKGROUND_COLOR);
    mat4_t rx = mat4_rotate_x(PITCH);
    mat4_t ry = mat4_rotate_y(YAW);
    mat4_t rotation = mat4_mul(ry, rx);

    for (int i = 0; i < VERTEX_COUNT; i++) {
        vec3_t v = {VERTICES[i].x, VERTICES[i].y, VERTICES[i].z};
        v = mat4_mul_vec3(rotation, v);
        v.z += CAMERA_DISTANCE;
        screen_point_t point = project(v, FOCAL_LENGTH, WINDOW_W, WINDOW_H);
        if (!point.valid)
            continue;

        draw_rect(point.x - 2, point.y - 2, 4, 4, POINT_COLOR);
    }
    sys_flush();
}
static void render_wireframe() {
    draw_rect(0, 0, WINDOW_W, WINDOW_H, BACKGROUND_COLOR);
    mat4_t rx = mat4_rotate_x(PITCH);
    mat4_t ry = mat4_rotate_y(YAW);
    mat4_t rotation = mat4_mul(ry, rx);

    for (int i = 0; i < FACE_COUNT; i++) {
        obj_face_t f = FACES[i];

        screen_point_t sp[3];
        bool ok = true;
        int idx[3] = {f.a, f.b, f.c};

        for (int j = 0; j < 3; j++) {
            vec3_t v = {VERTICES[idx[j]].x, VERTICES[idx[j]].y, VERTICES[idx[j]].z};
            v = mat4_mul_vec3(rotation, v);
            v.z += CAMERA_DISTANCE;
            sp[j] = project(v, FOCAL_LENGTH, WINDOW_W, WINDOW_H);
            if (!sp[j].valid) {
                ok = false;
                break;
            }
        }
        if (!ok)
            continue;

        draw_line(sp[0].x, sp[0].y, sp[1].x, sp[1].y, POINT_COLOR);
        draw_line(sp[1].x, sp[1].y, sp[2].x, sp[2].y, POINT_COLOR);
        draw_line(sp[2].x, sp[2].y, sp[0].x, sp[0].y, POINT_COLOR);
    }

    sys_flush();
}

int main(char *arg) {
    char *file_name = arg;
    bool toggle = true;
    init();

    load_obj(file_name);
    while (true) {
        event_t event;
        int ev;
        ev = sys_get_event(WINDOW_ID, &event);

        if (ev == 1 && event.type == EVENT_KEYPRESS) {
            if (event.c == 't')
                toggle = !toggle;
            else if (event.c == 'k')
                PITCH += 0.05f;
            else if (event.c == 'j')
                PITCH -= 0.05f;
            else if (event.c == 'h')
                YAW += 0.05f;
            else if (event.c == 'l')
                YAW -= 0.05f;
        }

        if (ev == -1)
            sys_exit();

        toggle ? render_wireframe() : render_inital();
        sys_sleep(5);
    }
    return 0;
}
