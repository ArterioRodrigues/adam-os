#include "../pch.h"

bool keyboard_pressed = false;
static bool shift_held = false;
static bool ctrl_held = false;

char scancode_to_ascii[] = {0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
                            0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
                            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
                            'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

char scancode_to_ascii_shift[] = {0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
                                  0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,
                                  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z',
                                  'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   0,   0,   ' '};

static bool handle_modifiers(unsigned char scancode) {
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
        shift_held = true;
        return true;
    }
    if (scancode == SCANCODE_LSHIFT_REL || scancode == SCANCODE_RSHIFT_REL) {
        shift_held = false;
        return true;
    }
    if (scancode == SCANCODE_LCTRL) {
        ctrl_held = true;
        return true;
    }
    if (scancode == SCANCODE_LCTRL_REL) {
        ctrl_held = false;
        return true;
    }
    return false;
}

static bool handle_chords(unsigned char scancode) {
    if (ctrl_held && scancode == SCANCODE_C) {
        sigint_foreground();
        return true;
    }
    if (shift_held && scancode == SCANCODE_ENTER) {
        if (terminal->window->is_focused)
            stdin_write(' ');
        else
            update_focused_window(EVENT_KEYPRESS, scancode, ' ', NULL, NULL, NULL);
        return true;
    }
    return false;
}

static void handle_input(unsigned char scancode) {
    bool is_terminal = terminal->window->is_focused;

    if (scancode == SCANCODE_ENTER)
        is_terminal ? stdin_write('\n') : update_focused_window(EVENT_KEYPRESS, scancode, '\n', NULL, NULL, NULL);

    else if (scancode == SCANCODE_BACKSPACE)
        is_terminal ? stdin_write('\b') : update_focused_window(EVENT_KEYPRESS, scancode, '\b', NULL, NULL, NULL);

    else if (scancode < sizeof(scancode_to_ascii)) {
        char c = shift_held ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
        if (c != 0)
            is_terminal ? stdin_write(c) : update_focused_window(EVENT_KEYPRESS, scancode, c, NULL, NULL, NULL);
    }
}

void keyboard_handler_main(registers_t *regs) {
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    if (handle_modifiers(scancode) || handle_chords(scancode) || (scancode & SCANCODE_RELEASE_MASK)) {
        keyboard_pressed = (scancode & SCANCODE_RELEASE_MASK) ? false : keyboard_pressed;
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    if (STANDARD_IN_BUFFER_SIZE <= stdin.count) {
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    keyboard_pressed = true;
    handle_input(scancode);

    if (stdin.wait_queue.process != NULL &&
        (stdin.wait_queue.requested_len <= stdin.count || scancode == SCANCODE_ENTER)) {
        stdin_wake_process(regs);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

void init_keyboard() {
    idt_set_gate(IRQ_KEYBOARD, (unsigned int)keyboard_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    stdin.count = 0;
    stdin.read_index = 0;
    stdin.write_index = 0;
}
