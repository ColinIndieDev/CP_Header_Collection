#define CPL_IMPLEMENTATION
#include "../cplibrary/cpl.h"
#include "../cpstd/cprng.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define SHOW_DETAILS true

// {{{ Start animation

f32 alpha_logo = 255.0f;
f32 alpha_bg = 255.0f;

b8 fading = true;
texture logo;

void start_anim_init() {
    cpl_load_texture(&logo, "assets/images/logo.png", CPL_FILTER_LINEAR);
}

void start_anim_update() {
    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);
    cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                  &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                  &(vec4f){0.0f, 0.0f, 0.0f, alpha_bg}, 0.0f);

    f32 logo_size = 500.0f;

    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, false);
    vec2f logo_pos = {(cpl_get_screen_width() / 2.0f) - (logo_size / 2.0f),
                      (cpl_get_screen_height() / 2.0f) - (logo_size / 2.0f)};
    cpl_draw_texture2D(&logo, &logo_pos, &(vec2f){logo_size, logo_size},
                       &(vec4f){255.0f, 255.0f, 255.0f, alpha_logo}, 0.0f);

    if (alpha_logo > 0) {
        alpha_logo -= (75.f * cpl_get_dt());
    }
    if (alpha_logo <= 0 && alpha_bg > 0) {
        alpha_bg -= (125.f * cpl_get_dt());
    }

    if (alpha_bg <= 0 && alpha_logo <= 0 && fading) {
        fading = false;
    }
}

// }}}

// {{{ Data

typedef struct {
    vec2f pos;
    f32 vel;
    f32 radius;
    f32 gravity;
    f32 jmp_force;
    f32 rot;
    b8 falling;
} bird;

typedef struct {
    vec2f pos;
    vec2f size;
    b8 active;
} green_pipe;

VEC_DEF(green_pipe, green_pipes)
VEC_DEF(vec2f, positions)

typedef struct {
    positions sky_positions;
    positions building_positions;
    positions forest_positions;
} bg;

texture sky_tex;
texture building_tex;
texture forest_tex;
texture bird_tex;
texture pipe_tex;
font default_font;

audio damage_sound;
audio fly_sound;
audio restart_sound;
audio music;

b8 game_over = false;
b8 restarting = false;
b8 spawn_pipes = false;

f32 score = 0.0f;
f32 highscore = 0.0f;
f32 alpha = 0.0f;

f32 pipe_clock = 0.0f;
f32 restart_clock = 0.0f;
f32 pipe_cooldown = 4.0f;

f32 gap = 200.0f;

green_pipes pipes;
bird player = {.vel = 100.0f,
               .radius = 40.0f,
               .rot = 0.0f,
               .falling = false,
               .gravity = 200.0f,
               .jmp_force = 150.0f};
bg background;

f32 tex_width = 800.0f * 1.5f;
f32 tex_height = 600.0f * 1.5f;

// }}}

// {{{ Bird

void bird_update(bird *b) {

    b->rot = 45.0f * b->vel / b->jmp_force;
    if (b->rot > 45.0f) {
        b->rot = 45.0f;
    }

    b->vel += b->gravity * cpl_get_dt();
    b->pos.y += b->vel * cpl_get_dt();
}

void bird_game_over_anim(bird *b) {
    if (!(b->pos.y - (100.0f / 4.0f) >= cpl_get_screen_height())) {
        b->vel += b->gravity * cpl_get_dt() * 5.0f;
        b->pos.y += b->vel * cpl_get_dt();
    }
}

// }}}

// {{{ Pipe

void green_pipe_update(green_pipe *p) {
    p->pos.x -= 100.0f * cpl_get_dt();
    if (p->active && p->pos.x + p->size.x < 0) {
        p->active = false;
    }
}

void spawn_pipe() {
    green_pipes_push_back(
        &pipes,
        (green_pipe){
            .pos = {cpl_get_screen_width(),
                    cpl_get_screen_height() - cprng_randf_range(0.0f, 350.0f)},
            .size = {125.0f, 850.0f},
            .active = true,
        });
}

