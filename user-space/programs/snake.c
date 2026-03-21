#include "../lib/lib.h"

/* ── Grid & window constants ─────────────────────────────────────────── */

#define GW 20 /* grid width  in cells */
#define GH 15 /* grid height in cells */
#define CELL 8
#define GRID_X 5
#define GRID_Y 5
#define SIDEBAR_X 170
#define WIN_W 250
#define WIN_H 135

/* ── Colors (VGA 256-color palette indices) ──────────────────────────── */

#define COL_BG 0x00
#define COL_BORDER 0x08
#define COL_SNAKE_HEAD 0x0A /* bright green */
#define COL_SNAKE_BODY 0x02 /* dark green */
#define COL_FOOD 0x0C       /* bright red */
#define COL_TEXT 0x0F        /* white */
#define COL_LABEL 0x07       /* grey */
#define COL_SIDEBAR 0x00

/* ── Directions ──────────────────────────────────────────────────────── */

#define DIR_UP 0
#define DIR_DOWN 1
#define DIR_LEFT 2
#define DIR_RIGHT 3

/* ── Max snake length (full grid) ────────────────────────────────────── */

#define MAX_LEN (GW * GH)

/* ── Game state ──────────────────────────────────────────────────────── */

static int snake_x[MAX_LEN];
static int snake_y[MAX_LEN];
static int snake_len;
static int head; /* ring-buffer index of head */
static int dir;
static int next_dir; /* buffered direction from input */

static int food_x, food_y;
static int score;
static int game_over;
static int move_interval; /* ticks between moves */

static uint32_t rng;
static uint32_t win_id;

/* Display buffer for diff rendering — stores color of each cell last drawn */
static uint8_t display[GH][GW];

/* Sidebar tracking */
static int prev_score;
static int sidebar_drawn;

/* Reusable syscall structs */
static create_rect_t R;
static create_text_t T;

/* ── RNG ─────────────────────────────────────────────────────────────── */

static int rand_range(int max) {
    rng = rng * 1103515245 + 12345;
    return ((rng >> 16) & 0x7FFF) % max;
}

/* ── Helper: get cell index in ring buffer ───────────────────────────── */

static int tail_index(void) {
    int t = head - snake_len + 1;
    if (t < 0)
        t += MAX_LEN;
    return t;
}

/* ── Check if a cell is occupied by the snake ────────────────────────── */

static int snake_at(int x, int y) {
    int idx = tail_index();
    for (int i = 0; i < snake_len; i++) {
        int ci = (idx + i) % MAX_LEN;
        if (snake_x[ci] == x && snake_y[ci] == y)
            return 1;
    }
    return 0;
}

/* ── Place food on an empty cell ─────────────────────────────────────── */

static void place_food(void) {
    int attempts = 0;
    do {
        food_x = rand_range(GW);
        food_y = rand_range(GH);
        attempts++;
    } while (snake_at(food_x, food_y) && attempts < 500);
}

/* ── Thin draw wrappers (matches Tetris pattern) ─────────────────────── */

static void draw_rect(int x, int y, int w, int h, uint8_t color) {
    R.x = x;
    R.y = y;
    R.width = w;
    R.height = h;
    R.color = color;
    sys_create_rect(&R);
}

static void draw_text(int x, int y, char *str, uint8_t color) {
    T.x = x;
    T.y = y;
    T.str = str;
    T.color = color;
    sys_create_text(&T);
}

static void draw_num(int x, int y, int n, uint8_t color) {
    char buf[12];
    itos(buf, n);
    draw_text(x, y, buf, color);
}

/* ── Build the "wanted" grid and diff against display ────────────────── */

static void render_grid(void) {
    uint8_t wanted[GH][GW];

    /* Start with empty background */
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            wanted[y][x] = COL_BG;

    /* Draw food */
    wanted[food_y][food_x] = COL_FOOD;

    /* Draw snake body and head */
    int idx = tail_index();
    for (int i = 0; i < snake_len; i++) {
        int ci = (idx + i) % MAX_LEN;
        int sx = snake_x[ci];
        int sy = snake_y[ci];
        if (sx >= 0 && sx < GW && sy >= 0 && sy < GH) {
            if (ci == head)
                wanted[sy][sx] = COL_SNAKE_HEAD;
            else
                wanted[sy][sx] = COL_SNAKE_BODY;
        }
    }

    /* Diff against last frame */
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            if (wanted[y][x] != display[y][x]) {
                display[y][x] = wanted[y][x];
                draw_rect(GRID_X + x * CELL, GRID_Y + y * CELL, CELL - 1, CELL - 1, wanted[y][x]);
            }
}

/* ── Sidebar (only redraws changed values) ───────────────────────────── */

static void render_sidebar(void) {
    if (!sidebar_drawn) {
        draw_text(SIDEBAR_X, 8, "SNAKE", COL_TEXT);
        draw_text(SIDEBAR_X, 24, "Score", COL_LABEL);
        draw_text(SIDEBAR_X, 60, "w  up", COL_LABEL);
        draw_text(SIDEBAR_X, 70, "s  down", COL_LABEL);
        draw_text(SIDEBAR_X, 80, "a  left", COL_LABEL);
        draw_text(SIDEBAR_X, 90, "d  right", COL_LABEL);
        draw_text(SIDEBAR_X, 110, "q  quit", COL_LABEL);
        sidebar_drawn = 1;
    }

    if (score != prev_score) {
        draw_rect(SIDEBAR_X, 36, 60, 10, COL_SIDEBAR);
        draw_num(SIDEBAR_X, 36, score, COL_TEXT);
        prev_score = score;
    }
}

