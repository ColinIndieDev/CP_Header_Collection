#pragma once

#define _GNU_SOURCE

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../cpstd/cparena.h"
#include "../cpstd/cpbase.h"
#include "../cpstd/cphash.h"
#include "../cpstd/cpmath.h"

typedef enum { CPL_FILTER_LINEAR, CPL_FILTER_NEAREST } texture_filtering;

// {{{ Key Inputs

#define CPL_MOUSE_BUTTON_1 0
#define CPL_MOUSE_BUTTON_2 1
#define CPL_MOUSE_BUTTON_3 2
#define CPL_MOUSE_BUTTON_4 3
#define CPL_MOUSE_BUTTON_5 4
#define CPL_MOUSE_BUTTON_6 5
#define CPL_MOUSE_BUTTON_7 6
#define CPL_MOUSE_BUTTON_8 7
#define CPL_MOUSE_BUTTON_LAST CPL_MOUSE_BUTTON_8
#define CPL_MOUSE_BUTTON_LEFT CPL_MOUSE_BUTTON_1
#define CPL_MOUSE_BUTTON_RIGHT CPL_MOUSE_BUTTON_2
#define CPL_MOUSE_BUTTON_MIDDLE CPL_MOUSE_BUTTON_3

#define CPL_KEY_SPACE 32
#define CPL_KEY_APOSTROPHE 39
#define CPL_KEY_COMMA 44
#define CPL_KEY_MINUS 45
#define CPL_KEY_PERIOD 46
#define CPL_KEY_SLASH 47
#define CPL_KEY_0 48
#define CPL_KEY_1 49
#define CPL_KEY_2 50
#define CPL_KEY_3 51
#define CPL_KEY_4 52
#define CPL_KEY_5 53
#define CPL_KEY_6 54
#define CPL_KEY_7 55
#define CPL_KEY_8 56
#define CPL_KEY_9 57
#define CPL_KEY_SEMICOLON 59
#define CPL_KEY_EQUAL 61
#define CPL_KEY_A 65
#define CPL_KEY_B 66
#define CPL_KEY_C 67
#define CPL_KEY_D 68
#define CPL_KEY_E 69
#define CPL_KEY_F 70
#define CPL_KEY_G 71
#define CPL_KEY_H 72
#define CPL_KEY_I 73
#define CPL_KEY_J 74
#define CPL_KEY_K 75
#define CPL_KEY_L 76
#define CPL_KEY_M 77
#define CPL_KEY_N 78
#define CPL_KEY_O 79
#define CPL_KEY_P 80
#define CPL_KEY_Q 81
#define CPL_KEY_R 82
#define CPL_KEY_S 83
#define CPL_KEY_T 84
#define CPL_KEY_U 85
#define CPL_KEY_V 86
#define CPL_KEY_W 87
#define CPL_KEY_X 88
#define CPL_KEY_Y 89
#define CPL_KEY_Z 90
#define CPL_KEY_LEFT_BRACKET 91
#define CPL_KEY_BACKSLASH 92
#define CPL_KEY_RIGHT_BRACKET 93
#define CPL_KEY_GRAVE_ACCENT 96
#define CPL_KEY_WORLD_1 161
#define CPL_KEY_WORLD_2 162
#define CPL_KEY_ESCAPE 256
#define CPL_KEY_ENTER 257
#define CPL_KEY_TAB 258
#define CPL_KEY_BACKSPACE 259
#define CPL_KEY_INSERT 260
#define CPL_KEY_DELETE 261
#define CPL_KEY_RIGHT 262
#define CPL_KEY_LEFT 263
#define CPL_KEY_DOWN 264
#define CPL_KEY_UP 265
#define CPL_KEY_PAGE_UP 266
#define CPL_KEY_PAGE_DOWN 267
#define CPL_KEY_HOME 268
#define CPL_KEY_END 269
#define CPL_KEY_CAPS_LOCK 280
#define CPL_KEY_SCROLL_LOCK 281
#define CPL_KEY_NUM_LOCK 282
#define CPL_KEY_PRINT_SCREEN 283
#define CPL_KEY_PAUSE 284
#define CPL_KEY_F1 290
#define CPL_KEY_F2 291
#define CPL_KEY_F3 292
#define CPL_KEY_F4 293
#define CPL_KEY_F5 294
#define CPL_KEY_F6 295
#define CPL_KEY_F7 296
#define CPL_KEY_F8 297
#define CPL_KEY_F9 298
#define CPL_KEY_F10 299
#define CPL_KEY_F11 300
#define CPL_KEY_F12 301
#define CPL_KEY_F13 302
#define CPL_KEY_F14 303
#define CPL_KEY_F15 304
#define CPL_KEY_F16 305
#define CPL_KEY_F17 306
#define CPL_KEY_F18 307
#define CPL_KEY_F19 308
#define CPL_KEY_F20 309
#define CPL_KEY_F21 310
#define CPL_KEY_F22 311
#define CPL_KEY_F23 312
#define CPL_KEY_F24 313
#define CPL_KEY_F25 314
#define CPL_KEY_KP_0 320
#define CPL_KEY_KP_1 321
#define CPL_KEY_KP_2 322
#define CPL_KEY_KP_3 323
#define CPL_KEY_KP_4 324
#define CPL_KEY_KP_5 325
#define CPL_KEY_KP_6 326
#define CPL_KEY_KP_7 327
#define CPL_KEY_KP_8 328
#define CPL_KEY_KP_9 329
#define CPL_KEY_KP_DECIMAL 330
#define CPL_KEY_KP_DIVIDE 331
#define CPL_KEY_KP_MULTIPLY 332
#define CPL_KEY_KP_SUBTRACT 333
#define CPL_KEY_KP_ADD 334
#define CPL_KEY_KP_ENTER 335
#define CPL_KEY_KP_EQUAL 336
#define CPL_KEY_LEFT_SHIFT 340
#define CPL_KEY_LEFT_CONTROL 341
#define CPL_KEY_LEFT_ALT 342
#define CPL_KEY_LEFT_SUPER 343
#define CPL_KEY_RIGHT_SHIFT 344
#define CPL_KEY_RIGHT_CONTROL 345
#define CPL_KEY_RIGHT_ALT 346
#define CPL_KEY_RIGHT_SUPER 347
#define CPL_KEY_MENU 348
#define CPL_KEY_LAST GLFW_KEY_MENU

