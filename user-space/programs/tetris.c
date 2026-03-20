#include "../lib/lib.h"

#define BW 10
#define BH 20
#define COLS 80
#define WRITE_ROWS 24
#define FBSIZE (COLS * WRITE_ROWS)

char fb[FBSIZE];
uint8_t board[BH][BW];
uint16_t pieces[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x6600, 0x6600, 0x6600, 0x6600}, // O
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
    {0x0C60, 0x4C80, 0xC600, 0x2640}, // Z
    {0x0E80, 0xC440, 0x2E00, 0x44C0}, // J
    {0x0E20, 0x4460, 0x8E00, 0xC880}, // L
};

char pchar[] = " #@$%&*+";

int cp, cr, cx, cy;
int np;
int score, lines, level;
int game_over;
int drop_interval;
uint32_t rng;

// ── Framebuffer ──────────────────────────────────────────────────────────────
//
// NO newlines. VGA is 80x25 = 2000 cells laid out linearly.
//
// We only write rows 0-23 (1920 bytes). Row 24 is never touched,
// so vga_index stops at 1920, well below VGA_SIZE (2000).
// This prevents the scroll code from ever firing.
//
// Row 0:     empty (kernel status bar lives here)
// Row 1:     title + score
// Row 2:     top border + lines
// Row 3-22:  board (20 rows) + level/next/controls on right
// Row 23:    bottom border + game over
// Row 24:    empty (not written — prevents VGA scroll)

int rand7(void) {
    rng = rng * 1103515245 + 12345;
    return ((rng >> 16) & 0x7FFF) % 7;
}

int pcell(int p, int r, int gx, int gy) { return (pieces[p][r] >> (15 - gy * 4 - gx)) & 1; }

