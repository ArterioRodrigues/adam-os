volatile unsigned short *vgaBuffer = (unsigned short *)0xB8000;
int vgaIndex = 0;

#define COLOR 0x0F
#define IDT_ENTRIES 256

struct idtEntry {
  unsigned short baseLow;
  unsigned short selector;
  unsigned char zero;
  unsigned char flags;
  unsigned short baseHigh;
} __attribute__((packed));

struct idtPtr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

struct idtEntry idt[IDT_ENTRIES];
struct idtPtr idtp;

char scancodeToAscii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
    0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

unsigned char inb(unsigned short port) {
  unsigned char result;
  __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

void outb(unsigned short port, unsigned char data) {
  __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

void printChar(char c) {
  if (c == '\n') {
    vgaIndex = (vgaIndex / 80 + 1) * 80;
  } else if (c == '\b') { // Backspace
    if (vgaIndex > 0) {
      vgaIndex--;
      vgaBuffer[vgaIndex] = (COLOR << 8) | ' ';
    }
  } else {
    vgaBuffer[vgaIndex] = (COLOR << 8) | c;
    vgaIndex++;
  }

  // Scroll if needed
  if (vgaIndex >= 80 * 25) {
    vgaIndex = 80 * 24;
  }
}

void print(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    printChar(str[i]);
  }
}

void clearScreen() {
  for (int i = 0; i < 80 * 25; i++) {
    vgaBuffer[i] = (COLOR << 8) | ' ';
  }
  vgaIndex = 0;
}

void idtSetGate(unsigned char num, unsigned int base, unsigned short selector,
                unsigned char flags) {
  idt[num].baseLow = base & 0xFFFF;
  idt[num].baseHigh = (base >> 16) & 0xFFFF;
  idt[num].selector = selector;
  idt[num].zero = 0;
  idt[num].flags = flags;
}

void keyboardHandlerMain() {
  unsigned char scancode = inb(0x60);
  if (scancode & 0x80) {
  } else {
    if (scancode == 0x1C)
      printChar('\n');
    else if (scancode == 0x0E)
      printChar('\n');
    else if (scancode < sizeof(scancodeToAscii)) {
      char c = scancodeToAscii[scancode];
      if (c != 0)
        printChar(c);
    }
  }

  outb(0x20, 0x20);
}

void remapPic() {
  unsigned char mask1 = inb(0x21);
  unsigned char mask2 = inb(0xA1);

  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  outb(0x21, 0x20);
  outb(0xA1, 0x28);

  outb(0x21, 0x04);

  outb(0xA1, 0x02);

  outb(0x21, 0x01);
  outb(0xA1, 0x11);

  outb(0x21, mask1);
  outb(0xA1, mask2);
}

extern void keyboardHandler();
extern void idtLoad();

void kernelMain() {
  clearScreen();
  print("Welcome to my OS!\n");
  print("Kernel loaded successfully!\n");
  print("This is written in C!\n");

  idtp.limit = (sizeof(struct idtEntry) * IDT_ENTRIES) - 1;
  idtp.base = (unsigned int)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idtSetGate(i, 0, 0, 0);
  }

  remapPic();
  idtSetGate(33, (unsigned int)keyboardHandler, 0x08, 0x8E);
  idtLoad((unsigned int)&idtp);

  __asm__("sti");

  outb(0x21, 0xFD);
  while (1)
    ;
}
