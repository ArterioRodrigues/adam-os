#ifndef ABI_TYPES_H
#define ABI_TYPES_H

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define NULL 0
#define false 0
#define true 1
#define bool uint8_t

typedef enum { RUNNING, READY, WAITING, ZOMBIE } process_status_t;

typedef struct {
    uint32_t pid;
    uint32_t parent_pid;
    process_status_t status;
} ps_entry_t;

typedef struct {
    int x;
    int y;
    uint32_t width;
    uint32_t height;
    char title[32];
} create_window_t;

typedef struct {
    uint32_t window_id;
    int x;
    int y;
    uint32_t width;
    uint32_t height;
    uint8_t color;
} create_rect_t;

typedef struct {
    uint32_t window_id;
    int x0;
    int y0;
    int x1;
    int y1;
    uint8_t color;
} create_line_t;

typedef struct {
    uint32_t window_id;
    int x;
    int y;
    char *str;
    uint8_t color;
} create_text_t;

typedef enum { EVENT_KEYPRESS, EVENT_KEYRELEASE, EVENT_MOUSE_CLICK, EVENT_MOUSE_MOVE } event_type_t;
typedef struct {
    event_type_t type;

    uint8_t keyboard_scancode;
    int mouse_x;
    int mouse_y;
    uint8_t mouse_button;

    char c;
} event_t;

typedef struct fat16_entry {
    uint8_t name[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved[14];
    uint16_t start_cluster;
    uint32_t file_size;
} __attribute__((packed)) fat16_entry_t;

#endif
