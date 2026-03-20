#include "../lib/lib.h"

/* ── Board constants ─────────────────────────────────────────────────── */

#define BW         10
#define BH         20
#define CELL       10
#define BOARD_X    5
#define BOARD_Y    5
#define SIDEBAR_X  115
#define WIN_W      200
#define WIN_H      215

/* ── VGA colors for each piece (index 1-7) ───────────────────────────── */

#define COL_BG       0x08  /* dark gray — board background */
#define COL_GRID     0x00  /* black — grid lines */
#define COL_GHOST    0x07  /* light gray */
#define COL_SIDEBAR  0x00  /* black — sidebar bg */
#define COL_TEXT     0x0F  /* white */
#define COL_LABEL    0x07  /* light gray — dim labels */

static const uint8_t piece_colors[8] = {
    0x00,  /* 0: empty */
    0x0B,  /* 1: I  — bright cyan */
    0x0E,  /* 2: O  — yellow */
    0x0D,  /* 3: T  — bright magenta */
    0x0A,  /* 4: S  — bright green */
    0x0C,  /* 5: Z  — bright red */
    0x09,  /* 6: J  — bright blue */
    0x06,  /* 7: L  — brown (orange) */
};

/* ── Piece data (4x4 bitmaps, 4 rotations each) ─────────────────────── */

static const uint16_t pieces[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, /* I */
    {0x6600, 0x6600, 0x6600, 0x6600}, /* O */
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, /* T */
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, /* S */
    {0x0C60, 0x4C80, 0xC600, 0x2640}, /* Z */
    {0x0E80, 0xC440, 0x2E00, 0x44C0}, /* J */
    {0x0E20, 0x4460, 0x8E00, 0xC880}, /* L */
};

/* ── Game state ──────────────────────────────────────────────────────── */

static uint8_t board[BH][BW];
static int cp, cr, cx, cy;       /* current piece, rotation, position */
static int np;                    /* next piece */
static int score, total_lines, level;
static int game_over;
static int drop_interval;
static uint32_t rng;
static uint32_t win_id;

/* ── RNG / piece logic ───────────────────────────────────────────────── */

static int rand7(void) {
    rng = rng * 1103515245 + 12345;
    return ((rng >> 16) & 0x7FFF) % 7;
}

static int pcell(int p, int r, int gx, int gy) {
    return (pieces[p][r] >> (15 - gy * 4 - gx)) & 1;
}

static int collides(int p, int r, int px, int py) {
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 4; gx++)
            if (pcell(p, r, gx, gy)) {
                int bx = px + gx;
                int by = py + gy;
                if (bx < 0 || bx >= BW || by >= BH)
                    return 1;
                if (by >= 0 && board[by][bx])
                    return 1;
            }
    return 0;
}

static void lock_piece(void) {
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 4; gx++)
            if (pcell(cp, cr, gx, gy)) {
                int bx = cx + gx;
                int by = cy + gy;
                if (by >= 0 && by < BH && bx >= 0 && bx < BW)
                    board[by][bx] = cp + 1;
            }
}

static int clear_rows(void) {
    int cleared = 0;
    for (int y = BH - 1; y >= 0; y--) {
        int full = 1;
        for (int x = 0; x < BW; x++)
            if (!board[y][x]) { full = 0; break; }

        if (full) {
            cleared++;
            for (int yy = y; yy > 0; yy--)
                for (int xx = 0; xx < BW; xx++)
                    board[yy][xx] = board[yy - 1][xx];
            for (int xx = 0; xx < BW; xx++)
                board[0][xx] = 0;
            y++; /* re-check this row */
        }
    }
    return cleared;
}

static void spawn(void) {
    cp = np;
    np = rand7();
    cr = 0;
    cx = 3;
    cy = -1;
    if (collides(cp, cr, cx, cy))
        game_over = 1;
}

static void award_lines(int n) {
    static const int pts[] = {0, 100, 300, 500, 800};
    score += pts[n] * (level + 1);
    total_lines += n;
    level = total_lines / 10;
    drop_interval = 20 - level * 2;
    if (drop_interval < 2)
        drop_interval = 2;
}

/* ── Drawing helpers ─────────────────────────────────────────────────── */

static void draw_rect(int x, int y, int w, int h, uint8_t color) {
    create_rect_t r;
    r.window_id = win_id;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    r.color = color;
    sys_create_rect(&r);
}

static void draw_text(int x, int y, char *str, uint8_t color) {
    create_text_t t;
    t.window_id = win_id;
    t.x = x;
    t.y = y;
    t.str = str;
    t.color = color;
    sys_create_text(&t);
}

static void draw_num(int x, int y, int n, uint8_t color) {
    char buf[12];
    itos(buf, n);
    draw_text(x, y, buf, color);
}

static void draw_cell(int bx, int by, uint8_t color) {
    int px = BOARD_X + bx * CELL;
    int py = BOARD_Y + by * CELL;
    draw_rect(px + 1, py + 1, CELL - 1, CELL - 1, color);
}

/* ── Rendering ───────────────────────────────────────────────────────── */

