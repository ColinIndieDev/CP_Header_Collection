#pragma once

#include "cpbase.h"
#include "cpvec.h"

#define CPM_PI 3.14159265358979323846f

#define CPM_MIN(x, y) ((x) < (y) ? (x) : (y))
#define CPM_MAX(x, y) ((x) > (y) ? (x) : (y))
#define CPM_ABS(x) ((x) > 0 ? (x) : -(x))
#define CPM_CLAMP(x, n, m) ((x) > (n) ? ((x) < (m) ? (x) : (m)) : (n))

VEC_DEF(f32, vecf)

// {{{ Arithmetic

f32 cpm_factorial(i32 n) {
    if (n == 0 || n == 1) {
        return 1.0f;
    }
    f32 result = 1.0f;
    for (int i = 2; i <= n; i++) {
        result *= (f32)i;
    }
    return result;
}

f32 cpm_expf(f32 x) {
    if (x > 88.0f) {
        return CPM_F32_MAX;
    }
    if (x < -88.0f) {
        return 0.0f;
    }

    f32 ln2 = 0.693147180559945f;
    i32 k = (i32)((x / ln2) + 0.5f);
    f32 r = x - ((f32)k * ln2);

    f32 result = 1.0f + r + ((r * r) / 2.0f) + ((r * r * r) / 6.0f) +
                 ((r * r * r * r) / 24.0f) + ((r * r * r * r * r) / 120.0f) +
                 ((r * r * r * r * r * r) / 720.0f);

    i32 bits = (k + 127) << 23;
    f32 pow2k;
    memcpy(&pow2k, &bits, sizeof(f32));

    return result * pow2k;
}

f32 cpm_powf(f32 x, i32 n) {
    f32 result = 1.0f;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return n > 0 ? result : 1 / result;
}

f32 cpm_sinf(f32 x) {
    f32 result = 0.0f;

    while (x > CPM_PI) {
        x -= 2 * CPM_PI;
    }
    while (x < -CPM_PI) {
        x += 2 * CPM_PI;
    }

    i32 n = 10;
    for (int i = 0; i < n; i++) {
        f32 sign = cpm_powf(-1, i);
        f32 num = cpm_powf(x, (2 * i) + 1);
        f32 den = cpm_factorial((2 * i) + 1);
        result += sign * (num / den);
    }
    return result;
}

f32 cpm_cosf(f32 x) {
    f32 result = 1.0f;
    f32 term = 1.0f;

    while (x > CPM_PI) {
        x -= 2 * CPM_PI;
    }
    while (x < -CPM_PI) {
        x += 2 * CPM_PI;
    }

    i32 n = 10;
    for (int i = 1; i <= n; i++) {
        f32 sign = -cpm_powf(x, 2);
        f32 num = 2.0f * (f32)i;
        term *= sign / (num * (num - 1));
        result += term;
    }
    return result;
}

f32 cpm_tanf(f32 x) { return cpm_sinf(x) / cpm_cosf(x); }

f32 cpm_sinhf(f32 x) { return (cpm_expf(x) - cpm_expf(-x)) / 2; }

f32 cpm_coshf(f32 x) { return (cpm_expf(x) + cpm_expf(-x)) / 2; }

f32 cpm_tanhf(f32 x) { return cpm_sinhf(x) / cpm_coshf(x); }

f32 cpm_sqrt(f32 n) {
    if (n < 0) {
        return -1.0f;
    }
    if (n == 0) {
        return 0.0f;
    }
    f32 tolerance = 1e-5f;
    f32 guess = n / 2.0f;
    for (u32 i = 0; i < 100; i++) {
        f32 newGuess = (guess + (n / guess)) / 2.0f;
        if (CPM_ABS((newGuess * newGuess) - n) < tolerance) {
            return newGuess;
        }
        guess = newGuess;
    }
    return guess;
}

f32 cpm_logf(f32 x) {
    if (x <= 0) {
        return -CPM_F32_MAX;
    }
    f32 y = x - 1.0f;
    for (i32 i = 0; i < 100; i++) {
        f32 ey = cpm_expf(y);
        y -= (ey - x) / ey;
    }
    return y;
}