int collides(int p, int r, int px, int py) {
    int gy;
    int gx;
    for (gy = 0; gy < 4; gy++)
        for (gx = 0; gx < 4; gx++)
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

void lock_piece(void) {
    int gy;
    int gx;
    for (gy = 0; gy < 4; gy++)
        for (gx = 0; gx < 4; gx++)
            if (pcell(cp, cr, gx, gy)) {
                int bx = cx + gx;
                int by = cy + gy;
                if (by >= 0 && by < BH && bx >= 0 && bx < BW)
                    board[by][bx] = cp + 1;
            }
}

int clear_rows(void) {
    int cleared = 0;
    int y;
    for (y = BH - 1; y >= 0; y--) {
        int full = 1;
        int x;
        for (x = 0; x < BW; x++)
            if (!board[y][x]) {
                full = 0;
                break;
            }
        if (full) {
            cleared++;
            int yy;
            for (yy = y; yy > 0; yy--) {
                int xx;
                for (xx = 0; xx < BW; xx++)
                    board[yy][xx] = board[yy - 1][xx];
            }
            int xx;
            for (xx = 0; xx < BW; xx++)
                board[0][xx] = 0;
            y++;
        }
    }
    return cleared;
}

void spawn(void) {
    cp = np;
    np = rand7();
    cr = 0;
    cx = 3;
    cy = -1;
    if (collides(cp, cr, cx, cy))
        game_over = 1;
}

void award_lines(int n) {
    int pts[5];
    pts[0] = 0;
    pts[1] = 100;
    pts[2] = 300;
    pts[3] = 500;
    pts[4] = 800;
    score += pts[n] * (level + 1);
    lines += n;
    level = lines / 10;
    drop_interval = 20 - level * 2;
    if (drop_interval < 2)
        drop_interval = 2;
}

// ── Framebuffer helpers ──────────────────────────────────────────────────────

void fb_put(int row, int col, char c) {
    if (row >= 0 && row < WRITE_ROWS && col >= 0 && col < COLS)
        fb[row * COLS + col] = c;
}

void fb_str(int row, int col, char *s) {
    int i;
    for (i = 0; s[i] && col + i < COLS; i++)
        fb[row * COLS + col + i] = s[i];
}

void fb_num(int row, int col, int n) {
    char tmp[12];
    itos(tmp, n);
    fb_str(row, col, tmp);
}

// ── Rendering ────────────────────────────────────────────────────────────────

void render(void) {
    // Fill rows 0-23 with spaces
    memset(fb, ' ', FBSIZE);

    // Row 1: title + score
    fb_str(1, 4, "T E T R I S");
    fb_str(1, 30, "Score:");
    fb_num(1, 37, score);

    // Row 2: top border + lines
    fb_str(2, 3, "+----------+");
    fb_str(2, 30, "Lines:");
    fb_num(2, 37, lines);

    // Row 3: level (right side)
    fb_str(3, 30, "Level:");
    fb_num(3, 37, level + 1);

    // Rows 3-22: board (20 rows)
    {
        int y;
        for (y = 0; y < BH; y++) {
            int sr = y + 3;
            fb_put(sr, 3, '|');
            fb_put(sr, 14, '|');

            int x;
            for (x = 0; x < BW; x++) {
                char c = ' ';

                int px = x - cx;
                int py = y - cy;
                if (px >= 0 && px < 4 && py >= 0 && py < 4 && pcell(cp, cr, px, py))
                    c = pchar[cp + 1];

                if (c == ' ' && board[y][x])
                    c = pchar[board[y][x]];

                fb_put(sr, 4 + x, c);
            }
        }
    }

    // Ghost piece
    {
        int gy = cy;
        while (!collides(cp, cr, cx, gy + 1))
            gy++;
        if (gy != cy) {
            int py;
            for (py = 0; py < 4; py++) {
                int px;
                for (px = 0; px < 4; px++) {
                    if (pcell(cp, cr, px, py)) {
                        int bx = cx + px;
                        int by = gy + py;
                        if (by >= 0 && by < BH) {
                            int sr = by + 3;
                            int idx = sr * COLS + 4 + bx;
                            if (idx < FBSIZE && fb[idx] == ' ')
                                fb[idx] = ':';
                        }
                    }
                }
            }
        }
    }

    // Next piece preview
    fb_str(5, 30, "Next:");
    {
        int py;
        for (py = 0; py < 4; py++) {
            int px;
            for (px = 0; px < 4; px++) {
                char c = pcell(np, 0, px, py) ? pchar[np + 1] : ' ';
                fb_put(6 + py, 30 + px, c);
            }
        }
    }

    // Controls
    fb_str(11, 30, "Controls:");
    fb_str(12, 30, "a/d   move");
    fb_str(13, 30, "w     rotate");
    fb_str(14, 30, "s     soft drop");
    fb_str(15, 30, "space hard drop");
    fb_str(16, 30, "q     quit");

    // Row 23: bottom border + game over
    fb_str(23, 3, "+----------+");
    if (game_over)
        fb_str(23, 18, "GAME OVER -- press q");

    // Flush:
    //   clear + home (kernel only matches this exact 7-byte string)
    //   write 1920 bytes (rows 0-23 only, row 24 untouched)
    //   vga_index ends at 1920, never hits 2000, no scroll
    sys_write(1, "\033[2J\033[H", 7);
    sys_write(1, fb, FBSIZE);
}

// ── Input ────────────────────────────────────────────────────────────────────

void handle_input(char c) {
    if (game_over)
        return;

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
}

// ── Main ─────────────────────────────────────────────────────────────────────

void tetris(void) {

    memset(board, 0, BH * BW);

    rng = sys_uptime();
    score = 0;
    lines = 0;
    level = 0;
    game_over = 0;
    drop_interval = 20;

    np = rand7();
    spawn();

    int frame = 0;

    while (1) {
        while (sys_poll(0)) {
            char c;
            sys_read(0, &c, 1);
            if (c == 'q' || c == 'Q') {
                sys_write(1, "\033[2J\033[H", 7);
                return;
            }
            handle_input(c);
        }

        if (!game_over) {
            frame++;
            if (frame >= drop_interval) {
                frame = 0;
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

        if (game_over) {
            while (1) {
                char c;
                sys_read(0, &c, 1);
                if (c == 'q' || c == 'Q')
                    break;
            }
            sys_write(1, "\033[2J\033[H", 7);
            return;
        }

        sys_sleep(5);
    }
}

int main() {
    tetris();
    return 0;
}