void remove_pipe() {
    u32 w = 0;
    for (u32 r = 0; r < pipes.size; r++) {
        if (pipes.data[r].active) {
            pipes.data[w++] = pipes.data[r];
        }
    }
    pipes.size = w;
}

// }}}

// {{{ Background

void bg_init(bg *b) {
    positions_reserve(&b->sky_positions, 3);
    positions_reserve(&b->building_positions, 3);
    positions_reserve(&b->forest_positions, 3);

    positions_push_back(&b->sky_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->sky_positions, (vec2f){tex_width, 0.0f});
    positions_push_back(&b->sky_positions, (vec2f){tex_width * 2.0f, 0.0f});
    positions_push_back(&b->building_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->building_positions, (vec2f){tex_width, 0.0f});
    positions_push_back(&b->building_positions,
                        (vec2f){tex_width * 2.0f, 0.0f});
    positions_push_back(&b->forest_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->forest_positions, (vec2f){tex_width, 0.0f});
    positions_push_back(&b->forest_positions, (vec2f){tex_width * 2.0f, 0.0f});
}
void bg_update(bg *b) {
    FOREACH_VEC(vec2f, positions, pos, &b->sky_positions) {
        pos->x -= 15.0f * cpl_get_dt();
    }
    FOREACH_VEC(vec2f, positions, pos, &b->building_positions) {
        pos->x -= 25.0f * cpl_get_dt();
    }
    FOREACH_VEC(vec2f, positions, pos, &b->forest_positions) {
        pos->x -= 35.0f * cpl_get_dt();
    }

    if (!positions_empty(&b->sky_positions) &&
        positions_front(&b->sky_positions)->x + tex_width <= 0) {
        f32 right_x = positions_back(&b->sky_positions)->x;
        positions_pop_front(&b->sky_positions);
        positions_push_back(&b->sky_positions,
                            (vec2f){right_x + tex_width, 0.0f});
    }
    if (!positions_empty(&b->building_positions) &&
        positions_front(&b->building_positions)->x + tex_width <= 0) {
        f32 right_x = positions_back(&b->building_positions)->x;
        positions_pop_front(&b->building_positions);
        positions_push_back(&b->building_positions,
                            (vec2f){right_x + tex_width, 0.0f});
    }
    if (!positions_empty(&b->forest_positions) &&
        positions_front(&b->forest_positions)->x + tex_width <= 0) {
        f32 right_x = positions_back(&b->forest_positions)->x;
        positions_pop_front(&b->forest_positions);
        positions_push_back(&b->forest_positions,
                            (vec2f){right_x + tex_width, 0.0f});
    }
}

// }}}

// {{{ UI etc.

void restart_anim() {
    if (alpha < 255.0f) {
        alpha += 255.0f * cpl_get_dt() / 2.0f;
    }
}

void ui_menu() {
    {
        char text[] = "Flappy Bird";
        f32 text_width = cpl_get_text_size(&default_font, text, 2.0f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     48.0f * 2.0f},
            2.0f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char text[] = "[ Click to start ]";
        f32 text_width = cpl_get_text_size(&default_font, text, 0.5f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     cpl_get_screen_height() / 1.5f},
            0.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char text[50];
        snprintf(text, 50, "Highscore: %d", (i32)highscore);
        f32 text_width = cpl_get_text_size(&default_font, text, 0.75f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     48.0f * 5.0f},
            0.75f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }
}

void ui_game_over() {
    {
        char text[] = "Game Over";
        f32 text_width = cpl_get_text_size(&default_font, text, 2.0f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     48.0f * 2.0f},
            2.0f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char text[] = "[ Click to restart ]";
        f32 text_width = cpl_get_text_size(&default_font, text, 0.5f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     cpl_get_screen_height() / 1.5f},
            0.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char text[50];
        snprintf(text, 50, "Highscore: %d", (i32)highscore);
        f32 text_width = cpl_get_text_size(&default_font, text, 0.75f).x;
        cpl_draw_text_shadow(
            &default_font, text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f),
                     48.0f * 5.0f},
            0.75f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }
}

