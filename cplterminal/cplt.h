#pragma once

#define _GNU_SOURCE

#include <pthread.h>

#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "../cpstd/cpbase.h"

i32 width = 0;
i32 height = 0;
i8 *screen_buf;
i32 *col_buf;
i8 bg_col[22] = "\x1b[48;2;0;0;0m";

b8 frame_sync = true;
b8 running = true;

i32 nb_frames = 0;
i32 fps = 0;
f32 last_frame = 0.0f;
f32 last_fps = 0.0f;
f32 time_scale = 1.0f;
f32 dt = 0.0f;

#define CPLT_DEF_COLOR "\x1b[0m"
#define WHITE (rgb){255, 255, 255}
#define BLACK (rgb){0, 0, 0}
#define RED (rgb){255, 0, 0}
#define GREEN (rgb){0, 255, 0}
#define BLUE (rgb){0, 0, 255}
#define PINK (rgb){255, 0, 255}
#define YELLOW (rgb){255, 255, 0}
#define CYAN (rgb){0, 255, 255}

#define CPLT_MAX_KEYS 256
#define CPLT_KEY_TIMEOUT 0.6f

b8 keyDown[CPLT_MAX_KEYS] = {false};
b8 keyPressed[CPLT_MAX_KEYS] = {false};
b8 keyReleased[CPLT_MAX_KEYS] = {false};
f32 keyTimers[CPLT_MAX_KEYS] = {0};

typedef struct {
    u8 r, g, b;
} rgb;

typedef struct {
    f32 x, y, w, h;
} rect;

struct termios orig_termios;

void cplt_disable_raw_mode();
void cplt_activate_raw_mode();
void cplt_hide_cursor(b8 hide);
void cplt_init(i32 w, i32 h);

u32 cplt_get_heap_size();
u32 cplt_get_heap_used();
u32 cplt_get_heap_free();
u32 cplt_get_stack_size();
u32 cplt_get_stack_used();

f32 cplt_get_time();
f32 cplt_get_dt();
void cplt_calc_fps();
void cplt_calc_dt();
b8 cplt_check_collision_rects(rect *a, rect *b);

void cplt_update_input();
b8 cplt_is_key_down(i8 key);
b8 cplt_is_key_pressed(i8 key);
b8 cplt_is_key_released(i8 key);
i32 cplt_rgb_to_i32(rgb color);

void cplt_render();
void cplt_clear(i8 c, rgb color);
void cplt_clear_bg(rgb color);
void cplt_draw_pixel(i32 x, i32 y, const i8 *c, rgb color);
void cplt_draw_text(i32 x, i32 y, i8 *text, rgb color);
void cplt_draw_rect(i32 x, i32 y, i32 w, i32 h, i8 *c, rgb color);
void cplt_draw_circle(i32 cx, i32 cy, i32 r, i8 *c, rgb color);
void cplt_draw_circle_out(i32 cx, i32 cy, i32 r, i8 *c, rgb color);

#ifdef CPLT_IMPLEMENTATION

void cplt_disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void cplt_activate_raw_mode() {
#ifdef _WIN32
    fprintf(stderr, "Windows is not supported! (termios)\n");
    exit(-1);
#endif

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(cplt_disable_raw_mode);

    struct termios raw = orig_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void cplt_hide_cursor(b8 hide) {
    if (hide) {
        write(STDOUT_FILENO, "\x1b[?25l", 6);
    } else {
        write(STDOUT_FILENO, "\x1b[?25h", 6);
    }
}

void cplt_init(i32 w, i32 h) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    width = (w > 0 && w <= ws.ws_col) ? w : ws.ws_col;
    height = (h > 0 && h <= ws.ws_row) ? h : ws.ws_row;

    screen_buf = malloc((u64)width * height);
    col_buf = malloc((u64)width * height * sizeof(i32));
    memset(screen_buf, ' ', (u64)width * height);
    memset(col_buf, 0, (u64)width * height * sizeof(i32));

    write(STDOUT_FILENO, "\x1b[?1049h", 8);
    write(STDOUT_FILENO, "\x1b[2J", 4);
    cplt_hide_cursor(true);
    cplt_activate_raw_mode();
}

