#ifndef OBJ_H
#define OBJ_H

#define MAX_VERTICES 4000
#define MAX_FACES 4000 

typedef struct {
    float x, y, z;
} obj_vertex_t;

typedef struct {
    int a, b, c;
} obj_face_t;

extern void sys_write(int fd, char *buf, int len);

void parse_obj(const char *buf, obj_vertex_t *vertices, int *vertex_count, obj_face_t *faces, int *face_count);
#endif