// }}}

// {{{ Highscore

void update_highscore() {
    if (score > highscore) {
        highscore = score;
    }
}
void read_highscore() {
    FILE *in = fopen("highscore.bin", "rb");
    if (!in) {
        return;
    }
    fread(&highscore, sizeof(f32), 1, in);
    fclose(in);
}
void save_highscore() {
    FILE *out = fopen("highscore.bin", "wb");
    fwrite(&highscore, sizeof(f32), 1, out);
    fclose(out);
}

// }}}

void handle_input();
void handle_collisions();
void game_init();
void game_update();
void game_render();

void main_loop() {
    cpl_web_window_resize();
    cpl_update();
    if (!fading) {
        handle_input();
    }
    handle_collisions();
    game_update();
    game_render();
}

int main() {
    cpl_init_window(800, 600, "Flappy Bird Clone");
    cpl_enable_vsync(false);
    cpl_audio_init();
    game_init();
    start_anim_init();
    cpl_audio_play_music(&music);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (!cpl_window_should_close()) {
        main_loop();
    }
    save_highscore();
#endif
    cpl_audio_close();
    cpl_close_window();
}

// {{{ Init

void game_init() {
    pipe_clock = cpl_get_time();
#ifndef __EMSCRIPTEN__
    read_highscore();
#endif
    green_pipes_reserve(&pipes, 100);
    player.pos =
        (vec2f){cpl_get_screen_width() / 2.0f, cpl_get_screen_height() / 2.0f};
    bg_init(&background);
    cpl_load_texture(&sky_tex, "assets/images/sky.png", CPL_FILTER_LINEAR);
    cpl_load_texture(&building_tex, "assets/images/buildings.png",
                     CPL_FILTER_LINEAR);
    cpl_load_texture(&forest_tex, "assets/images/woods.png", CPL_FILTER_LINEAR);
    cpl_load_texture(&bird_tex, "assets/images/bird.png", CPL_FILTER_LINEAR);
    cpl_load_texture(&pipe_tex, "assets/images/pipe.png", CPL_FILTER_LINEAR);
    cpl_create_font(&default_font, "assets/fonts/default.ttf", "default",
                    CPL_FILTER_NEAREST);

    damage_sound = cpl_load_audio("assets/sounds/damage.wav");
    fly_sound = cpl_load_audio("assets/sounds/jump.mp3");
    restart_sound = cpl_load_audio("assets/sounds/restart.mp3");
    music = cpl_load_audio("assets/sounds/music.mp3");
}

// }}}

// {{{ Update

void game_update() {
    update_highscore();

    player.pos.x = cpl_get_screen_width() / 2.0f;
    if (!player.falling) {
        player.pos.y = cpl_get_screen_height() / 2.0f;
    }

    if (game_over) {
        bird_game_over_anim(&player);
        cpl_audio_stop_music();
    } else {
        bg_update(&background);
        cpl_audio_resume_music();
    }
    if (player.falling && !game_over) {
        bird_update(&player);
        if (pipe_clock + pipe_cooldown < cpl_get_time()) {
            spawn_pipe();
            pipe_clock = cpl_get_time();
        }
    }
    remove_pipe();
}

void handle_collisions() {
    circle_collider player_collider = {.pos = player.pos,
                                       .radius = player.radius};

    FOREACH_VEC(green_pipe, green_pipes, p, &pipes) {
        if (!game_over) {
            green_pipe_update(p);
        }
        rect_collider lower_pipe_collider = {.pos = p->pos, .size = p->size};
        rect_collider upper_pipe_collider = {
            .pos = (vec2f){p->pos.x, p->pos.y - p->size.y - gap},
            .size = p->size};

        if (cpl_check_collision_circle_rect(&player_collider,
                                            &upper_pipe_collider) ||
            cpl_check_collision_circle_rect(&player_collider,
                                            &lower_pipe_collider)) {
            if (!game_over) {
                cpl_audio_play_sound(&damage_sound);
            }
            game_over = true;
        }
    }

    if (player.pos.y <= 0.0f) {
        player.vel = 0.0f;
        player.pos.y = 0.0f;
    } else if (!game_over && player.pos.y - (bird_tex.size.y / 4.0f) >=
                                 cpl_get_screen_height()) {
        cpl_audio_play_sound(&damage_sound);
        game_over = true;
    }
}

