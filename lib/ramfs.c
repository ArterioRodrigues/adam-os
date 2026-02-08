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

ramfs_node_t *ramfs_create(ramfs_type_t type, char *name) {
  ramfs_node_t *file = kmalloc(sizeof(ramfs_node_t));
  strcpy(file->name, name);

  file->type = type;
  file->parent = NULL;
  file->children = NULL;
  file->next = NULL;

  return file;
}

void ramfs_make(ramfs_type_t type, char *path, char *name) {
  ramfs_node_t *current = ramfs_find(path);
  if (current->type != RAMFS_DIR) {
    return;
  }

  ramfs_node_t *child = current->children;

  if (child == NULL) {
    child = ramfs_create(type, name);
    child->parent = current;
    current->children = child;
    return;
  }

  ramfs_node_t *prev = child;
  while (child != NULL) {
    if (strcmp(child->name, name))
      return;
    prev = child;
    child = child->next;
  }

  child = ramfs_create(type, name);
  child->parent = current;
  prev->next = child;
}

ramfs_node_t *ramfs_find(char *path) {
  if (strcmp(path, "/"))
    return ramfs_root;

  char temp[256];
  strcpy(temp, path);

  ramfs_node_t *current = ramfs_root;
  char token[50];
  strtok(token, temp, '/');

  while (*token != NULL) {
    ramfs_node_t *child = current->children;

    if (strcmp(token, "..")) {
      current = current->parent;
      child = current->parent->children;
    }

    while (child) {
      if (strcmp(child->name, token)) {
        current = child;
        break;
      }
      child = child->next;
    }

    if (!child) {
      return NULL;
    }
    strtok(token, NULL, '/');
  }
  return current;
}

bool ramfs_write(char *path, char *content) {
  ramfs_node_t *file = ramfs_find(path);

  if (!file || file->type != RAMFS_FILE) {
    return false;
  }

  file->size = strlen(content);
  file->data = kmalloc(file->size + 1);

  if (!file->data) {
    return false;
  }

  strcpy((char *)file->data, content);
  return true;
}

