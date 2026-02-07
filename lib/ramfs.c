#include "ramfs.h"
#include "../pch.h"

ramfs_node_t *ramfs_root;

void init_ramfs() {
  ramfs_root = kmalloc(sizeof(ramfs_node_t));
  strcpy(ramfs_root->name, "/");
  ramfs_root->type = RAMFS_DIR;
  ramfs_root->parent = NULL;
  ramfs_root->children = NULL;
  ramfs_root->next = NULL;
};

ramfs_node_t *create_empty_file(char *name) {
  ramfs_node_t *file = kmalloc(sizeof(ramfs_node_t));
  strcpy(file->name, name);
  file->type = RAMFS_FILE;

  file->parent = NULL;
  file->children = NULL;
  file->next = NULL;

  return file;
}

void make_file(char *path, char *name) {
  ramfs_node_t *current = ramfs_find(path);
  if (current->type == RAMFS_DIR && strcmp(current->name, path)) {
    ramfs_node_t *child = current->children;

    if (child == NULL) {
      child = create_empty_file(name);
      child->parent = current;
      current->children = child;
    }

    else {

      ramfs_node_t *prev = child;
      while (child != NULL) {
        prev = child;
        child = child->next;
      }

      child = create_empty_file(name);
      child->parent = current;
      prev->next = child;
    }
  }
}

void ramfs_ls(char *path) {

  ramfs_node_t *current = ramfs_find(path);
  ramfs_node_t *child = current->children;

  print("\n");
  while (child != NULL) {
    print(child->name);
    print("\n");

    child = child->next;
  }
}

ramfs_node_t *ramfs_find(char *path) {
  if (strcmp(path, "/") == 0)
    return ramfs_root;

  char temp[256];
  strcpy(temp, path);

  ramfs_node_t *current = ramfs_root;
  char *token = strtok(token, '/');

  while (token) {
    ramfs_node_t *child = current->children;
    while (child) {
      if (strcmp(child->name, token) == 0) {
        current = child;
        break;
      }
      child = child->next;
    }
    if (!child)
      return NULL;
    token = strtok(NULL, '/');
  }
  return current;
}