// }}}

// {{{ Colors

#define WHITE (vec4f){255.0f, 255.0f, 255.0f, 255.0f}
#define BLACK (vec4f){0.0f, 0.0f, 0.0f, 255.0f}
#define RED (vec4f){255.0f, 0.0f, 0.0f, 255.0f}
#define GREEN (vec4f){0.0f, 255.0f, 0.0f, 255.0f}
#define BLUE (vec4f){0.0f, 0.0f, 255.0f, 255.0f}

// }}}

// {{{ Logging

typedef enum { LOG_INFO, LOG_WARN, LOG_ERR } log_level;

void cpl_log(log_level level, char *message) {
    switch (level) {
    case LOG_INFO:
        printf("[CPL] [INFO]: %s", message);
        break;
    case LOG_WARN:
        printf("[CPL] [WARNING]: %s", message);
        break;
    case LOG_ERR:
        fprintf(stderr, "[CPL] [ERROR]: %s", message);
        break;
    }
}

// }}}

// {{{ Profiler

u32 cpl_get_heap_size() {
    struct mallinfo2 mi = mallinfo2();
    return mi.arena;
}

u32 cpl_get_heap_used() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks;
}

u32 cpl_get_heap_free() {
    struct mallinfo2 mi = mallinfo2();
    return mi.fordblks;
}

u32 cpl_get_stack_size() {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    size_t size = 0;
    pthread_attr_getstacksize(&attr, &size);
    pthread_attr_destroy(&attr);
    return size;
}

u32 cpl_get_stack_used() {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    void *base = NULL;
    size_t size = 0;
    pthread_attr_getstack(&attr, &base, &size);
    pthread_attr_destroy(&attr);
    char marker;
    void *cur = &marker;
    return (u32)(base + size - cur);
}

// }}}

// {{{ Shader

typedef struct {
    u32 id;
} shader;

b8 cpl_check_shader_compile_errors(u32 shader, char *type) {
    i32 success = 0;
    char infoLog[1024];

    if (strcmp(type, "PROGRAM") == 0) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "[CPL] [ERROR] Program linking error:\n%s\n",
                    infoLog);
            return false;
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "[CPL] [ERROR] Shader compilation error: %s\n%s\n",
                    type, infoLog);
            return false;
        }
    }
    return true;
}

char *cpl_read_shader_file(char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    u32 read = fread(buffer, 1, size, f);
    fclose(f);

    if (read != size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}

void cpl_create_shader(shader *s, char *vert_path, char *frag_path) {
    char *vert_code = cpl_read_shader_file(vert_path);
    char *frag_code = cpl_read_shader_file(frag_path);

    u32 vert = 0;
    u32 frag = 0;
    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, (const GLchar *const *)&vert_code, NULL);
    glCompileShader(vert);
    cpl_check_shader_compile_errors(vert, "VERTEX");
    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, (const GLchar *const *)&frag_code, NULL);
    glCompileShader(frag);
    cpl_check_shader_compile_errors(frag, "FRAGMENT");
    s->id = glCreateProgram();
    glAttachShader(s->id, vert);
    glAttachShader(s->id, frag);
    glLinkProgram(s->id);
    cpl_check_shader_compile_errors(s->id, "PROGRAM");

    free(vert_code);
    free(frag_code);
    glDeleteShader(vert);
    glDeleteShader(frag);
}

void cpl_use_shader(shader *s) { glUseProgram(s->id); }

void cpl_shader_set_b8(shader *s, char *name, b8 val) {
    glUniform1i(glGetUniformLocation(s->id, name), val);
}

void cpl_shader_set_i32(shader *s, char *name, i32 val) {
    glUniform1i(glGetUniformLocation(s->id, name), val);
}

void cpl_shader_set_f32(shader *s, char *name, f32 val) {
    glUniform1f(glGetUniformLocation(s->id, name), val);
}

void cpl_shader_set_rgba(shader *s, char *name, vec4f *c) {
    glUniform4f(glGetUniformLocation(s->id, name), c->r / 255.0f, c->g / 255.0f,
                c->b / 255.0f, c->a / 255.0f);
}

void cpl_shader_set_mat4f(shader *s, char *name, mat4f mat) {
    glUniformMatrix4fv(glGetUniformLocation(s->id, name), 1, GL_FALSE,
                       (const GLfloat *)mat.data);
}

void cpl_shader_set_vec2f(shader *s, char *name, vec2f *v) {
    glUniform2f(glGetUniformLocation(s->id, name), v->x, v->y);
}

void cpl_shader_set_vec3f(shader *s, char *name, vec3f *v) {
    glUniform3f(glGetUniformLocation(s->id, name), v->x, v->y, v->z);
}

// }}}

// {{{ Rectangle

typedef struct {
    vec2f pos;
    vec2f size;
    vec4f color;
    f32 rot;

    u32 vbo, vao, ebo;
} rect;