// {{{ Profiling

u32 cplt_get_heap_size() {
    struct mallinfo2 mi = mallinfo2();
    return mi.arena;
}

u32 cplt_get_heap_used() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks;
}

u32 cplt_get_heap_free() {
    struct mallinfo2 mi = mallinfo2();
    return mi.fordblks;
}

u32 cplt_get_stack_size() {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    size_t size = 0;
    pthread_attr_getstacksize(&attr, &size);
    pthread_attr_destroy(&attr);
    return size;
}

u32 cplt_get_stack_used() {
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

f32 cplt_get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (f32)ts.tv_sec + ((f32)ts.tv_nsec / 1e9f);
}

f32 cplt_get_dt() { return dt; }

void cplt_calc_fps() {
    f32 curTime = cplt_get_time();
    nb_frames++;
    if (curTime - last_fps >= 1.0) {
        fps = nb_frames;
        nb_frames = 0;
        last_fps = curTime;
    }
}

void cplt_calc_dt() {
    f32 curFrame = cplt_get_time();
    dt = (curFrame - last_frame) * time_scale;
    last_frame = curFrame;
}

b8 cplt_check_collision_rects(rect *a, rect *b) {
    return (a->x + a->w >= b->x && b->x + b->w >= a->x) &&
           (a->y + a->h >= b->y && b->y + b->h >= a->y);
}

// {{{ Inputs

void cplt_update_input() {
    f32 now = cplt_get_time();

    memset(keyPressed, false, sizeof(keyPressed));
    memset(keyReleased, false, sizeof(keyReleased));

    i8 buf[64];
    i32 n;
    while ((n = (i32)read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        for (i32 i = 0; i < n; i++) {
            u8 uc = (u8)buf[i];
            if (!keyDown[uc]) {
                keyPressed[uc] = true;
            }
            keyDown[uc] = true;
            keyTimers[uc] = now;
        }
    }

    for (i32 i = 0; i < CPLT_MAX_KEYS; i++) {
        if (keyDown[i] && (now - keyTimers[i] > CPLT_KEY_TIMEOUT)) {
            keyDown[i] = false;
            keyReleased[i] = true;
        }
    }
}

b8 cplt_is_key_down(i8 key) { return keyDown[(u8)key]; }

b8 cplt_is_key_pressed(i8 key) { return keyPressed[(u8)key]; }

b8 cplt_is_key_released(i8 key) { return keyReleased[(u8)key]; }

// }}}

i32 cplt_rgb_to_i32(rgb color) {
    return (color.r << 16) | (color.g << 8) | color.b;
}

// {{{ Drawing

void cplt_render() {
    if (frame_sync) {
        write(STDOUT_FILENO, "\x1b[?2026h", 8);
    }

    i32 lineBufSize = (width * 22) + 64;
    i8 *line = malloc((size_t)lineBufSize);
    if (!line) {
        return;
    }

    i32 totalSize = ((lineBufSize + 8) * height) + 64;
    i8 *out = malloc((size_t)totalSize);
    if (!out) {
        free(line);
        return;
    }
    i32 outPos = 0;

    outPos += snprintf(out + outPos, (size_t)(totalSize - outPos), "\x1b[H");

    for (i32 y = 0; y < height; y++) {
        i32 pos = 0;

        pos += snprintf(line + pos, (size_t)(lineBufSize - pos), "%s", bg_col);
        i32 lastColor = -1;

        for (i32 x = 0; x < width; x++) {
            i32 idx = (y * width) + x;
            i32 packedColor = col_buf[idx];

            if (packedColor != lastColor) {
                i32 r = (packedColor >> 16) & 0xFF;
                i32 g = (packedColor >> 8) & 0xFF;
                i32 b = packedColor & 0xFF;
                pos += snprintf(line + pos, (size_t)(lineBufSize - pos),
                                "\x1b[38;2;%d;%d;%dm", r, g, b);
                lastColor = packedColor;
            }

            i8 c = screen_buf[idx];
            if (c < 32 || c == 127) {
                c = ' ';
            }
            line[pos++] = c;
        }

        pos +=
            snprintf(line + pos, (size_t)(lineBufSize - pos), CPLT_DEF_COLOR);

        if (y < height - 1) {
            line[pos++] = '\r';
            line[pos++] = '\n';
        }

        memcpy(out + outPos, line, (size_t)pos);
        outPos += pos;
    }

    write(STDOUT_FILENO, out, (size_t)outPos);
    free(line);
    free(out);

    if (frame_sync) {
        write(STDOUT_FILENO, "\x1b[?2026l", 8);
    }
}

void cplt_clear(i8 c, rgb color) {
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            screen_buf[(y * width) + x] = c;
            col_buf[(y * width) + x] = cplt_rgb_to_i32(color);
        }
    }
}

