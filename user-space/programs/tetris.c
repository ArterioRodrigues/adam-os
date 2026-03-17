// =============================================================================
// tetris.c — Tetris for AdamOS
// Place in: user-space/programs/tetris.c
//
// Requires 3 new syscalls (see kernel-changes.md):
//   sys_poll(fd)       — returns 1 if stdin has data, 0 otherwise
//   sys_sleep(ticks)   — sleeps for N timer ticks (100Hz, so 5 = 50ms)
//   sys_uptime()       — returns raw tick count (for RNG seed)
//
// Launch from shell:  fork tetris
// =============================================================================

#include "../lib/lib.h"
#include "../lib/string.h"

// ── Board ────────────────────────────────────────────────────────────────────

#define BW 10
#define BH 20

uint8_t board[BH][BW]; // 0 = empty, 1-7 = locked piece type

// ── Tetromino encoding ───────────────────────────────────────────────────────
// Each piece has 4 rotations stored as 16-bit bitmasks over a 4×4 grid.
// Bit 15 = cell (0,0), bit 14 = (1,0), ... bit 0 = (3,3).
//
//   Row 0:  bits 15-12
//   Row 1:  bits 11-8
//   Row 2:  bits  7-4
//   Row 3:  bits  3-0

uint16_t pieces[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x6600, 0x6600, 0x6600, 0x6600}, // O
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
    {0x0C60, 0x4C80, 0xC600, 0x2640}, // Z
    {0x0E80, 0xC440, 0x2E00, 0x44C0}, // J
    {0x0E20, 0x4460, 0x8E00, 0xC880}, // L
};

// Display characters for each piece type (1-indexed, 0 = empty)
char pchar[] = " #@$%&*+";

// ── Game state ───────────────────────────────────────────────────────────────

int cp, cr, cx, cy; // current piece, rotation, x, y
int np;             // next piece
int score, lines, level;
int game_over;
int drop_interval;
uint32_t rng;

// ── Frame buffer ─────────────────────────────────────────────────────────────
// 25 rows × 80 cols. We render the entire screen into this buffer, then
// clear the VGA and write it all at once to minimize flicker.

#define COLS 80
#define ROWS 24
#define FSIZE (ROWS * (COLS + 1)) // +1 for newline per row

char fb[FSIZE + COLS + 1]; // extra row at bottom with no newline

// ── Helpers ──────────────────────────────────────────────────────────────────

int rand7(void) {
    rng = rng * 1103515245 + 12345;
    return ((rng >> 16) & 0x7FFF) % 7;
}

// Is cell (cx_, cy_) in the 4×4 grid of piece p rotation r filled?
int pcell(int p, int r, int cx_, int cy_) {
    return (pieces[p][r] >> (15 - cy_ * 4 - cx_)) & 1;
}

// Collision test: would piece p at rotation r, position (px, py) overlap
// walls, floor, or locked cells?
int collides(int p, int r, int px, int py) {
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

// Lock the current piece into the board
void lock_piece(void) {
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 4; gx++)
            if (pcell(cp, cr, gx, gy)) {
                int bx = cx + gx;
                int by = cy + gy;
                if (by >= 0 && by < BH && bx >= 0 && bx < BW)
                    board[by][bx] = cp + 1;
            }
}