void cpl_create_rect(rect *r, vec2f *pos, vec2f *size, vec4f *color, f32 rot) {
    r->pos = *pos;
    r->size = *size;
    r->color = *color;
    r->rot = rot;

    f32 vertices[12] = {size->x, 0.0f,    0.0f, size->x, size->y, 0.0f,
                        0.0f,    size->y, 0.0f, 0.0f,    0.0f,    0.0f};
    u32 indices[6] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);
    glGenBuffers(1, &r->ebo);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cpl_destroy_rect(rect *r) {
    if (r->vao != 0 && glIsVertexArray(r->vao)) {
        glDeleteVertexArrays(1, &r->vao);
        r->vao = 0;
    }
    if (r->vbo != 0 && glIsBuffer(r->vbo)) {
        glDeleteBuffers(1, &r->vbo);
        r->vbo = 0;
    }
    if (r->ebo != 0 && glIsBuffer(r->ebo)) {
        glDeleteBuffers(1, &r->ebo);
        r->ebo = 0;
    }
}

void cpl_draw_rect_raw(shader *s, rect *r) {
    mat4f transform;
    mat4f_identity(&transform);

    vec3f center3 = {r->size.x * 0.5f, r->size.y * 0.5f, 0.0f};
    vec3f neg_center3 = {-r->size.x * 0.5f, -r->size.y * 0.5f, 0.0f};
    mat4f_translate(&transform, &center3);
    mat4f_rotate(&transform, cpm_rad(r->rot), &(vec3f){0.0f, 0.0f, 1.0f});
    mat4f_translate(&transform, &neg_center3);

    cpl_shader_set_mat4f(s, "transform", transform);
    cpl_shader_set_vec3f(s, "offset", &(vec3f){r->pos.x, r->pos.y, 0.0f});
    cpl_shader_set_rgba(s, "input_color", &r->color);

    glBindVertexArray(r->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

// }}}

// {{{ Triangle

typedef struct {
    vec2f pos;
    vec2f size;
    vec4f color;
    f32 rot;

    u32 vbo, vao;
} triangle;

void cpl_create_triangle(triangle *t, vec2f *pos, vec2f *size, vec4f *color,
                         f32 rot) {
    t->pos = *pos;
    t->size = *size;
    t->color = *color;
    t->rot = rot;

    f32 vertices[9] = {0.0f, 0.0f,           0.0f,    size->x, 0.0f,
                       0.0f, size->x / 2.0f, size->y, 0.0f};

    glGenVertexArrays(1, &t->vao);
    glGenBuffers(1, &t->vbo);
    glBindVertexArray(t->vao);
    glBindBuffer(GL_ARRAY_BUFFER, t->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cpl_destroy_triangle(triangle *t) {
    if (t->vao != 0 && glIsVertexArray(t->vao)) {
        glDeleteVertexArrays(1, &t->vao);
        t->vao = 0;
    }
    if (t->vbo != 0 && glIsBuffer(t->vbo)) {
        glDeleteBuffers(1, &t->vbo);
        t->vbo = 0;
    }
}

void cpl_draw_triangle_raw(shader *s, triangle *t) {
    mat4f transform;
    mat4f_identity(&transform);

    vec3f center3 = {t->size.x * 0.5f, t->size.y * 0.5f, 0.0f};
    vec3f neg_center3 = {-t->size.x * 0.5f, -t->size.y * 0.5f, 0.0f};
    mat4f_translate(&transform, &center3);
    mat4f_rotate(&transform, cpm_rad(t->rot), &(vec3f){0.0f, 0.0f, 1.0f});
    mat4f_translate(&transform, &neg_center3);

    cpl_shader_set_mat4f(s, "transform", transform);
    cpl_shader_set_vec3f(s, "offset", &(vec3f){t->pos.x, t->pos.y, 0.0f});
    cpl_shader_set_rgba(s, "input_color", &t->color);

    glBindVertexArray(t->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// }}}

// {{{ Circle

typedef struct {
    vec2f pos;
    f32 radius;
    vec4f color;

    u32 vao, vbo;
    i32 vertex_cnt;
} circle;

void cpl_create_circle(circle *c, vec2f *pos, f32 radius, vec4f *color) {
    c->pos = *pos;
    c->color = *color;
    c->radius = radius;

    i32 segments = CPM_MIN(32, (i32)cpm_ceilf(2.0f * CPM_PI * radius / 2.0f));
    u64 vertices_size = ((u64)(segments + 1) * 3) + 3;
    f32 *vertices = malloc(vertices_size * sizeof(f32));
    for (u32 i = 0; i < 3; i++) {
        vertices[i] = 0;
    }
    for (u32 i = 0; i <= segments; i++) {
        f32 theta = 2 * CPM_PI / (f32)segments * (f32)i;
        f32 x = 0.0f + (radius * cpm_cosf(theta));
        f32 y = 0.0f + (radius * cpm_sinf(theta));
        vertices[(u64)(i + 1) * 3] = x;
        vertices[((i + 1) * 3) + 1] = y;
        vertices[((i + 1) * 3) + 2] = 0.0f;
    }
    c->vertex_cnt = (i32)(vertices_size / 3);
    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);
    glBindVertexArray(c->vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
    glBufferData(GL_ARRAY_BUFFER, (i32)(vertices_size * sizeof(f32)), vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    free(vertices);
}

void cpl_destroy_circle(circle *c) {
    if (c->vao != 0 && glIsVertexArray(c->vao)) {
        glDeleteVertexArrays(1, &c->vao);
        c->vao = 0;
    }
    if (c->vbo != 0 && glIsBuffer(c->vbo)) {
        glDeleteBuffers(1, &c->vbo);
        c->vbo = 0;
    }
}

void cpl_draw_circle_raw(shader *s, circle *c) {
    mat4f transform;
    mat4f_identity(&transform);

    cpl_shader_set_mat4f(s, "transform", transform);
    cpl_shader_set_vec3f(s, "offset", &(vec3f){c->pos.x, c->pos.y, 0.0f});
    cpl_shader_set_rgba(s, "input_color", &c->color);

    glBindVertexArray(c->vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, c->vertex_cnt);
    glBindVertexArray(0);
}

// }}}

// {{{ Line

typedef struct {
    vec2f start, end;
    vec4f color;

    u32 vao, vbo;
} line;

void cpl_create_line(line *l, vec2f *start, vec2f *end, vec4f *color) {
    l->start = *start;
    l->end = *end;
    l->color = *color;

    f32 vertices[6] = {
        start->x, start->y, 0.0f, end->x, end->y, 0.0f,
    };

    glGenVertexArrays(1, &l->vao);
    glGenBuffers(1, &l->vbo);
    glBindVertexArray(l->vao);
    glBindBuffer(GL_ARRAY_BUFFER, l->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void cpl_destroy_line(line *l) {
    if (l->vao != 0 && glIsVertexArray(l->vao)) {
        glDeleteVertexArrays(1, &l->vao);
        l->vao = 0;
    }
    if (l->vbo != 0 && glIsBuffer(l->vbo)) {
        glDeleteBuffers(1, &l->vbo);
        l->vbo = 0;
    }
}

void cpl_draw_line_raw(shader *s, line *l) {
    mat4f transform;
    mat4f_identity(&transform);

    cpl_shader_set_mat4f(s, "transform", transform);
    cpl_shader_set_vec3f(s, "offset", &(vec3f){0.0f, 0.0f, 0.0f});
    cpl_shader_set_rgba(s, "input_color", &l->color);
    glBindVertexArray(l->vao);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

// }}}

// {{{ Text

typedef struct {
    u32 id;
    vec2f size;
    vec2f bearing;
    u32 advance;
} letter;

VEC_DEF(letter, vec_letters)

typedef struct {
    u32 vao, vbo;
    char *name;
    vec_letters letters;
} font;

void cpl_create_font(font *f, char *path, char *name,
                     texture_filtering filter) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        cpl_log(LOG_ERR, "Could not init FreeType Library");
        exit(-1);
    }

    if (access(path, F_OK) == -1) {
        char *message = malloc(100);
        snprintf(message, 100, "Failed to load %s", name);
        cpl_log(LOG_ERR, message);
        exit(-1);
    }

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face)) {
        cpl_log(LOG_ERR, "Failed to load font");
        exit(-1);
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    vec_letters_reserve(&f->letters, 128);
    for (u8 c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            cpl_log(LOG_ERR, "Failed to load Glyph");
            continue;
        }

        u32 tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                     (GLsizei)face->glyph->bitmap.width,
                     (GLsizei)face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        filter == CPL_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        filter == CPL_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);

        letter character = {.id = tex,
                            .size = {(f32)face->glyph->bitmap.width,
                                     (f32)face->glyph->bitmap.rows},
                            .bearing = {(f32)face->glyph->bitmap_left,
                                        (f32)face->glyph->bitmap_top},
                            .advance = face->glyph->advance.x};
        vec_letters_push_back(&f->letters, character);
    }
    f->name = name;

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glGenVertexArrays(1, &f->vao);
    glGenBuffers(1, &f->vbo);
    glBindVertexArray(f->vao);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void cpl_delete_font(font *f) {
    if (f->vao != 0 && glIsVertexArray(f->vao)) {
        glDeleteVertexArrays(1, &f->vao);
        f->vao = 0;
    }
    if (f->vbo != 0 && glIsBuffer(f->vbo)) {
        glDeleteBuffers(1, &f->vbo);
        f->vbo = 0;
    }
    for (u32 i = 0; i < f->letters.size; i++) {
        glDeleteTextures(1, &vec_letters_at(&f->letters, i)->id);
    }
    vec_letters_destroy(&f->letters);
}

void cpl_draw_text_raw(shader *s, font *f, char *text, vec2f *pos, f32 scale,
                       vec4f *color) {
    cpl_shader_set_vec3f(s, "text_color",
                         &(vec3f){color->r, color->g, color->b});
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(f->vao);

    for (u32 i = 0; i < strlen(text); i++) {
        letter *l = vec_letters_at(&f->letters, text[i]);

        f32 x_pos = pos->x + (l->bearing.x * scale);
        f32 y_pos =
            pos->y +
            ((vec_letters_at(&f->letters, 'H')->bearing.y - l->bearing.y) *
             scale);
        f32 width = l->size.x * scale;
        f32 height = l->size.y * scale;

        f32 vertices[6][4] = {{x_pos, y_pos + height, 0.0f, 1.0f},
                              {x_pos, y_pos, 0.0f, 0.0f},
                              {x_pos + width, y_pos, 1.0f, 0.0f},

                              {x_pos, y_pos + height, 0.0f, 1.0f},
                              {x_pos + width, y_pos, 1.0f, 0.0f},
                              {x_pos + width, y_pos + height, 1.0f, 1.0f}};

        glBindTexture(GL_TEXTURE_2D, l->id);
        glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        pos->x += ((f32)(l->advance >> 6)) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

vec2f cpl_get_text_size(font *f, char *text, f32 scale) {
    f32 width = 0.0f;
    f32 height = 0.0f;
    f32 max_above_base = 0.0f;
    f32 max_below_base = 0.0f;

    for (u32 i = 0; i < strlen(text); i++) {

        letter *l = vec_letters_at(&f->letters, text[i]);
        f32 h = l->size.y * scale;
        max_above_base = CPM_MAX(max_above_base, l->bearing.y * scale);
        max_below_base = CPM_MAX(max_below_base, (h - (l->bearing.y * scale)));
        width += (f32)(l->advance >> 6) * scale;
    }
    height = max_above_base + max_below_base;
    return (vec2f){width, height};
}

// }}}

// {{{ Texture & Texture2D

typedef struct {
    u32 id;
    vec2f size;
} texture;

void cpl_load_texture(texture *t, char *path, texture_filtering filter) {
    glGenTextures(1, &t->id);
    glBindTexture(GL_TEXTURE_2D, t->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filter == CPL_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filter == CPL_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
    stbi_set_flip_vertically_on_load(1);
    i32 width = 0;
    i32 height = 0;
    i32 channels = 0;
    u8 *data = stbi_load(path, &width, &height, &channels, 0);
    GLenum format = 0;
    if (channels == 1) {
        format = GL_RED;
    } else if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
    }
    if (data) {
        t->size.x = (f32)width;
        t->size.y = (f32)height;
        glTexImage2D(GL_TEXTURE_2D, 0, (i32)format, (i32)t->size.x,
                     (i32)t->size.y, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cpl_log(LOG_ERR, "Failed to load texture\n");
    }
    stbi_image_free(data);
}

void cpl_unload_texture(texture *t) {
    if (t->id != 0) {
        glDeleteTextures(1, &t->id);
    }
}

typedef struct {
    vec2f pos;
    vec2f size;
    f32 rot;
    vec4f color;
    texture *tex;

    u32 vbo, vao, ebo;
} texture2D;

void cpl_create_texture2D(texture2D *t, vec2f *pos, vec2f *size, f32 rot,
                          vec4f *color, texture *tex) {
    t->pos = *pos;
    t->size = *size;
    t->rot = rot;
    t->color = *color;
    t->tex = tex;

    f32 vertices[22] = {
        size->x, 0.0f,    0.0f, 1.0f, 1.0f, size->x, size->y, 0.0f, 1.0f, 0.0f,

        0.0f,    size->y, 0.0f, 0.0f, 0.0f, 0.0f,    0.0f,    0.0f, 0.0f, 1.0f};
    u32 indices[6] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &t->vao);
    glGenBuffers(1, &t->vbo);
    glGenBuffers(1, &t->ebo);
    glBindVertexArray(t->vao);
    glBindBuffer(GL_ARRAY_BUFFER, t->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32),
                          (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cpl_destroy_texture2D(texture2D *t) {
    if (t->vao != 0 && glIsVertexArray(t->vao)) {
        glDeleteVertexArrays(1, &t->vao);
        t->vao = 0;
    }
    if (t->vbo != 0 && glIsBuffer(t->vbo)) {
        glDeleteBuffers(1, &t->vbo);
        t->vbo = 0;
    }
    if (t->ebo != 0 && glIsBuffer(t->ebo)) {
        glDeleteBuffers(1, &t->ebo);
        t->ebo = 0;
    }
    t->tex = NULL;
}

void cpl_draw_texture2D_raw(shader *s, texture2D *t) {
    mat4f transform;
    mat4f_identity(&transform);

    vec3f pos3 = {t->pos.x, t->pos.y, 0.0f};
    mat4f_translate(&transform, &pos3);

    vec2f center = {t->pos.x + (t->size.x * 0.5f),
                    t->pos.y + (t->size.y * 0.5f)};
    mat4f_translate(&transform, &(vec3f){center.x, center.y, 0.0f});
    mat4f_rotate(&transform, cpm_rad(t->rot), &(vec3f){0.0f, 0.0f, 1.0f});
    mat4f_translate(&transform, &(vec3f){-center.x, -center.y, 0.0f});

    cpl_shader_set_i32(s, "tex", 0);
    cpl_shader_set_mat4f(s, "transform", transform);
    cpl_shader_set_rgba(s, "input_color", &t->color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->tex->id);
    glBindVertexArray(t->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// }}}

// {{{ Lights 2D

typedef struct {
    vec2f pos;
    f32 radius;
    f32 intensity;
    vec4f color;
} point_light_2D;

typedef struct {
    f32 intensity;
    vec4f color;
} global_light_2D;

// }}}

// {{{ General

// {{{ Variables

f32 cpl_screen_width = 0.0f;
f32 cpl_screen_height = 0.0f;

GLFWwindow *cpl_window = NULL;

mat4f cpl_projection_2D;

typedef enum {
    CPL_SHAPE_2D_UNLIT,
    CPL_SHAPE_2D_LIT,
    CPL_TEXT,
    CPL_TEXTURE_2D_UNLIT,
    CPL_DRAW_MODES_COUNT
} cpl_draw_mode;

cpl_draw_mode cpl_cur_draw_mode;

shader cpl_shaders[CPL_DRAW_MODES_COUNT];

i32 cpl_nb_frames = 0;
f32 cpl_last_time = 0.0f;
f32 cpl_last_frame = 0.0f;
f32 cpl_dt = 0.0f;
f32 cpl_time_scale = 1.0f;
i32 cpl_fps = 0;

HASHMAP_DEF(i32, b8, keys)

keys key_states;
keys prev_key_states;
keys mouse_button_states;
keys prev_mouse_button_states;

GLubyte *cpl_renderer;
GLubyte *cpl_vendor;
GLubyte *cpl_version;

typedef struct {
    vec2f pos;
    f32 zoom;
    f32 rot;
} cam_2D;

cam_2D cpl_cam_2D;

mat4f *cpl_cam_2D_get_view_mat(cam_2D *cam) {
    mat4f *view = malloc(sizeof(mat4f));
    mat4f_identity(view);

    mat4f_translate(view, &(vec3f){-cam->pos.x, -cam->pos.y, 0.0f});

    mat4f_translate(view, &(vec3f){cam->pos.x, cam->pos.y, 0.0f});
    mat4f_rotate(view, cpm_rad(cam->rot), &(vec3f){0.0f, 0.0f, 1.0f});
    mat4f_translate(view, &(vec3f){-cam->pos.x, -cam->pos.y, 0.0f});

    mat4f_translate(view, &(vec3f){cam->pos.x, cam->pos.y, 0.0f});
    mat4f_scale(view, &(vec3f){cam->zoom, cam->zoom, 1.0f});
    mat4f_translate(view, &(vec3f){-cam->pos.x, -cam->pos.y, 0.0f});

    return view;
}

// }}}

// {{{ Collisions

typedef struct {
    vec2f pos;
    vec2f size;
} rect_collider;

typedef struct {
    vec2f pos;
    vec2f size;
} triangle_collider;

typedef struct {
    vec2f pos;
    f32 radius;
} circle_collider;

b8 cpl_check_collision_rects(rect_collider *a, rect_collider *b) {
    b8 collision_x =
        a->pos.x + a->size.x >= b->pos.x && b->pos.x + b->size.x >= a->pos.x;
    b8 collision_y =
        a->pos.y + a->size.y >= b->pos.y && b->pos.y + b->size.y >= a->pos.y;

    return collision_x && collision_y;
}

b8 cpl_check_collision_circle_rect(circle_collider *a, rect_collider *b) {
    vec2f circleCenter = a->pos;
    vec2f rectCenter =
        vec2f_add(&b->pos, &(vec2f){b->size.x * 0.5f, b->size.y * 0.5f});
    vec2f halfExtents = (vec2f){b->size.x * 0.5f, b->size.y * 0.5f};
    vec2f difference = vec2f_sub(&circleCenter, &rectCenter);
    vec2f clamped = vec2f_clamp(
        &difference, &(vec2f){-halfExtents.x, -halfExtents.y}, &halfExtents);
    vec2f closest = vec2f_add(&rectCenter, &clamped);
    vec2f delta = vec2f_sub(&closest, &circleCenter);

    return vec2f_length(&delta) <= a->radius;
}

b8 cpl_check_collision_vec2f_rect(vec2f *a, rect_collider *b) {
    return b->pos.x < a->x && a->x < b->pos.x + b->size.x && b->pos.y < a->y &&
           a->y < b->pos.y + b->size.y;
}

b8 cpl_check_collision_circles(circle_collider *a, circle_collider *b) {
    vec2f dist = vec2f_sub(&a->pos, &b->pos);
    f32 distance2 = (dist.x * dist.x) + (dist.y * dist.y);
    f32 radius_sum = a->radius + b->radius;
    return distance2 <= radius_sum * radius_sum;
}

b8 cpl_check_collision_vec2f_circle(vec2f *a, circle_collider *b) {
    vec2f dist = vec2f_sub(a, &b->pos);
    f32 distance2 = (dist.x * dist.x) + (dist.y * dist.y);
    return distance2 <= b->radius * b->radius;
}

// }}}

// {{{ Window

void cpl_framebuffer_size_callback(GLFWwindow *window, i32 width, i32 height) {
    glViewport(0, 0, width, height);
    cpl_screen_width = (f32)width;
    cpl_screen_height = (f32)height;
    mat4f_ortho(&cpl_projection_2D, 0.0f, cpl_screen_width, cpl_screen_height,
                0.0f, -1.0f, 1.0f);
}

void cpl_init_shaders() {
    cpl_create_shader(&cpl_shaders[CPL_SHAPE_2D_UNLIT],
                      "shaders/vert/2D/shape.vert",
                      "shaders/frag/2D/shape_unlit.frag");
    cpl_create_shader(&cpl_shaders[CPL_SHAPE_2D_LIT],
                      "shaders/vert/2D/shape.vert",
                      "shaders/frag/2D/shape_lit.frag");
    cpl_create_shader(&cpl_shaders[CPL_TEXT], "shaders/vert/2D/text.vert",
                      "shaders/frag/2D/text.frag");
    cpl_create_shader(&cpl_shaders[CPL_TEXTURE_2D_UNLIT],
                      "shaders/vert/2D/texture.vert",
                      "shaders/frag/2D/texture.frag");
}
void cpl_init_window(i32 width, i32 height, char *title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);

    cpl_screen_width = (f32)width;
    cpl_screen_height = (f32)height;

    cpl_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (cpl_window == NULL) {
        fprintf(stderr, "[CPL] [ERROR] Failed to create window\n");
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(cpl_window);
    glfwSetFramebufferSizeCallback(cpl_window, cpl_framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
        fprintf(stderr, "[CPL] [ERROR] Failed to initialize GLAD");
        exit(-1);
    }

    cpl_cam_2D = (cam_2D){{0.0f, 0.0f}, 1.0f, 0.0f};
    mat4f_ortho(&cpl_projection_2D, 0.0f, cpl_screen_width, cpl_screen_height,
                0.0f, -1.0f, 1.0f);

    cpl_init_shaders();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    cpl_renderer = (GLubyte *)glGetString(GL_RENDERER);
    cpl_vendor = (GLubyte *)glGetString(GL_VENDOR);
    cpl_version = (GLubyte *)glGetString(GL_VERSION);

    keys_init(&key_states, GLFW_KEY_LAST);
    keys_init(&prev_key_states, GLFW_KEY_LAST);
    keys_init(&mouse_button_states, GLFW_MOUSE_BUTTON_LAST);
    keys_init(&prev_mouse_button_states, GLFW_MOUSE_BUTTON_LAST);
}

b8 cpl_window_should_close() { return glfwWindowShouldClose(cpl_window); }

void cpl_destroy_window() { glfwSetWindowShouldClose(cpl_window, 1); }

void cpl_close_window() { glfwTerminate(); }

// }}}

// {{{ Drawing

void cpl_clear_background(vec4f *color) {
    glClearColor(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f,
                 color->a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cpl_begin_draw(cpl_draw_mode draw_mode, b8 mode_2D) {
    cpl_cur_draw_mode = draw_mode;
    cpl_use_shader(&cpl_shaders[draw_mode]);

    mat4f view_projection_2D;
    if (mode_2D) {
        mat4f *view = cpl_cam_2D_get_view_mat(&cpl_cam_2D);
        mat4f_mul(&cpl_projection_2D, view,
                  &view_projection_2D);
        free(view);
    }
    cpl_shader_set_mat4f(&cpl_shaders[draw_mode], "projection",
                         mode_2D ? view_projection_2D : cpl_projection_2D);
}

void cpl_draw_rect(vec2f *pos, vec2f *size, vec4f *color, f32 rot) {
    rect r;
    cpl_create_rect(&r, pos, size, color, rot);
    cpl_draw_rect_raw(&cpl_shaders[cpl_cur_draw_mode], &r);
    cpl_destroy_rect(&r);
}

void cpl_draw_triangle(vec2f *pos, vec2f *size, vec4f *color, f32 rot) {
    triangle t;
    cpl_create_triangle(&t, pos, size, color, rot);
    cpl_draw_triangle_raw(&cpl_shaders[cpl_cur_draw_mode], &t);
    cpl_destroy_triangle(&t);
}

void cpl_draw_circle(vec2f *pos, f32 radius, vec4f *color) {
    circle c;
    cpl_create_circle(&c, pos, radius, color);
    cpl_draw_circle_raw(&cpl_shaders[cpl_cur_draw_mode], &c);
    cpl_destroy_circle(&c);
}

void cpl_draw_line(vec2f *start, vec2f *end, f32 thickness, vec4f *color) {
    line l;
    cpl_create_line(&l, start, end, color);
    glLineWidth(thickness);
    cpl_draw_line_raw(&cpl_shaders[cpl_cur_draw_mode], &l);
    glLineWidth(1.0f);
    cpl_destroy_line(&l);
}

void cpl_draw_text(font *font, char *text, vec2f *pos, f32 scale,
                   vec4f *color) {
    cpl_draw_text_raw(&cpl_shaders[cpl_cur_draw_mode], font, text, pos, scale,
                      color);
}

void cpl_draw_text_shadow(font *font, char *text, vec2f *pos, f32 scale,
                          vec4f *color, vec2f *shadow_off,
                          vec4f *shadow_color) {
    cpl_draw_text_raw(&cpl_shaders[cpl_cur_draw_mode], font, text,
                      &(vec2f){pos->x + shadow_off->x, pos->y + shadow_off->y},
                      scale, shadow_color);
    cpl_draw_text_raw(&cpl_shaders[cpl_cur_draw_mode], font, text, pos, scale,
                      color);
}

void cpl_draw_texture2D(texture *tex, vec2f *pos, vec2f *size, vec4f *color,
                        f32 rot) {
    texture2D t;
    cpl_create_texture2D(&t, pos, size, rot, color, tex);
    cpl_draw_texture2D_raw(&cpl_shaders[cpl_cur_draw_mode], &t);
    cpl_destroy_texture2D(&t);
}

// }}}

void cpl_reset_shader() { cpl_use_shader(&cpl_shaders[cpl_cur_draw_mode]); }

// {{{ Lighting 2D

void cpl_set_ambient_light_2D(f32 strength) {
    shader *ss = &cpl_shaders[CPL_SHAPE_2D_LIT];
    cpl_use_shader(ss);
    cpl_shader_set_f32(ss, "ambient", strength);

    // TODO add for texture 2D extra version if texture_lit exists

    cpl_reset_shader();
}
void cpl_set_global_light_2D(global_light_2D *l) {
    shader *ss = &cpl_shaders[CPL_SHAPE_2D_LIT];
    cpl_use_shader(ss);
    cpl_shader_set_f32(ss, "g_light.intensity", l->intensity);
    cpl_shader_set_rgba(ss, "g_light.color", &l->color);

    // TODO add for texture 2D extra version if texture_lit exists

    cpl_reset_shader();
}

void cpl_add_point_lights_2D(point_light_2D *ls, u32 size) {
    shader *ss = &cpl_shaders[CPL_SHAPE_2D_LIT];
    cpl_use_shader(ss);

    cpl_shader_set_i32(ss, "point_lights_cnt", (i32)size);

    mem_arena *arena = mem_arena_create(KiB(1));
    for (u32 i = 0; i < size; i++) {
        char *pos = mem_arena_push(arena, 50, true);
        snprintf(pos, 50, "point_lights[%d].pos", i);
        char *radius = mem_arena_push(arena, 50, true);
        snprintf(radius, 50, "point_lights[%d].r", i);
        char *intensity = mem_arena_push(arena, 50, true);
        snprintf(intensity, 50, "point_lights[%d].intensity", i);
        char *color = mem_arena_push(arena, 50, true);
        snprintf(color, 50, "point_lights[%d].color", i);

        cpl_shader_set_vec2f(ss, pos, &ls[i].pos);
        cpl_shader_set_f32(ss, radius, ls[i].radius);
        cpl_shader_set_f32(ss, intensity, ls[i].intensity);
        cpl_shader_set_rgba(ss, color, &ls[i].color);

        mem_arena_clear(arena);
    }
    mem_arena_destroy(arena);

    // TODO add for texture 2D extra version if texture_lit exists

    cpl_reset_shader();
}

// }}}

void cpl_update_input() {
    prev_key_states = key_states;
    for (i32 key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        keys_put(&key_states, key, glfwGetKey(cpl_window, key) == GLFW_PRESS);
    }

    prev_mouse_button_states = mouse_button_states;
    for (i32 button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST;
         button++) {
        keys_put(&mouse_button_states, button,
                 glfwGetMouseButton(cpl_window, button) == GLFW_PRESS);
    }
}

b8 cpl_is_key_down(i32 key) { return *keys_get(&key_states, key); }
b8 cpl_is_key_up(i32 key) { return !(*keys_get(&key_states, key)); }
b8 cpl_is_key_pressed(i32 key) {
    return (*keys_get(&key_states, key)) && !(*keys_get(&prev_key_states, key));
}
b8 cpl_is_key_released(i32 key) {
    return !(*keys_get(&key_states, key)) && (*keys_get(&prev_key_states, key));
}

b8 cpl_is_mouse_down(i32 button) {
    return *keys_get(&mouse_button_states, button);
}
b8 cpl_is_mouse_pressed(i32 button) {
    return (*keys_get(&mouse_button_states, button)) &&
           !(*keys_get(&prev_mouse_button_states, button));
}
b8 cpl_is_mouse_released(i32 button) {
    return !(*keys_get(&mouse_button_states, button)) &&
           (*keys_get(&prev_mouse_button_states, button));
}
vec2f cpl_get_mouse_pos() {
    f64 x = 0;
    f64 y = 0;
    glfwGetCursorPos(cpl_window, &x, &y);
    return (vec2f){(f32)x, (f32)y};
}

b8 cpl_is_key_down_old(i32 key) {
    if (glfwGetKey(cpl_window, key) == GLFW_PRESS) {
        return true;
    }
    return false;
}

// {{{ Timing

void cpl_calc_fps() {
    f32 cur_time = (f32)glfwGetTime();
    cpl_nb_frames++;
    if (cur_time - cpl_last_time >= 1.0) {
        cpl_fps = cpl_nb_frames;
        cpl_nb_frames = 0;
        cpl_last_time += 1.0f;
    }
}
i32 cpl_get_fps() { return cpl_fps; }

void cpl_calc_dt() {
    f32 cur_frame = (f32)glfwGetTime();
    cpl_dt = (cur_frame - cpl_last_frame) * cpl_time_scale;
    cpl_last_frame = cur_frame;
}

f32 cpl_get_dt() { return cpl_dt; }
f32 cpl_get_time() { return (f32)glfwGetTime(); }
f32 cpl_get_time_scale() { return cpl_time_scale; }

f32 cpl_get_screen_width() { return cpl_screen_width; }
f32 cpl_get_screen_height() { return cpl_screen_height; }

void cpl_set_time_scale(f32 scale) { cpl_time_scale = scale; }

// }}}

void cpl_enable_vsync(b8 enabled) { glfwSwapInterval(enabled); }

void cpl_update() {
    cpl_calc_fps();
    cpl_calc_dt();
}

void cpl_end_frame() {
    glfwSwapBuffers(cpl_window);
    glfwPollEvents();
}

void cpl_display_details(font *font) {
    cpl_begin_draw(CPL_TEXT, false);

    mem_arena *arena = mem_arena_create(KiB(1));

    char *version_str = mem_arena_push(arena, 50, true);
    snprintf(version_str, 50, "OpenGL version: %s", cpl_version);
    cpl_draw_text(font, version_str, &(vec2f){10.0f, 10.0f}, 0.5f, &WHITE);

    char *renderer_str = mem_arena_push(arena, 50, true);
    snprintf(renderer_str, 50, "Renderer: %s", cpl_renderer);
    cpl_draw_text(font, renderer_str, &(vec2f){10.0f, 40.0f}, 0.5f, &WHITE);

    char *vendor_str = mem_arena_push(arena, 50, true);
    snprintf(vendor_str, 50, "Vendor: %s", cpl_vendor);
    cpl_draw_text(font, vendor_str, &(vec2f){10.0f, 70.0f}, 0.5f, &WHITE);

    char *fps = mem_arena_push(arena, 15, true);
    snprintf(fps, 15, "FPS: %d", cpl_get_fps());
    cpl_draw_text(font, fps, &(vec2f){10.0f, 100.0f}, 0.5f, &WHITE);

    char *stack_used = mem_arena_push(arena, 50, true);
    snprintf(stack_used, 50, "Stack used: %.3f / %.3f MB (%f%%)",
             MB((f32)cpl_get_stack_used()), MB((f32)cpl_get_stack_size()),
             (f32)cpl_get_stack_used() / (f32)cpl_get_stack_size());
    cpl_draw_text(font, stack_used, &(vec2f){10.0f, 130.0f}, 0.5f, &WHITE);

    char *heap_total = mem_arena_push(arena, 50, true);
    snprintf(heap_total, 50, "Heap size: %d MB",
             (i32)MB((f32)cpl_get_heap_size()));
    cpl_draw_text(font, heap_total, &(vec2f){10.0f, 160.0f}, 0.5f, &WHITE);

    char *heap_used = mem_arena_push(arena, 50, true);
    snprintf(heap_used, 50, "Heap used: %d MB",
             (i32)MB((f32)cpl_get_heap_used()));
    cpl_draw_text(font, heap_used, &(vec2f){10.0f, 190.0f}, 0.5f, &WHITE);

    char *heap_free = mem_arena_push(arena, 50, true);
    snprintf(heap_free, 50, "Heap free: %d MB",
             (i32)MB((f32)cpl_get_heap_free()));
    cpl_draw_text(font, heap_free, &(vec2f){10.0f, 220.0f}, 0.5f, &WHITE);

    mem_arena_destroy(arena);
}

// }}}
