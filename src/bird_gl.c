#include "../cplibrary/cpl.h"
#include "../cpstd/cprng.h"
#include "bird_anim.h"
#include <stdio.h>

// {{{ Bird

typedef struct {
    vec2f pos;
    f32 vel;
    f32 gravity;
    f32 jmp_force;
    f32 rot;
    b8 falling;
} bird;

void bird_update(bird *b) {
    /*
    b->rot = 45.0f * b->vel / b->jmp_force;
    if (b->rot > 45.0f) {
        b->rot = 45.0f;
    }
    */
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

typedef struct {
    vec2f pos;
    vec2f size;
    b8 active;
} green_pipe;

void green_pipe_update(green_pipe *p) {
    p->pos.x -= 100.0f * cpl_get_dt();
    if (p->active && p->pos.x + p->size.x < 0) {
        p->active = false;
    }
}

VEC_DEF(green_pipe, green_pipes)

green_pipes pipes;

// }}}

// {{{ Background

VEC_DEF(vec2f, positions)

typedef struct {
    positions sky_positions;
    positions building_positions;
    positions forest_positions;
} bg;

void bg_init(bg *b) {
    positions_reserve(&b->sky_positions, 2);
    positions_reserve(&b->building_positions, 2);
    positions_reserve(&b->forest_positions, 2);

    positions_push_back(&b->sky_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->sky_positions,
                        (vec2f){cpl_get_screen_width(), 0.0f});
    positions_push_back(&b->building_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->building_positions,
                        (vec2f){cpl_get_screen_width(), 0.0f});
    positions_push_back(&b->forest_positions, (vec2f){0.0f, 0.0f});
    positions_push_back(&b->forest_positions,
                        (vec2f){cpl_get_screen_width(), 0.0f});
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
        positions_front(&b->sky_positions)->x + cpl_get_screen_width() <= 0) {
        f32 right_x = positions_back(&b->sky_positions)->x;
        positions_pop_front(&b->sky_positions);
        positions_push_back(&b->sky_positions,
                            (vec2f){right_x + cpl_get_screen_width(), 0.0f});
    }
    if (!positions_empty(&b->building_positions) &&
        positions_front(&b->building_positions)->x + cpl_get_screen_width() <=
            0) {
        f32 right_x = positions_back(&b->building_positions)->x;
        positions_pop_front(&b->building_positions);
        positions_push_back(&b->building_positions,
                            (vec2f){right_x + cpl_get_screen_width(), 0.0f});
    }
    if (!positions_empty(&b->forest_positions) &&
        positions_front(&b->forest_positions)->x + cpl_get_screen_width() <=
            0) {
        f32 right_x = positions_back(&b->forest_positions)->x;
        positions_pop_front(&b->forest_positions);
        positions_push_back(&b->forest_positions,
                            (vec2f){right_x + cpl_get_screen_width(), 0.0f});
    }
}

// }}}

// {{{ UI etc.

f32 alpha = 0.0f;
void restart_anim() {
    if (alpha < 255.0f) {
        alpha += 255.0f * cpl_get_dt() / 2.0f;
    }
}

b8 game_over = false;
b8 restarting = false;
f32 score = 0;
f32 highscore = 0;

font *default_font;

void ui_menu() {
    {
        char *text = "Flappy Bird";
        f32 text_width = cpl_get_text_size(default_font, text, 2.0f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     48.0f * 2.0f},
            2.0f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char *text = "[ Click to start ]";
        f32 text_width = cpl_get_text_size(default_font, text, 0.5f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     cpl_get_screen_height() / 1.5f},
            0.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char *text = malloc(50);
        snprintf(text, 50, "Highscore: %d", (i32)highscore);
        f32 text_width = cpl_get_text_size(default_font, text, 0.75f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     48.0f * 5.0f},
            0.75f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }
}