// Clear full rows, return count
int clear_rows(void) {
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
                for (int x = 0; x < BW; x++)
                    board[yy][x] = board[yy - 1][x];
            for (int x = 0; x < BW; x++)
                board[0][x] = 0;
            y++; // re-check this row
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

// ── Rendering ────────────────────────────────────────────────────────────────
// Layout (80×25):
//
// Row  0:  "  TETRIS"                 + score on right
// Row  1:  blank
// Row  2:  top border "+----------+"  + lines on right
// Row 3-22: board rows "|..........|" + info on right
// Row 23:  bottom border              + controls
// Row 24:  game-over message (no trailing newline to avoid VGA scroll)

// Write a string into the framebuffer at a given row and column
void fb_str(int row, int col, char *s) {
    int base = row * (COLS + 1);
    for (int i = 0; s[i] && col + i < COLS; i++)
        fb[base + col + i] = s[i];
}

// Write an integer into the framebuffer
void fb_num(int row, int col, int n) {
    char tmp[12];
    itos(tmp, n);
    fb_str(row, col, tmp);
}

void render(void) {
    // Fill with spaces, add newlines
    for (int r = 0; r < ROWS; r++) {
        int base = r * (COLS + 1);
        memset(fb + base, ' ', COLS);
        fb[base + COLS] = '\n';
    }
    // Last row (row 24): no newline — prevents VGA scroll
    int last = ROWS * (COLS + 1);
    memset(fb + last, ' ', COLS);

    // ── Title + Score ──
    fb_str(0, 4, "T E T R I S");
    fb_str(0, 30, "Score:");
    fb_num(0, 37, score);

    // ── Lines + Level ──
    fb_str(2, 30, "Lines:");
    fb_num(2, 37, lines);

    fb_str(3, 30, "Level:");
    fb_num(3, 37, level + 1);

    // ── Board border ──
    fb_str(2, 3, "+----------+");
    fb_str(23, 3, "+----------+");

    // ── Board cells ──
    for (int y = 0; y < BH; y++) {
        int sr = y + 3; // screen row
        fb[sr * (COLS + 1) + 3] = '|';
        fb[sr * (COLS + 1) + 14] = '|';

        for (int x = 0; x < BW; x++) {
            char c = '.';

            // Is the current falling piece here?
            int px = x - cx;
            int py = y - cy;
            if (px >= 0 && px < 4 && py >= 0 && py < 4) {
                if (pcell(cp, cr, px, py))
                    c = pchar[cp + 1];
            }

            // Locked piece on board (only if falling piece isn't here)
            if (c == '.' && board[y][x])
                c = pchar[board[y][x]];

            // Empty cell
            if (c == '.')
                c = ' ';

            fb[sr * (COLS + 1) + 4 + x] = c;
        }
    }

    // ── Ghost piece (drop preview) ──
    int gy = cy;
    while (!collides(cp, cr, cx, gy + 1))
        gy++;
    if (gy != cy) {
        for (int py = 0; py < 4; py++)
            for (int px = 0; px < 4; px++)
                if (pcell(cp, cr, px, py)) {
                    int bx = cx + px;
                    int by = gy + py;
                    if (by >= 0 && by < BH) {
                        int sr = by + 3;
                        int idx = sr * (COLS + 1) + 4 + bx;
                        if (fb[idx] == ' ')
                            fb[idx] = ':';
                    }
                }
    }

    // ── Next piece preview ──
    fb_str(5, 30, "Next:");
    for (int py = 0; py < 4; py++)
        for (int px = 0; px < 4; px++) {
            char c = pcell(np, 0, px, py) ? pchar[np + 1] : ' ';
            fb[(6 + py) * (COLS + 1) + 30 + px] = c;
        }

    // ── Controls ──
    fb_str(11, 30, "Controls:");
    fb_str(12, 30, "a/d   move");
    fb_str(13, 30, "w     rotate");
    fb_str(14, 30, "s     soft drop");
    fb_str(15, 30, "space hard drop");
    fb_str(16, 30, "q     quit");

    // ── Game over ──
    if (game_over)
        fb_str(24, 4, "GAME OVER  --  press q to quit");

    // ── Flush to screen ──
    sys_write(1, "\033[2J", 4);
    int total = ROWS * (COLS + 1) + COLS; // 24 rows with \n + 1 row without
    sys_write(1, fb, total);
}

// ── Input handling ───────────────────────────────────────────────────────────

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
        // basic wall kick: try shifting left or right by 1
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
        // hard drop
        while (!collides(cp, cr, cx, cy + 1))
            cy++;
    }
}

// ── Scoring ──────────────────────────────────────────────────────────────────

void award_lines(int n) {
    // Classic NES scoring
    int pts[5];
    pts[0] = 0;
    pts[1] = 100;
    pts[2] = 300;
    pts[3] = 500;
    pts[4] = 800;
    score += pts[n] * (level + 1);
    lines += n;
    level = lines / 10;

    // Speed up: 20 frames at level 0 → 2 frames at level 9+
    drop_interval = 20 - level * 2;
    if (drop_interval < 2)
        drop_interval = 2;
}

// ── Main ─────────────────────────────────────────────────────────────────────

void main(void) {
    // Init board
    memset(board, 0, BH * BW);

    // Seed RNG with uptime ticks
    rng = sys_uptime();

    // Init game state
    score = 0;
    lines = 0;
    level = 0;
    game_over = 0;
    drop_interval = 20;

    // First two pieces
    np = rand7();
    spawn();

    int frame = 0;

    // ── Game loop ────────────────────────────────────────────────────────
    while (1) {
        // Drain all pending input
        while (sys_poll(0)) {
            char c;
            sys_read(0, &c, 1);
            if (c == 'q' || c == 'Q') {
                // Clear screen and exit cleanly
                sys_write(1, "\033[2J", 4);
                return;
            }
            handle_input(c);
        }

        // Gravity: drop piece every drop_interval frames
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

        // Game over: wait for 'q'
        if (game_over) {
            while (1) {
                char c;
                sys_read(0, &c, 1);
                if (c == 'q' || c == 'Q')
                    break;
            }
            sys_write(1, "\033[2J", 4);
            return;
        }

        // ~20 fps (5 ticks at 100Hz = 50ms per frame)
        sys_sleep(5);
    }
}
