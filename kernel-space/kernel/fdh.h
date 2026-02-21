#ifndef FD_H
#define FD_H
#include "types.h"

#define MAX_FDS 16

typedef enum { FD_NONE, FD_FILE, FD_PIPE, FD_STDIN, FD_STDOUT } fd_type_t;

typedef struct {
    fd_type_t type;
    void *data;
    uint32_t offset;
    bool is_open;
} file_descriptor_t;

#endif