/* ── Full render ─────────────────────────────────────────────────────── */

static void render(void) {
    render_grid();
    render_sidebar();
    if (game_over)
        draw_text(GRID_X + 30, GRID_Y + 50, "GAME OVER", COL_TEXT);
    sys_flush();
}

/* ── Initial full draw ───────────────────────────────────────────────── */

static void render_initial(void) {
    /* Clear entire window */
    draw_rect(0, 0, WIN_W, WIN_H, COL_SIDEBAR);

    /* Draw grid background */
    draw_rect(GRID_X, GRID_Y, GW * CELL, GH * CELL, COL_BG);

    /* Draw border around grid */
    draw_rect(GRID_X - 1, GRID_Y - 1, GW * CELL + 2, 1, COL_BORDER);               /* top */
    draw_rect(GRID_X - 1, GRID_Y + GH * CELL, GW * CELL + 2, 1, COL_BORDER);       /* bottom */
    draw_rect(GRID_X - 1, GRID_Y, 1, GH * CELL, COL_BORDER);                        /* left */
    draw_rect(GRID_X + GW * CELL, GRID_Y, 1, GH * CELL, COL_BORDER);               /* right */

    /* Reset display buffer */
    memset(display, COL_BG, GH * GW);
    sidebar_drawn = 0;
    prev_score = -1;

    render();
}

/* ── Input handling ──────────────────────────────────────────────────── */

static int handle_input(char c) {
    if (c == 'q' || c == 'Q')
        return 1;

    if (game_over)
        return 0;

    /* Prevent 180-degree turns (immediate self-collision) */
    if ((c == 'w' || c == 'W') && dir != DIR_DOWN)
        next_dir = DIR_UP;
    else if ((c == 's' || c == 'S') && dir != DIR_UP)
        next_dir = DIR_DOWN;
    else if ((c == 'a' || c == 'A') && dir != DIR_RIGHT)
        next_dir = DIR_LEFT;
    else if ((c == 'd' || c == 'D') && dir != DIR_LEFT)
        next_dir = DIR_RIGHT;

    return 0;
}

/* ── Move the snake one step ─────────────────────────────────────────── */

static void step(void) {
    if (game_over)
        return;

    dir = next_dir;

    /* Compute new head position */
    int hx = snake_x[head];
    int hy = snake_y[head];

    switch (dir) {
    case DIR_UP:
        hy--;
        break;
    case DIR_DOWN:
        hy++;
        break;
    case DIR_LEFT:
        hx--;
        break;
    case DIR_RIGHT:
        hx++;
        break;
    }

    /* Wall collision */
    if (hx < 0 || hx >= GW || hy < 0 || hy >= GH) {
        game_over = 1;
        return;
    }

    /* Self collision — check against body (not the tail tip, which will move) */
    int ate = (hx == food_x && hy == food_y);
    /* When not eating, the tail will vacate its cell, so skip it in collision check */
    int idx = tail_index();
    int check_len = ate ? snake_len : snake_len - 1;
    for (int i = 0; i < check_len; i++) {
        int ci = (idx + (ate ? i : i + 1)) % MAX_LEN;
        if (snake_x[ci] == hx && snake_y[ci] == hy) {
            game_over = 1;
            return;
        }
    }

    /* Advance head in ring buffer */
    head = (head + 1) % MAX_LEN;
    snake_x[head] = hx;
    snake_y[head] = hy;

    if (ate) {
        snake_len++;
        score += 10;
        /* Speed up every 50 points, minimum interval of 3 */
        move_interval = 10 - (score / 50);
        if (move_interval < 3)
            move_interval = 3;
        place_food();
    }
    /* If not eating, snake_len stays the same — tail naturally advances */
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main() {
    /* Create window */
    create_window_t win;
    win.x = 150;
    win.y = 60;
    win.width = WIN_W;
    win.height = WIN_H;
    strcpy(win.title, "SNAKE");
    win_id = sys_create_window(&win);
    R.window_id = win_id;
    T.window_id = win_id;

    /* Seed RNG */
    rng = sys_uptime();

    /* Init snake: 3 segments in the middle, moving right */
    snake_len = 3;
    head = 2;
    for (int i = 0; i < 3; i++) {
        snake_x[i] = GW / 2 - 2 + i;
        snake_y[i] = GH / 2;
    }

    dir = DIR_RIGHT;
    next_dir = DIR_RIGHT;
    score = 0;
    game_over = 0;
    move_interval = 10; /* 10 ticks × 10ms = 100ms per move */

    place_food();
    render_initial();

    int tick = 0;
    int dirty = 0;

    while (1) {
        /* Drain all pending events */
        event_t event;
        int ev;
        while ((ev = sys_get_event(win_id, &event)) == 1) {
            if (event.type == EVENT_KEYPRESS) {
                if (handle_input(event.c))
                    goto quit;
                dirty = 1;
            }
        }
        if (ev == -1)
            sys_exit();

        /* Move snake on timer */
        if (!game_over) {
            tick++;
            if (tick >= move_interval) {
                tick = 0;
                step();
                dirty = 1;
            }
        }

        /* Only render when something changed */
        if (dirty) {
            render();
            dirty = 0;
        }

        sys_sleep(1);
    }

quit:
    sys_exit();
    return 0;
}
