#include "../pch.h"

static char desktop_pending_launch[12] = {0};
desktop_icon_t desktop_icons[MAX_DESKTOP_ICONS];
int desktop_icon_count = 0;

static const uint16_t bmp_tetris[16] = {
    0x0000, 0x0000, 0x0FF0, 0x0FF0, 0x0FF0, 0x03C0, 0x03C0, 0x03C0,
    0x03C0, 0x03C0, 0x03C0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
static const uint16_t bmp_calc[16] = {
    0x0000, 0x7FFE, 0x6006, 0x6186, 0x6186, 0x6186, 0x67E6, 0x67E6,
    0x6186, 0x6186, 0x6186, 0x6006, 0x7FFE, 0x0000, 0x0000, 0x0000,
};

static void add_icon(char *label, char *program, uint8_t color, const uint16_t *bitmap) {
    if (desktop_icon_count >= MAX_DESKTOP_ICONS)
        return;

    desktop_icon_t *icon = &desktop_icons[desktop_icon_count];
    icon->x = ICON_START_X;
    icon->y = ICON_START_Y + desktop_icon_count * ICON_SPACING_Y;

    strcpy(icon->label, label);
    strcpy(icon->program, program);
    icon->color = color;
    memcpy(icon->bitmap, bitmap, 16 * sizeof(uint16_t));

    desktop_icon_count++;
}

void init_desktop_icons(void) {
    desktop_icon_count = 0;
    add_icon("Tetris", "TETRIS", BLACK, bmp_tetris);
    add_icon("Calc", "CALC", BLACK, bmp_calc);
}

static void draw_icon(desktop_icon_t *icon) {
    for (int row = 0; row < ICON_BITMAP_SIZE; row++) {
        uint16_t bits = icon->bitmap[row];
        for (int col = 0; col < ICON_BITMAP_SIZE; col++) {
            uint8_t color = bits & (0x8000 >> col) ? icon->color : WHITE;
            int px = icon->x + col * ICON_SCALE;
            int py = icon->y + row * ICON_SCALE;

            for (int sy = 0; sy < ICON_SCALE; sy++)
                for (int sx = 0; sx < ICON_SCALE; sx++)
                    vga_put_pixel(px + sx, py + sy, color);
        }
    }
}

void draw_desktop_icons(void) {
    for (int i = 0; i < desktop_icon_count; i++)
        draw_icon(&desktop_icons[i]);
}

bool handle_icon_click() {
    if (desktop_pending_launch[0])
        return false;

    for (int i = 0; i < desktop_icon_count; i++) {
        desktop_icon_t *icon = &desktop_icons[i];

        int hit_w = ICON_RENDER_SIZE + 16;
        int hit_h = ICON_RENDER_SIZE + ICON_LABEL_GAP + 10;

        if (mouse_x >= icon->x && mouse_x < icon->x + hit_w && mouse_y >= icon->y && mouse_y < icon->y + hit_h) {
            strcpy(desktop_pending_launch, icon->program);
            return true;
        }
    }
    return false;
}

static void desktop_spawn_program(char *filename) {
    uint8_t *stub = kmalloc(PAGE_SIZE);
    if (!stub)
        return;
    memset(stub, 0, PAGE_SIZE);

    uint32_t name_offset = 128;
    uint32_t arg_offset = 192;
    uint32_t name_addr = USER_FUNC_VADDR + name_offset;
    uint32_t arg_addr = USER_FUNC_VADDR + arg_offset;

    strcpy((char *)(stub + name_offset), filename);
    stub[arg_offset] = '\0';

    int i = 0;

    /* mov eax, 11 (SYSCALL_EXEC) */
    stub[i++] = 0xB8;
    stub[i++] = SYSCALL_EXEC;
    stub[i++] = 0x00;
    stub[i++] = 0x00;
    stub[i++] = 0x00;

    /* mov ecx, name_addr */
    stub[i++] = 0xB9;
    stub[i++] = (name_addr >> 0) & 0xFF;
    stub[i++] = (name_addr >> 8) & 0xFF;
    stub[i++] = (name_addr >> 16) & 0xFF;
    stub[i++] = (name_addr >> 24) & 0xFF;

    /* mov edx, arg_addr */
    stub[i++] = 0xBA;
    stub[i++] = (arg_addr >> 0) & 0xFF;
    stub[i++] = (arg_addr >> 8) & 0xFF;
    stub[i++] = (arg_addr >> 16) & 0xFF;
    stub[i++] = (arg_addr >> 24) & 0xFF;

    /* int 0x80 */
    stub[i++] = 0xCD;
    stub[i++] = 0x80;

    /* Fallback: mov eax, 1 (SYSCALL_EXIT) */
    stub[i++] = 0xB8;
    stub[i++] = SYSCALL_EXIT;
    stub[i++] = 0x00;
    stub[i++] = 0x00;
    stub[i++] = 0x00;

    /* int 0x80 */
    stub[i++] = 0xCD;
    stub[i++] = 0x80;

    page_directory_t *pd = create_kernel_page_directory((void *)stub);
    registers_t regs = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    pcb_t *pcb = create_process_control_block(pd, regs, 0, NULL);
    scheduler_enqueue(pcb);
    kfree(stub);
}

void process_desktop_launch(void) {
    if (!desktop_pending_launch[0])
        return;

    desktop_spawn_program(desktop_pending_launch);
    desktop_pending_launch[0] = '\0';
}