void ui_game_over() {
    {
        char *text = "Game Over";
        f32 text_width = cpl_get_text_size(default_font, text, 2.0f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     48.0f * 2.0f},
            2.0f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char *text = "[ Click to restart ]";
        f32 text_width = cpl_get_text_size(default_font, text, 0.5f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     cpl_get_screen_height() / 1.5f},
            0.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }

    {
        char *text = malloc(50);
        snprintf(text, 50, "Highscore: %d", (i32)highscore);
        f32 text_width = cpl_get_text_size(default_font, text, 0.75f).x;
        cpl_draw_text_shadow(
            default_font, text,
            &(vec2f){cpl_get_screen_width() / 2.0f - text_width / 2.0f,
                     48.0f * 5.0f},
            0.75f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    }
}

// }}}

void handle_input(
    bird *p /*, const Audio &flySound, const Audio &restartSound*/);

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

void spawn_pipe() {
    green_pipes_push_back(
        &pipes,
        (green_pipe){
            .pos = {cpl_get_screen_width(),
                    cpl_get_screen_height() - cprng_randf_range(0.0f, 350.0f)},
            .size = {100.0f, 500.0f},
            .active = true,
        });
}

b8 spawn_pipes = false;

void remove_pipe() {
    u32 w = 0;
    for (u32 r = 0; r < pipes.size; r++) {
        if (pipes.data[r].active) {
            pipes.data[w++] = pipes.data[r];
        }
    }
    pipes.size = w;
}

texture *sky_tex;
texture *building_tex;
texture *forest_tex;
texture *bird_tex;
texture *pipe_tex;

bird *player;
bg *background;

/*
Audio damageSound;
Audio flySound;
Audio restartSound;
Audio music;
*/

f32 pipe_clock = 0.0f;
f32 restart_clock = 0.0f;
f32 pipe_cooldown = 3.0f;

void game_loop();

int main() {
    cpl_init_window(800, 600, "Flappy Bird Clone");
    pipe_clock = cpl_get_time();

    read_highscore();

    player = malloc(sizeof(bird));
    background = malloc(sizeof(bg));

    green_pipes_reserve(&pipes, 100);

    player->vel = 100.0f;
    player->rot = 0.0f;
    player->falling = false;
    player->gravity = 200.0f;
    player->jmp_force = 150.0f;
    player->pos =
        (vec2f){cpl_get_screen_width() / 2.0f, cpl_get_screen_height() / 2.0f};

    cpl_cam_2D.pos =
        (vec2f){cpl_get_screen_width() / 2.0f, cpl_get_screen_height() / 2.0f};
    bg_init(background);

    sky_tex = malloc(sizeof(texture));
    building_tex = malloc(sizeof(texture));
    forest_tex = malloc(sizeof(texture));
    bird_tex = malloc(sizeof(texture));
    pipe_tex = malloc(sizeof(texture));

    default_font = malloc(sizeof(font));

    cpl_load_texture(sky_tex, "assets/images/sky.png", CPL_FILTER_LINEAR);
    cpl_load_texture(building_tex, "assets/images/buildings.png",
                     CPL_FILTER_LINEAR);
    cpl_load_texture(forest_tex, "assets/images/woods.png", CPL_FILTER_LINEAR);
    cpl_load_texture(bird_tex, "assets/images/bird.png", CPL_FILTER_LINEAR);
    cpl_load_texture(pipe_tex, "assets/images/pipe.png", CPL_FILTER_LINEAR);

    cpl_create_font(default_font, "assets/fonts/default.ttf", "default",
                    CPL_FILTER_NEAREST);
    /*
    damageSound = AudioManager::LoadAudio("assets/sounds/damage.wav");
    damageSound = AudioManager::LoadAudio("assets/sounds/damage.wav");
    flySound = AudioManager::LoadAudio("assets/sounds/jump.mp3");
    restartSound = AudioManager::LoadAudio("assets/sounds/restart.mp3");
    music = AudioManager::LoadAudio("assets/sounds/music.mp3");
    */

    start_anim_init();
    // AudioManager::PlayMusic(music);

    while (!cpl_window_should_close()) {
        game_loop();
    }
    save_highscore();

    cpl_close_window();
}

void game_loop() {
    cpl_update();
    cpl_update_input();

    if (!fading) {
        handle_input(player /*, flySound, restartSound*/);
    }

    update_highscore();

    if (!game_over) {
        bg_update(background);
    }
    if (player->falling && !game_over) {
        bird_update(player);
    }
    if (game_over) {
        bird_game_over_anim(player);
    }

    /*
    if (game_over)
        AudioManager::StopMusic();
    else
        AudioManager::ResumeMusic();
    */

    if (player->falling && !game_over) {
        if (pipe_clock + pipe_cooldown < cpl_get_time()) {
            spawn_pipe();
            pipe_clock = cpl_get_time();
        }
    }
    remove_pipe();

    circle_collider player_collider = {
        .pos = player->pos, .radius = 15.0f};

    FOREACH_VEC(green_pipe, green_pipes, p, &pipes) {
        if (!game_over) {
            green_pipe_update(p);
        }
        /*
        rect_collider upper_pipe_collider = {.pos = p->pos, .size = p->size};
        rect_collider lower_pipe_collider = {
            .pos = (vec2f){p->pos.x, p->pos.y - (p->size.y + 150.0f)},
            .size = p->size};
        */
        f32 gap = 150.0f;

        rect_collider lower_pipe_collider = {.pos = p->pos, .size = p->size};

        rect_collider upper_pipe_collider = {
            .pos = (vec2f){p->pos.x, p->pos.y - p->size.y - gap},
            .size = p->size};
        if (cpl_check_collision_circle_rect(&player_collider,
                                            &upper_pipe_collider) ||
            cpl_check_collision_circle_rect(&player_collider,
                                            &lower_pipe_collider)) {
            if (!game_over) {
                // AudioManager::PlaySFX(damageSound);
            }
            game_over = true;
        }
    }

    if (player->pos.y <= 0.0f) {
        player->vel = 0.0f;
        player->pos.y = 0.0f;
    } else if (!game_over && player->pos.y - bird_tex->size.y / 4.0f >=
                                 cpl_get_screen_height()) {
        // AudioManager::PlaySFX(damageSound);
        game_over = true;
    }

    cpl_clear_background(&BLACK);
    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, false);

    vec2f screen_size = {.x = cpl_get_screen_width(),
                         .y = cpl_get_screen_height()};
    FOREACH_VEC(vec2f, positions, p, &background->sky_positions) {
        cpl_draw_texture2D(sky_tex, p, &screen_size, &WHITE, 0.0f);
    }
    FOREACH_VEC(vec2f, positions, p, &background->building_positions) {
        cpl_draw_texture2D(building_tex, p, &screen_size, &WHITE, 0.0f);
    }
    FOREACH_VEC(vec2f, positions, p, &background->forest_positions) {
        cpl_draw_texture2D(forest_tex, p, &screen_size, &WHITE, 0.0f);
    }

    FOREACH_VEC(green_pipe, green_pipes, p, &pipes) {
        /*
        cpl_draw_texture2D(pipe_tex,
                           &(vec2f){p->pos.x, p->pos.y - (p->size.y + 145.0f)},
                           &(vec2f){100.0f, 500.0f}, &WHITE, 180.0f);

        cpl_draw_texture2D(pipe_tex, &p->pos, &(vec2f){100.0f, 500.0f}, &WHITE,
                           0.0f);
        */
        f32 gap = 150.0f;
        cpl_draw_texture2D(pipe_tex,
                           &(vec2f){p->pos.x, p->pos.y - p->size.y - gap},
                           &(vec2f){100.0f, 500.0f}, &WHITE, 0.0f);
        cpl_draw_texture2D(pipe_tex, &(vec2f){p->pos.x, p->pos.y},
                           &(vec2f){100.0f, 500.0f}, &WHITE, 0.0f);
    }

    vec2f off = {bird_tex->size.x / 2.0f, bird_tex->size.y / 2.0f};

    cpl_draw_texture2D(bird_tex,
                       &(vec2f){player->pos.x - off.x, player->pos.y - off.y},
                       &(vec2f){100.0f, 100.0f}, &WHITE, player->rot);

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);
    if (game_over && !restarting) {
        cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                      &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                      &(vec4f){0.0f, 0.0f, 0.0f, 150.0f}, 0.0f);
    }
    if (restarting) {
        restart_anim();
        cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                      &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                      &(vec4f){0.0f, 0.0f, 0.0f, alpha}, 0.0f);

        if (restart_clock + 2.0f < cpl_get_time()) {
            player->pos = (vec2f){cpl_get_screen_width() / 2.0f,
                                  cpl_get_screen_height() / 2.0f};
            player->vel = 0.0f;
            score = 0;
            // Hope it is like .clear() in C++
            pipes.size = 0;
            game_over = false;
            restarting = false;
        }
    }

    cpl_begin_draw(CPL_TEXT, false);
    if (!game_over && player->falling) {
        score += 1 * cpl_get_dt();
    }
    char *score_text = malloc(5);
    snprintf(score_text, 5, "%d", (i32)score);
    f32 text_width = cpl_get_text_size(default_font, score_text, 1.5f).x;

    if (player->falling) {
        cpl_draw_text_shadow(
            default_font, score_text,
            &(vec2f){(cpl_get_screen_width() / 2.0f) - (text_width / 2.0f), 10},
            1.5f, &WHITE, &(vec2f){5.0f, -5.0f}, &BLACK);
    } else {
        ui_menu();
    }
    if (game_over && !restarting) {
        ui_game_over();
    }
    free(score_text);

    // ShowDetails();

    start_anim_update();

    cpl_end_frame();
}

void handle_input(bird *player/*, const Audio &flySound,
                 const Audio &restartSound*/) {
    if (cpl_is_key_down(CPL_KEY_ESCAPE)) {
        cpl_destroy_window();
    }
    if (cpl_is_mouse_down(CPL_MOUSE_BUTTON_LEFT) && !game_over) {
        if (!player->falling) {
            player->falling = true;
        }
        player->vel = -player->jmp_force;
        // AudioManager::PlaySFX(flySound);
    }
    if (cpl_is_mouse_down(CPL_MOUSE_BUTTON_LEFT) && game_over && !restarting) {
        // AudioManager::PlaySFX(restartSound);
        if (!restarting) {
            restarting = true;
            alpha = 0.0f; // Alpha for restart animation

            restart_clock = cpl_get_time();
        }
    }
}
