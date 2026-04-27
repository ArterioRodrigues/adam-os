#ifndef OBJ_H
#define OBJ_H

#define MAX_VERTICES 256
#define MAX_FACES    512

typedef struct {
    float x, y, z;
} obj_vertex_t;

typedef struct {
    int a, b, c;
} obj_face_t;

void parse_obj(const char *buf, obj_vertex_t *vertices, int *vertex_count, obj_face_t *faces, int *face_count);
#endif
