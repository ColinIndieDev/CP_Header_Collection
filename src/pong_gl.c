#include "../cplibrary/cpl.h"
#include "../cpstd/cparena.h"
#include "../cpstd/cprng.h"

#include <stdio.h>

typedef struct {
    vec2f p;
    vec2f s;
    u32 score;
} player;

typedef struct {
    vec2f p;
    f32 r;
    vec2f dir;
} ball;

void reset_ball(ball *pong, f32 *speed, f32 start_speed);
void update(player *p1, player *p2, ball *pong, f32 player_width,
            f32 player_height, f32 off, f32 *ball_speed, f32 ball_speed_start);
void render(player p1, player p2, ball pong, font *font, texture *tex,
            mem_arena *arena);

int main() {
    cpl_init_window(800, 600, "Hello CPL");
    cpl_enable_vsync(false);

    cprng_rand_seed();

    f32 off = 10.0f;

    f32 player_width = 15.0f;
    f32 player_height = 150.0f;

    player p1 = {
        .p = {off, (cpl_get_screen_height() / 2.0f) - (player_height / 2.0f)},
        .s = {player_width, player_height},
        .score = 0};
    player p2 = {
        .p = {cpl_get_screen_width() - player_width - off,
              (cpl_get_screen_height() / 2.0f) - (player_height / 2.0f)},
        .s = {player_width, player_height},
        .score = 0};
    ball pong = {
        .p = {cpl_get_screen_width() / 2.0f, cpl_get_screen_height() / 2.0f},
        .r = 20.0f,
        .dir = {cprng_rand() % 2 == 0 ? -1.0f : 1.0f,
                cprng_rand() % 2 == 0 ? -1.0f : 1.0f}};

    f32 last_reset_time = 0.0f;

    font default_font;
    cpl_create_font(&default_font, "fonts/default.ttf", "default",
                    CPL_FILTER_NEAREST);

    f32 ball_speed_start = 250.0f;
    f32 ball_speed = ball_speed_start;

    texture logo;
    cpl_load_texture(&logo, "assets/logo.png", CPL_FILTER_LINEAR);

    mem_arena *arena = mem_arena_create(KiB(1));

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!cpl_window_should_close()) {
        cpl_update();

        update(&p1, &p2, &pong, player_width, player_height, off, &ball_speed,
               ball_speed_start);

        render(p1, p2, pong, &default_font, &logo, arena);
    }
    mem_arena_destroy(arena);
    cpl_delete_font(&default_font);
    cpl_unload_texture(&logo);
    cpl_close_window();
}

void update(player *p1, player *p2, ball *pong, f32 player_width,
            f32 player_height, f32 off, f32 *ball_speed, f32 ball_speed_start) {
    p2->p.x = cpl_get_screen_width() - player_width - off;

    f32 speed = 1000.0f;
    if (cpl_is_key_down(CPL_KEY_W)) {
        p1->p.y -= speed * cpl_get_dt();
    }
    if (cpl_is_key_down(CPL_KEY_S)) {
        p1->p.y += speed * cpl_get_dt();
    }
    if (cpl_is_key_down(CPL_KEY_H)) {
        p2->p.y -= speed * cpl_get_dt();
    }
    if (cpl_is_key_down(CPL_KEY_N)) {
        p2->p.y += speed * cpl_get_dt();
    }

    p1->p.y = CPM_CLAMP(p1->p.y, 0, cpl_get_screen_height() - player_height);
    p2->p.y = CPM_CLAMP(p2->p.y, 0, cpl_get_screen_height() - player_height);

    f32 ball_acceleration = 10.0f;
    *ball_speed += ball_acceleration * cpl_get_dt();

    pong->p = vec2f_add(&pong->p,
                        &(vec2f){pong->dir.x * (*ball_speed) * cpl_get_dt(),
                                 pong->dir.y * (*ball_speed) * cpl_get_dt()});

    if (pong->p.y - pong->r <= 0.0f) {
        pong->p.y = pong->r;
        pong->dir.y *= -1;
    } else if (pong->p.y + pong->r >= cpl_get_screen_height()) {
        pong->p.y = cpl_get_screen_height() - pong->r;
        pong->dir.y *= -1;
    }

    circle_collider ball_collider = {.pos = pong->p, .radius = pong->r};
    rect_collider p1_collider = {.pos = p1->p, .size = p1->s};
    rect_collider p2_collider = {.pos = p2->p, .size = p2->p};

    if (cpl_check_collision_circle_rect(&ball_collider, &p1_collider)) {
        pong->p.x = p1->p.x + p1->s.x + pong->r;
        pong->dir.x *= -1;
    } else if (cpl_check_collision_circle_rect(&ball_collider, &p2_collider)) {
        pong->p.x = p2->p.x - pong->r;
        pong->dir.x *= -1;
    }

    if (pong->p.x <= 0.0f) {
        p2->score++;
        reset_ball(pong, ball_speed, ball_speed_start);
    } else if (pong->p.x >= cpl_get_screen_width()) {
        p1->score++;
        reset_ball(pong, ball_speed, ball_speed_start);
    }
}

