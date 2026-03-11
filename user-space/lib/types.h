#ifndef TYPES_H
#define TYPES_H

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
    char *name;
} ps_entry_t;
#endif
