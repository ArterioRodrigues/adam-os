#ifndef RENDER_H
#define RENDER_H

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float m[4][4];
} mat4_t;

typedef struct {
    int x, y;
    bool valid;
} screen_point_t;

vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_scale(vec3_t v, float s);
vec3_t vec3_cross(vec3_t a, vec3_t b);
float vec3_dot(vec3_t a, vec3_t b);

mat4_t mat4_identity(void);
mat4_t mat4_mul(mat4_t a, mat4_t b);
vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);

mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);

screen_point_t project(vec3_t v, float focal_length, int screen_w, int screen_h);
#endif