f32 cpm_modf(f32 x, f32 y) {
    u32 fit = (u32)x / (u32)y;
    return x - (y * (float)fit);
}

f32 cpm_floorf(f32 x) { return (f32)((i32)x); }

f32 cpm_ceilf(f32 x) {

    return x > (f32)((i32)x) ? (f32)((i32)x + 1) : (f32)((i32)x);
}

f32 cpm_rad(f32 deg) { return deg * (CPM_PI / 180.0f); }

b8 cpm_isnan(f32 x) {
    /* Prevent this being changed
     * by compiler optimizations
     * (fastmath)
     */
    volatile f32 y = x;
    return (b8)(y != y);
}

// }}}

// {{{ Linear algebra

typedef struct {
    vecf data;
    u32 rows, cols;
} mat2D;

void mat2D_init(mat2D *m, u32 rows, u32 cols, f32 val) {
    vecf_init(&m->data, (u64)rows * cols, val);
    m->rows = rows;
    m->cols = cols;
}

f32 *mat2D_at(mat2D *m, u32 row, u32 col) {
    assert(row < m->rows && col < m->cols);

    return &m->data.data[(row * m->cols) + col];
}

f32 mat2D_get(mat2D *m, u32 row, u32 col) {
    assert(row < m->rows && col < m->cols);

    return m->data.data[(row * m->cols) + col];
}

f32 *mat2D_row_ptr(mat2D *m, u32 row) {
    return &m->data.data[(u64)row * m->cols];
}
void mat2D_get_row(mat2D *m, u32 row, f32 *out) {
    assert(row < m->rows);

    memcpy(out, &m->data.data[(u64)row * m->cols], m->cols * sizeof(f32));
}

u32 mat2D_size(mat2D *m) { return m->data.size; }

void mat2D_destroy(mat2D *m) {
    vecf_destroy(&m->data);
    m->rows = 0;
    m->cols = 0;
}

void mat2D_add(mat2D *m1, mat2D *m2, mat2D *out) {
    assert(m1->rows == m2->rows && m1->cols == m2->cols &&
           out->rows == m1->rows && out->cols == m1->cols);

    for (u32 i = 0; i < m1->data.size; i++) {
        out->data.data[i] = m1->data.data[i] + m2->data.data[i];
    }
}
void mat2D_sub(mat2D *m1, mat2D *m2, mat2D *out) {
    assert(m1->rows == m2->rows && m1->cols == m2->cols &&
           out->rows == m1->rows && out->cols == m1->cols);

    for (u32 i = 0; i < m1->data.size; i++) {
        out->data.data[i] = m1->data.data[i] - m2->data.data[i];
    }
}
void mat2D_mul(mat2D *m1, mat2D *m2, mat2D *out) {
    assert(m1->rows == m2->rows && m1->cols == m2->cols &&
           out->rows == m1->rows && out->cols == m1->cols);

    for (u32 i = 0; i < m1->data.size; i++) {
        out->data.data[i] = m1->data.data[i] * m2->data.data[i];
    }
}
void mat2D_div(mat2D *m1, mat2D *m2, mat2D *out) {
    assert(m1->rows == m2->rows && m1->cols == m2->cols &&
           out->rows == m1->rows && out->cols == m1->cols);

    for (u32 i = 0; i < m1->data.size; i++) {
        out->data.data[i] = m1->data.data[i] / m2->data.data[i];
    }
}

