#include "../lib/lib.h"

/* ── Board constants ─────────────────────────────────────────────────── */

#define BW 10
#define BH 20
#define CELL 10
#define BOARD_X 5
#define BOARD_Y 5
#define SIDEBAR_X 115
#define WIN_W 200
#define WIN_H 215

/* ── Colors ──────────────────────────────────────────────────────────── */

#define COL_BG 0x08
#define COL_GHOST 0x07
#define COL_SIDEBAR 0x00
#define COL_TEXT 0x0F
#define COL_LABEL 0x07

static const uint8_t piece_colors[8] = {
    COL_BG, 0x0B, 0x0E, 0x0D, 0x0A, 0x0C, 0x09, 0x06,
};

/* ── Piece data ──────────────────────────────────────────────────────── */

static const uint16_t pieces[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, {0x6600, 0x6600, 0x6600, 0x6600}, {0x0E40, 0x4C40, 0x4E00, 0x4640},
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, {0x0C60, 0x4C80, 0xC600, 0x2640}, {0x0E80, 0xC440, 0x2E00, 0x44C0},
    {0x0E20, 0x4460, 0x8E00, 0xC880},
};

/* ── Game state ──────────────────────────────────────────────────────── */

static uint8_t board[BH][BW];
static int cp, cr, cx, cy;
static int np;
static int score, total_lines, level;
static int game_over;
static int drop_interval;
static uint32_t rng;
static uint32_t win_id;

/*
 * Double-buffer: display[][] holds the color each cell was LAST drawn as.
 * Only cells where wanted != display get a syscall.
 * Typical frame: 4-8 rect calls instead of 200+.
 */
static uint8_t display[BH][BW];

/* Sidebar change tracking */
static int prev_score, prev_lines, prev_level, prev_np;
static int sidebar_drawn;

/* Reusable syscall structs — set window_id once, avoids re-init each call */
static create_rect_t R;
static create_text_t T;

/* ── RNG / piece logic ───────────────────────────────────────────────── */

static int rand7(void) {
    rng = rng * 1103515245 + 12345;
    return ((rng >> 16) & 0x7FFF) % 7;
}

static int pcell(int p, int r, int gx, int gy) { return (pieces[p][r] >> (15 - gy * 4 - gx)) & 1; }

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
            if (!board[y][x]) {
                full = 0;
                break;
            }
        if (full) {
            cleared++;
            for (int yy = y; yy > 0; yy--)
                for (int xx = 0; xx < BW; xx++)
                    board[yy][xx] = board[yy - 1][xx];
            for (int xx = 0; xx < BW; xx++)
                board[0][xx] = 0;
            y++;
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

/* ── Thin draw wrappers ──────────────────────────────────────────────── */

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

/* ── Diff-based board renderer ───────────────────────────────────────── */

static void render_board(void) {
    /*
     * Build wanted[][] from board + ghost + active piece,
     * then only issue syscalls for cells that differ from display[][].
     */
    uint8_t wanted[BH][BW];

    /* Locked board */
    for (int y = 0; y < BH; y++)
        for (int x = 0; x < BW; x++)
            wanted[y][x] = board[y][x] ? piece_colors[board[y][x]] : COL_BG;

    /* Ghost piece */
    int ghost_y = cy;
    while (!collides(cp, cr, cx, ghost_y + 1))
        ghost_y++;

    if (ghost_y != cy) {
        for (int gy = 0; gy < 4; gy++)
            for (int gx = 0; gx < 4; gx++)
                if (pcell(cp, cr, gx, gy)) {
                    int bx = cx + gx, by = ghost_y + gy;
                    if (by >= 0 && by < BH && bx >= 0 && bx < BW && wanted[by][bx] == COL_BG)
                        wanted[by][bx] = COL_GHOST;
                }
    }

    /* Active piece */
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 4; gx++)
            if (pcell(cp, cr, gx, gy)) {
                int bx = cx + gx, by = cy + gy;
                if (by >= 0 && by < BH && bx >= 0 && bx < BW)
                    wanted[by][bx] = piece_colors[cp + 1];
            }

    /* Diff against last frame — only changed cells get a syscall */
    for (int y = 0; y < BH; y++)
        for (int x = 0; x < BW; x++)
            if (wanted[y][x] != display[y][x]) {
                display[y][x] = wanted[y][x];
                draw_rect(BOARD_X + x * CELL, BOARD_Y + y * CELL, CELL, CELL, wanted[y][x]);
            }
}