void handle_input() {
    if (cpl_is_key_down(CPL_KEY_ESCAPE)) {
        cpl_destroy_window();
    }
    if (cpl_is_mouse_pressed(CPL_MOUSE_BUTTON_LEFT) && !game_over) {
        if (!player.falling) {
            player.falling = true;
        }
        player.vel = -player.jmp_force;
        cpl_audio_play_sound(&fly_sound);
    }
    if (cpl_is_mouse_down(CPL_MOUSE_BUTTON_LEFT) && game_over && !restarting) {
        cpl_audio_play_sound(&restart_sound);
        if (!restarting) {
            restarting = true;
            alpha = 0.0f;
            restart_clock = cpl_get_time();
        }
    }
}

// }}}

// {{{ Rendering

void game_render() {
    cpl_clear_background(&(vec4f){132.0f, 226.0f, 138.0f, 255.0f});
    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, false);

    vec2f tex_size = {.x = tex_width, .y = tex_height};
    FOREACH_VEC(vec2f, positions, p, &background.sky_positions) {
        cpl_draw_texture2D(&sky_tex, p, &tex_size, &WHITE, 0.0f);
    }
    FOREACH_VEC(vec2f, positions, p, &background.building_positions) {
        cpl_draw_texture2D(&building_tex, p, &tex_size, &WHITE, 0.0f);
    }
    FOREACH_VEC(vec2f, positions, p, &background.forest_positions) {
        cpl_draw_texture2D(&forest_tex, p, &tex_size, &WHITE, 0.0f);
    }

    FOREACH_VEC(green_pipe, green_pipes, p, &pipes) {
        cpl_draw_texture2D(&pipe_tex,
                           &(vec2f){p->pos.x, p->pos.y - p->size.y - gap},
                           &p->size, &WHITE, 180.0f);
        cpl_draw_texture2D(&pipe_tex, &(vec2f){p->pos.x, p->pos.y}, &p->size,
                           &WHITE, 0.0f);
    }

    vec2f off = {player.radius * 2.25f, player.radius * 2.25f};

    cpl_draw_texture2D(&bird_tex,
                       &(vec2f){player.pos.x - off.x, player.pos.y - off.y},
                       &(vec2f){player.radius * 4.5f, player.radius * 4.5f},
                       &WHITE, player.rot);

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);

    if (game_over && !restarting) {
        cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                      &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                      &(vec4f){0.0f, 0.0f, 0.0f, 150.0f}, 0.0f);
    }

    cpl_begin_draw(CPL_TEXT, false);
    if (!game_over && player.falling) {
        score += 1 * cpl_get_dt();
    }
    char score_text[16];
    snprintf(score_text, 16, "%d", (i32)score);
    f32 text_width = cpl_get_text_size(&default_font, score_text, 1.5f).x;

    if (player.falling) {
        cpl_draw_text_shadow(
            &default_font, score_text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f), 10},
            1.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    } else {
        ui_menu();
    }
    if (game_over && !restarting) {
        ui_game_over();
    }
    start_anim_update();

#if SHOW_DETAILS
    cpl_display_details(&default_font);
#endif

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);

    if (restarting) {
        restart_anim();
        cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                      &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                      &(vec4f){255.0f, 255.0f, 255.0f, alpha}, 0.0f);

        if (restart_clock + 2.0f < cpl_get_time()) {
            player.pos = (vec2f){cpl_get_screen_width() / 2.0f,
                                 cpl_get_screen_height() / 2.0f};
            player.vel = 0.0f;
            score = 0;
            pipes.size = 0;
            game_over = false;
            restarting = false;
        }
    }

    cpl_end_frame();
}

// }}}
