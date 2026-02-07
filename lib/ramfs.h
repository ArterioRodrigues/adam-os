#ifndef RAMFS_H
#define RAMFS_H
#include "../kernel/types.h"

typedef enum { RAMFS_FILE, RAMFS_DIR } ramfs_type_t;

typedef struct ramfs_node {
  char name[64];
  ramfs_type_t type;

  struct ramfs_node *parent;
  struct ramfs_node *children;
  struct ramfs_node *next;

  uint8_t *data;
  uint32_t size;
} ramfs_node_t;

extern ramfs_node_t *ramfs_root;

void init_ramfs();
void make_file(char *path, char *name);
void ramfs_ls(char *path);
ramfs_node_t *ramfs_find(char *path);
#endif
