#include "../pch.h"

bool keyboard_pressed = false;
static bool shift_held = false;

char scancode_to_ascii[] = {0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
                            0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
                            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
                            'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

char scancode_to_ascii_shift[] = {0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
                                  0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,
                                  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z',
                                  'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   0,   0,   ' '};

#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LSHIFT_REL 0xAA
#define SCANCODE_RSHIFT_REL 0xB6
#define SCANCODE_END 0x4F

void keyboard_handler_main(registers_t *regs) {
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
        shift_held = true;
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    if (scancode == SCANCODE_LSHIFT_REL || scancode == SCANCODE_RSHIFT_REL) {
        shift_held = false;
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    if (shift_held && scancode == SCANCODE_ENTER) {
        stdin_write(' ');
        print("\n");
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }
    if (STANDARD_IN_BUFFER_SIZE <= stdin.count) {
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    if (scancode & SCANCODE_RELEASE_MASK) {
        keyboard_pressed = false;
    } else {
        keyboard_pressed = true;

        if (scancode == SCANCODE_ENTER)
            stdin_write('\n');

        else if (scancode == SCANCODE_BACKSPACE)
            stdin_write('\b');

        else if (scancode < sizeof(scancode_to_ascii)) {
            char c = shift_held ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
            if (c != 0)
                stdin_write(c);
        }
    }

    if (stdin.wait_queue.process != NULL && stdin.wait_queue.requested_len <= stdin.count ||
        scancode == SCANCODE_ENTER) {
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