void mat2D_print(mat2D *m) {
    for (int r = 0; r < m->rows; r++) {
        for (int c = 0; c < m->cols; c++) {
            printf("%f", *mat2D_at(m, r, c));

            if (c < m->cols - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

typedef union {
    f32 data[4];
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
} vec4f;
typedef union {
    f32 data[3];
    struct {
        f32 x, y, z;
    };
} vec3f;
typedef union {
    f32 data[2];
    struct {
        f32 x, y;
    };
} vec2f;

vec2f vec2f_add(vec2f *a, vec2f *b) {
    return (vec2f){a->x + b->x, a->y + b->y};
}
vec2f vec2f_sub(vec2f *a, vec2f *b) {
    return (vec2f){a->x - b->x, a->y - b->y};
}
vec2f vec2f_mul(vec2f *a, vec2f *b) {
    return (vec2f){a->x * b->x, a->y * b->y};
}
vec2f vec2f_div(vec2f *a, vec2f *b) {
    return (vec2f){a->x / b->x, a->y / b->y};
}

f32 vec2f_dist(vec2f *v1, vec2f *v2) {
    f32 a = CPM_ABS(v1->x - v2->x);
    f32 b = CPM_ABS(v1->y - v2->y);

    return cpm_sqrt((a * a) + (b * b));
}

f32 vec2f_dist2(vec2f *v1, vec2f *v2) {
    f32 a = CPM_ABS(v1->x - v2->x);
    f32 b = CPM_ABS(v1->y - v2->y);

    return (a * a) + (b * b);
}

f32 vec2f_dot(vec2f *a, vec2f *b) { return (a->x * b->x) + (a->y * b->y); }

f32 vec2f_length(vec2f *a) { return cpm_sqrt((a->x * a->x) + (a->y * a->y)); }

vec2f vec2f_clamp(vec2f *v, vec2f *n, vec2f *m) {
    return (vec2f){CPM_CLAMP(v->x, n->x, m->x), CPM_CLAMP(v->y, n->y, m->y)};
}

typedef struct {
    f32 data[4][4];
} mat4f;

void mat4f_identity(mat4f *m) {
    for (u32 j = 0; j < 4; j++) {
        for (u32 i = 0; i < 4; i++) {
            m->data[i][j] = i == j ? 1.0f : 0.0f;
        }
    }
}

void mat4f_translate(mat4f *m, vec3f *v) {
    m->data[3][0] += (m->data[0][0] * v->x) + (m->data[1][0] * v->y) +
                     (m->data[2][0] * v->z);
    m->data[3][1] += (m->data[0][1] * v->x) + (m->data[1][1] * v->y) +
                     (m->data[2][1] * v->z);
    m->data[3][2] += (m->data[0][2] * v->x) + (m->data[1][2] * v->y) +
                     (m->data[2][2] * v->z);
    m->data[3][3] += (m->data[0][3] * v->x) + (m->data[1][3] * v->y) +
                     (m->data[2][3] * v->z);
}

void mat4f_rotate(mat4f *m, f32 angle_rad, vec3f *axis) {
    f32 c = cpm_cosf(angle_rad);
    f32 s = cpm_sinf(angle_rad);
    f32 t = 1.0f - c;

    f32 x = axis->x;
    f32 y = axis->y;
    f32 z = axis->z;

    mat4f rot;
    mat4f_identity(&rot);

    rot.data[0][0] = (t * x * x) + c;
    rot.data[0][1] = (t * x * y) + (s * z);
    rot.data[0][2] = (t * x * z) - (s * y);

    rot.data[1][0] = (t * x * y) - (s * z);
    rot.data[1][1] = (t * y * y) + c;
    rot.data[1][2] = (t * y * z) + (s * x);

    rot.data[2][0] = (t * x * z) + (s * y);
    rot.data[2][1] = (t * y * z) - (s * x);
    rot.data[2][2] = (t * z * z) + c;

    mat4f result;
    mat4f_identity(&result);
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 4; k++) {
                sum += m->data[k][row] * rot.data[col][k];
            }
            result.data[col][row] = sum;
        }
    }
    *m = result;
}

void mat4f_scale(mat4f *m, vec3f *v) {
    m->data[0][0] *= v->x;
    m->data[1][1] *= v->y;
    m->data[2][2] *= v->z;
}

void mat4f_mul(mat4f *a, mat4f *b, mat4f *dest) {
    mat4f_identity(dest);
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 4; k++) {
                sum += a->data[k][row] * b->data[col][k];
            }
            dest->data[col][row] = sum;
        }
    }
}

void mat4f_ortho(mat4f *m, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                 f32 far) {
    mat4f_identity(m);
    m->data[0][0] = 2.0f / (right - left);
    m->data[1][1] = 2.0f / (top - bottom);
    m->data[2][2] = -2.0f / (far - near);
    m->data[3][0] = -(right + left) / (right - left);
    m->data[3][1] = -(top + bottom) / (top - bottom);
    m->data[3][2] = -(far + near) / (far - near);
}

// }}}