/* ── Sidebar (only redraws changed values) ───────────────────────────── */

static void render_sidebar(void) {
    if (!sidebar_drawn) {
        draw_text(SIDEBAR_X, 8, "TETRIS", COL_TEXT);
        draw_text(SIDEBAR_X, 24, "Score", COL_LABEL);
        draw_text(SIDEBAR_X, 48, "Lines", COL_LABEL);
        draw_text(SIDEBAR_X, 72, "Level", COL_LABEL);
        draw_text(SIDEBAR_X, 100, "Next", COL_LABEL);
        draw_text(SIDEBAR_X, 152, "a/d move", COL_LABEL);
        draw_text(SIDEBAR_X, 162, "w   spin", COL_LABEL);
        draw_text(SIDEBAR_X, 172, "s   drop", COL_LABEL);
        draw_text(SIDEBAR_X, 182, "spc slam", COL_LABEL);
        draw_text(SIDEBAR_X, 192, "q   quit", COL_LABEL);
        sidebar_drawn = 1;
    }

    if (score != prev_score) {
        draw_rect(SIDEBAR_X, 34, 80, 10, COL_SIDEBAR);
        draw_num(SIDEBAR_X, 34, score, COL_TEXT);
        prev_score = score;
    }
    if (total_lines != prev_lines) {
        draw_rect(SIDEBAR_X, 58, 80, 10, COL_SIDEBAR);
        draw_num(SIDEBAR_X, 58, total_lines, COL_TEXT);
        prev_lines = total_lines;
    }
    if (level != prev_level) {
        draw_rect(SIDEBAR_X, 82, 80, 10, COL_SIDEBAR);
        draw_num(SIDEBAR_X, 82, level + 1, COL_TEXT);
        prev_level = level;
    }
    if (np != prev_np) {
        draw_rect(SIDEBAR_X, 112, 35, 35, COL_SIDEBAR);
        for (int py = 0; py < 4; py++)
            for (int px = 0; px < 4; px++)
                if (pcell(np, 0, px, py))
                    draw_rect(SIDEBAR_X + px * 8, 112 + py * 8, 7, 7, piece_colors[np + 1]);
        prev_np = np;
    }
}

static void render(void) {
    render_board();
    render_sidebar();
    if (game_over)
        draw_text(BOARD_X + 10, BOARD_Y + 95, "GAME OVER", COL_TEXT);
    sys_flush();
}

/* ── One-time full draw ──────────────────────────────────────────────── */

static void render_initial(void) {
    draw_rect(0, 0, WIN_W, WIN_H, COL_SIDEBAR);
    draw_rect(BOARD_X, BOARD_Y, BW * CELL, BH * CELL, COL_BG);

    memset(display, COL_BG, BH * BW);
    sidebar_drawn = 0;
    prev_score = -1;
    prev_lines = -1;
    prev_level = -1;
    prev_np = -1;

    render();
}

/* ── Input ───────────────────────────────────────────────────────────── */

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
        else if (!collides(cp, nr, cx - 1, cy)) {
            cx--;
            cr = nr;
        } else if (!collides(cp, nr, cx + 1, cy)) {
            cx++;
            cr = nr;
        }
    } else if (c == 's' || c == 'S') {
        if (!collides(cp, cr, cx, cy + 1))
            cy++;
    } else if (c == ' ') {
        while (!collides(cp, cr, cx, cy + 1))
            cy++;
    }
    return 0;
}

/* ── Main loop ───────────────────────────────────────────────────────── */

int main() {
    create_window_t win;
    win.x = 200;
    win.y = 50;
    win.width = WIN_W;
    win.height = WIN_H;
    strcpy(win.title, "TETRIS");
    win_id = sys_create_window(&win);
    R.window_id = win_id;
    T.window_id = win_id;

    memset(board, 0, BH * BW);
    rng = sys_uptime();
    score = 0;
    total_lines = 0;
    level = 0;
    game_over = 0;
    drop_interval = 20;
    np = rand7();
    spawn();

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

        /* Gravity */
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
                dirty = 1;
            }
        }

        /* Only render when something changed */
        if (dirty) {
            render();
            dirty = 0;
        }

        sys_sleep(5);
    }

quit:
    sys_exit();
    return 0;
}
