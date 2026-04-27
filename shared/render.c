#include "render.h"
#include "math.h"

vec3_t vec3_add(vec3_t a, vec3_t b) {
    vec3_t c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;

    return c;
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    vec3_t c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;

    return c;
}

vec3_t vec3_scale(vec3_t v, float s) {
    vec3_t c;

    c.x = v.x * s;
    c.y = v.y * s;
    c.z = v.z * s;

    return c;
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

float vec3_dot(vec3_t a, vec3_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

mat4_t mat4_identity(void) {
    mat4_t mat;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            mat.m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
    return mat;
}

mat4_t mat4_mul(mat4_t a, mat4_t b) {
    mat4_t c;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++)
                sum += a.m[i][k] * b.m[k][j];
            c.m[i][j] = sum;
        }
    }
    return c;
}

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v) {
    vec3_t r;
    r.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3];
    r.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3];
    r.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3];
    return r;
}

mat4_t mat4_rotate_x(float angle) {
    float c = cos_approx(angle);
    float s = sin_approx(angle);
    mat4_t mat = mat4_identity();
    mat.m[1][1] =  c;
    mat.m[1][2] = -s;
    mat.m[2][1] =  s;
    mat.m[2][2] =  c;
    return mat;
}

mat4_t mat4_rotate_y(float angle) {
    float c = cos_approx(angle);
    float s = sin_approx(angle);
    mat4_t mat = mat4_identity();
    mat.m[0][0] =  c;
    mat.m[0][2] =  s;
    mat.m[2][0] = -s;
    mat.m[2][2] =  c;
    return mat;
}

mat4_t mat4_rotate_z(float angle) {
    float c = cos_approx(angle);
    float s = sin_approx(angle);
    mat4_t mat = mat4_identity();
    mat.m[0][0] =  c;
    mat.m[0][1] = -s;
    mat.m[1][0] =  s;
    mat.m[1][1] =  c;
    return mat;
}