void render(player p1, player p2, ball pong, font *font, texture *tex,
            mem_arena *arena) {
    cpl_clear_background(&BLACK);

    cpl_begin_draw(CPL_SHAPE_2D_LIT, false);

    cpl_set_ambient_light_2D(0.1f);

    point_light_2D *point_l = malloc(sizeof(point_light_2D) * 1);
    point_l[0] = (point_light_2D){
        .pos = pong.p, .radius = 100.0f, .intensity = 1.0f, .color = WHITE};

    cpl_add_point_lights_2D(point_l, 1);

    // Redundant because not affected by light 2D
    cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                  &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                  &BLACK, 0.0f);

    cpl_draw_line(
        &(vec2f){cpl_get_screen_width() / 2.0f, 0.0f},
        &(vec2f){cpl_get_screen_width() / 2.0f, cpl_get_screen_height()}, 3.0f,
        &WHITE);

    cpl_draw_rect(&p1.p, &p1.s, &BLUE, 0.0f);
    cpl_draw_rect(&p2.p, &p2.s, &RED, 0.0f);

    cpl_draw_circle(&pong.p, pong.r, &WHITE);

    cpl_begin_draw(CPL_TEXT, false);

    f32 score_font_size = 1.0f;

    char *p1_score = mem_arena_push(arena, 10, true);
    snprintf(p1_score, 10, "%d", p1.score);
    vec2f p1_score_text_size =
        cpl_get_text_size(font, p1_score, score_font_size);
    cpl_draw_text(
        font, p1_score,
        &(vec2f){(cpl_get_screen_width() * 0.25f) - p1_score_text_size.x,
                 cpl_get_screen_height() - p1_score_text_size.y - 10.0f},
        score_font_size, &WHITE);

    char *p2_score = mem_arena_push(arena, 10, true);
    snprintf(p2_score, 10, "%d", p2.score);
    vec2f p2_score_text_size =
        cpl_get_text_size(font, p2_score, score_font_size);
    cpl_draw_text(
        font, p2_score,
        &(vec2f){(cpl_get_screen_width() * 0.75f) - p2_score_text_size.x,
                 cpl_get_screen_height() - p2_score_text_size.y - 10.0f},
        score_font_size, &WHITE);

    mem_arena_clear(arena);

    cpl_display_details(font);

    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, false);

    cpl_draw_texture2D(tex, &(vec2f){0.0f, 0.0f},
                       &(vec2f){tex->size.x / 10.0f, tex->size.y / 10.0f},
                       &WHITE, 0.0f);

    cpl_end_frame();
}

void reset_ball(ball *pong, f32 *speed, f32 start_speed) {
    pong->p.x = cpl_get_screen_width() / 2.0f;
    pong->p.y = cpl_get_screen_height() / 2.0f;
    pong->dir.x = cprng_rand() % 2 == 0 ? -1.0f : 1.0f;
    pong->dir.y = cprng_rand() % 2 == 0 ? -1.0f : 1.0f;
    *speed = start_speed;
}
