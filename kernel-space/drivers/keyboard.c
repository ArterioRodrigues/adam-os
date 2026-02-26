#include "../pch.h"

bool keyboard_pressed = false;

char scancode_to_ascii[] = {0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
                            0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
                            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
                            'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

void keyboard_handler_main(registers_t *regs) {

    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    char c = scancode_to_ascii[scancode];

    if (STANDARD_IN_BUFFER_SIZE <= stdin.count) {
        outb(PIC1_COMMAND, PIC_EOI);
        return;
    }

    else if (scancode & SCANCODE_RELEASE_MASK)
        keyboard_pressed = false;

    else {
        keyboard_pressed = true;
        if (scancode == SCANCODE_ENTER)
            stdin_write('\n');
        else if (scancode == SCANCODE_BACKSPACE)
            stdin_write('\b');
        else if (scancode < sizeof(scancode_to_ascii) && c != 0)
            stdin_write(c);
    }

    if (stdin.wait_queue.process != NULL && stdin.wait_queue.requested_len <= stdin.count) {
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
