#include "../pch.h"

stdin_buffer_t stdin;

void stdin_write(char c) {
    if (c == '\b') {
        stdin.buffer[max(0, stdin.write_index - 1)] = ' ';
        stdin.write_index = max(0, stdin.write_index - 1);
        stdin.count = max(0, stdin.count - 1);
        print_char('\b');
        return;
    }

    stdin.buffer[stdin.write_index] = c;
    stdin.write_index = (stdin.write_index + 1) % STANDARD_IN_BUFFER_SIZE;
    stdin.count = stdin.count + 1;
    print_char_color(c, BRIGHT_GREEN);
}
void stdin_read(char *buf, int len) {
    for (int i = 0; i < len; i++) {
        buf[i] = stdin.buffer[stdin.read_index];
        stdin.count--;
        stdin.read_index = (stdin.read_index + 1) % STANDARD_IN_BUFFER_SIZE;
    }
}
void stdin_wake_process(registers_t *regs) {
    pcb_t *process = stdin.wait_queue.process;
    process->status = RUNNING;

    memcpy(&current_process->registers, regs, sizeof(registers_t));

    memcpy(regs, &process->registers, sizeof(registers_t));
    set_kernel_stack(process->kernel_stack);
    load_page_directory(process->page_directory);

    char *buf = (char *)stdin.wait_queue.process->registers.ecx;
    stdin_read(buf, stdin.wait_queue.requested_len);
    process->registers.eax = stdin.wait_queue.requested_len;

    stdin.wait_queue.process = NULL;
    stdin.wait_queue.requested_len = -1;

    process->status = READY;
    memcpy(regs, &current_process->registers, sizeof(registers_t));
    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
}
void stdin_block_process(registers_t *regs, int len) {
    current_process->status = WAITING;
    stdin.wait_queue.process = current_process;
    stdin.wait_queue.requested_len = len;
    update_scheduler(regs);
}