void cplt_clear_bg(rgb color) {
    color.r = color.r < 0 ? 0 : color.r % 256;
    color.g = color.g < 0 ? 0 : color.g % 256;
    color.b = color.b < 0 ? 0 : color.b % 256;
    snprintf(bg_col, sizeof(bg_col), "\x1b[48;2;%d;%d;%dm", color.r, color.g,
             color.b);
}

void cplt_draw_pixel(i32 x, i32 y, const i8 *c, rgb color) {
    if (x < width && x >= 0 && y < height && y >= 0) {
        screen_buf[(y * width) + x] = c[0];
        col_buf[(y * width) + x] = cplt_rgb_to_i32(color);
    }
}

void cplt_draw_text(i32 x, i32 y, i8 *text, rgb color) {
    if (y < 0 || y >= height) {
        return;
    }
    i32 len = (i32)strlen(text);
    if (x >= width || x + len <= 0) {
        return;
    }

    i32 start = 0;
    if (x < 0) {
        start = -x;
        x = 0;
    }
    i32 end = len;
    if (x + (end - start) > width) {
        end = start + (width - x);
    }

    for (i32 i = start; i < end; i++) {
        cplt_draw_pixel(x + (i - start), y, &text[i], color);
    }
}

void cplt_draw_rect(i32 x, i32 y, i32 w, i32 h, i8 *c, rgb color) {
    for (i32 iy = y; iy < h + y; iy++) {
        if (iy >= height || iy < 0) {
            continue;
        }
        for (i32 ix = x; ix < w + x; ix++) {
            cplt_draw_pixel(ix, iy, c, color);
        }
    }
}

void cplt_draw_circle(i32 cx, i32 cy, i32 r, i8 *c, rgb color) {
    i32 x = 0;
    i32 y = r;
    i32 d = 1 - r;

    while (x <= y) {
        for (i32 i = cx - x; i <= cx + x; i++) {
            cplt_draw_pixel(i, cy + y, c, color);
            cplt_draw_pixel(i, cy - y, c, color);
        }
        for (i32 i = cx - y; i <= cx + y; i++) {
            cplt_draw_pixel(i, cy + x, c, color);
            cplt_draw_pixel(i, cy - x, c, color);
        }
        x++;
        if (d < 0) {
            d += (2 * x) + 1;
        } else {
            y--;
            d += (2 * (x - y)) + 1;
        }
    }
}

void cplt_draw_circle_out(i32 cx, i32 cy, i32 r, i8 *c, rgb color) {
    i32 x = 0;
    i32 y = r;
    i32 d = 1 - r;

    while (x <= y) {
        cplt_draw_pixel(cx + x, cy + y, c, color);
        cplt_draw_pixel(cx + y, cy + x, c, color);
        cplt_draw_pixel(cx - x, cy + y, c, color);
        cplt_draw_pixel(cx - y, cy + x, c, color);
        cplt_draw_pixel(cx + x, cy - y, c, color);
        cplt_draw_pixel(cx + y, cy - x, c, color);
        cplt_draw_pixel(cx - x, cy - y, c, color);
        cplt_draw_pixel(cx - y, cy - x, c, color);
        x++;
        if (d < 0) {
            d += (2 * x) + 1;
        } else {
            y--;
            d += (2 * (x - y)) + 1;
        }
    }
}

// }}}

#endif
