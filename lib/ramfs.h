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
void ramfs_ls(char *path);
ramfs_node_t *ramfs_find(char *path);

bool ramfs_write(char *path, char *content);
void ramfs_cat(char *path);

void ramfs_make(ramfs_type_t type, char *path, char *name);

#define ramfs_make_folder(path, name) ramfs_make(RAMFS_DIR, path, name);
#define ramfs_make_file(path, name) ramfs_make(RAMFS_FILE , path, name);

#endif