static void render(void) {
    /* Clear entire window */
    draw_rect(0, 0, WIN_W, WIN_H, COL_SIDEBAR);

    /* Board background */
    draw_rect(BOARD_X, BOARD_Y, BW * CELL, BH * CELL, COL_BG);

    /* Grid lines */
    for (int x = 0; x <= BW; x++)
        draw_rect(BOARD_X + x * CELL, BOARD_Y, 1, BH * CELL, COL_GRID);
    for (int y = 0; y <= BH; y++)
        draw_rect(BOARD_X, BOARD_Y + y * CELL, BW * CELL, 1, COL_GRID);

    /* Locked pieces on board */
    for (int y = 0; y < BH; y++)
        for (int x = 0; x < BW; x++)
            if (board[y][x])
                draw_cell(x, y, piece_colors[board[y][x]]);

    /* Ghost piece */
    int ghost_y = cy;
    while (!collides(cp, cr, cx, ghost_y + 1))
        ghost_y++;
    if (ghost_y != cy) {
        for (int gy = 0; gy < 4; gy++)
            for (int gx = 0; gx < 4; gx++)
                if (pcell(cp, cr, gx, gy)) {
                    int bx = cx + gx;
                    int by = ghost_y + gy;
                    if (by >= 0 && by < BH && !board[by][bx])
                        draw_cell(bx, by, COL_GHOST);
                }
    }

    /* Current piece */
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 4; gx++)
            if (pcell(cp, cr, gx, gy)) {
                int bx = cx + gx;
                int by = cy + gy;
                if (by >= 0 && by < BH)
                    draw_cell(bx, by, piece_colors[cp + 1]);
            }

    /* ── Sidebar ─────────────────────────────────────────────────── */

    int sy = 8;

    draw_text(SIDEBAR_X, sy, "TETRIS", COL_TEXT);
    sy += 16;

    draw_text(SIDEBAR_X, sy, "Score", COL_LABEL);
    sy += 10;
    draw_num(SIDEBAR_X, sy, score, COL_TEXT);
    sy += 14;

    draw_text(SIDEBAR_X, sy, "Lines", COL_LABEL);
    sy += 10;
    draw_num(SIDEBAR_X, sy, total_lines, COL_TEXT);
    sy += 14;

    draw_text(SIDEBAR_X, sy, "Level", COL_LABEL);
    sy += 10;
    draw_num(SIDEBAR_X, sy, level + 1, COL_TEXT);
    sy += 18;

    /* Next piece preview */
    draw_text(SIDEBAR_X, sy, "Next", COL_LABEL);
    sy += 12;
    for (int py = 0; py < 4; py++)
        for (int px = 0; px < 4; px++)
            if (pcell(np, 0, px, py)) {
                int rx = SIDEBAR_X + px * 8;
                int ry = sy + py * 8;
                draw_rect(rx, ry, 7, 7, piece_colors[np + 1]);
            }
    sy += 40;

    /* Controls */
    draw_text(SIDEBAR_X, sy, "a/d move", COL_LABEL);
    sy += 10;
    draw_text(SIDEBAR_X, sy, "w   spin", COL_LABEL);
    sy += 10;
    draw_text(SIDEBAR_X, sy, "s   drop", COL_LABEL);
    sy += 10;
    draw_text(SIDEBAR_X, sy, "spc slam", COL_LABEL);
    sy += 10;
    draw_text(SIDEBAR_X, sy, "q   quit", COL_LABEL);

    /* Game over overlay */
    if (game_over) {
        draw_rect(BOARD_X + 5, BOARD_Y + 90, 90, 20, COL_SIDEBAR);
        draw_text(BOARD_X + 10, BOARD_Y + 95, "GAME OVER", COL_TEXT);
    }

    sys_flush();
}

/* ── Input handling ──────────────────────────────────────────────────── */

/* Returns 1 if user pressed quit */
static int handle_input(char c) {
    if (c == 'q' || c == 'Q')
        return 1;

    if (game_over)
        return 0;

    if (c == 'a' || c == 'A') {
        if (!collides(cp, cr, cx - 1, cy))
            cx--;
    } else if (c == 'd' || c == 'D') {
        if (!collides(cp, cr, cx + 1, cy))
            cx++;
    } else if (c == 'w' || c == 'W') {
        int nr = (cr + 1) % 4;
        if (!collides(cp, nr, cx, cy))
            cr = nr;
        else if (!collides(cp, nr, cx - 1, cy)) { cx--; cr = nr; }
        else if (!collides(cp, nr, cx + 1, cy)) { cx++; cr = nr; }
    } else if (c == 's' || c == 'S') {
        if (!collides(cp, cr, cx, cy + 1))
            cy++;
    } else if (c == ' ') {
        while (!collides(cp, cr, cx, cy + 1))
            cy++;
    }

    return 0;
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main() {
    /* Create window */
    create_window_t win;
    win.x = 200;
    win.y = 50;
    win.width = WIN_W;
    win.height = WIN_H;
    strcpy(win.title, "TETRIS");
    win_id = sys_create_window(&win);

    /* Init game state */
    memset(board, 0, BH * BW);
    rng = sys_uptime();
    score = 0;
    total_lines = 0;
    level = 0;
    game_over = 0;
    drop_interval = 20;
    np = rand7();
    spawn();

    int tick = 0;
    uint32_t last_time = sys_uptime();

    render();

    while (1) {
        /* Process all pending events */
        event_t event;
        while (sys_get_event(win_id, &event) == 1) {
            if (event.type == EVENT_KEYPRESS) {
                if (handle_input(event.c))
                    goto quit;
            }
        }

        /* Check if window was closed */
        if (sys_get_event(win_id, &event) == -1)
            sys_exit();

        /* Tick-based gravity */
        uint32_t now = sys_uptime();
        if (now != last_time) {
            last_time = now;

            if (!game_over) {
                tick++;
                if (tick >= drop_interval) {
                    tick = 0;
                    if (!collides(cp, cr, cx, cy + 1)) {
                        cy++;
                    } else {
                        lock_piece();
                        int cl = clear_rows();
                        if (cl > 0)
                            award_lines(cl);
                        spawn();
                    }
                }
            }

            render();
        }

        sys_sleep(1);
    }

quit:
    sys_exit();
    return 0;
}
