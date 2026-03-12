#define CPLT_IMPLEMENTATION
#include "../cplterminal/cplt.h"
#include "../cpstd/cpmath.h"
#include "../cpstd/cprng.h"
#include "../cpstd/cpvec.h"

VEC_DEF(vec2f, snake_body)

#define WINDOW_WIDTH 60
#define WINDOW_HEIGHT 20

int main() {
    cplt_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    cprng_rand_seed();

    snake_body snake;
    i32 score = 0;
    vec2f dir = {1.0f, 0.0f};

    vec2f apple = {(f32)(cprng_rand_range(1, WINDOW_WIDTH - 1)),
                   (f32)(cprng_rand_range(1, WINDOW_HEIGHT - 3))};

    snake_body_init(&snake, 3, (vec2f){cpm_floorf(WINDOW_WIDTH / 2.0f), cpm_floorf(WINDOW_HEIGHT / 2.0f)});

    snake_body_at(&snake, 1)->x += 1;
    snake_body_at(&snake, 2)->x += 2;

    f32 last_time = 0.0f;
    f32 time_dist = 0.1f;

    while (running) {
        cplt_calc_fps();
        cplt_calc_dt();
        cplt_update_input();

        if (cplt_get_time() > last_time + time_dist) {
            last_time = cplt_get_time();

            snake_body_push_back(&snake,
                                 (vec2f){snake_body_back(&snake)->x + dir.x,
                                         snake_body_back(&snake)->y + dir.y});

            if (snake_body_back(&snake)->x == apple.x &&
                snake_body_back(&snake)->y == apple.y) {
                apple = (vec2f){(f32)(cprng_rand_range(1, WINDOW_WIDTH - 1)),
                                (f32)(cprng_rand_range(1, WINDOW_HEIGHT - 4))};
                score++;
            } else {
                snake_body_pop_front(&snake);
            }

            if (snake_body_back(&snake)->x >= WINDOW_WIDTH - 1 ||
                snake_body_back(&snake)->x < 1 ||
                snake_body_back(&snake)->y > WINDOW_HEIGHT - 3 ||
                snake_body_back(&snake)->y < 1) {
                dir = (vec2f){1.0f, 0.0f};
                while (snake.size > 3) {
                    snake_body_pop_back(&snake);
                }
                *snake_body_at(&snake, 0) = (vec2f){cpm_floorf(WINDOW_WIDTH / 2.0f), cpm_floorf(WINDOW_HEIGHT / 2.0f)};
                *snake_body_at(&snake, 1) = (vec2f){(cpm_floorf(WINDOW_WIDTH / 2.0f)) + 1, cpm_floorf(WINDOW_HEIGHT / 2.0f)};
                *snake_body_at(&snake, 2) = (vec2f){(cpm_floorf(WINDOW_WIDTH / 2.0f) / 2) + 2, cpm_floorf(WINDOW_HEIGHT / 2.0f)};

                apple = (vec2f){(f32)(cprng_rand_range(1, WINDOW_WIDTH - 2)),
                                (f32)(cprng_rand_range(1, WINDOW_HEIGHT - 3))};
                score = 0;
            }
        }
        if (cplt_is_key_pressed('q')) {
            running = false;
        }

        if (cplt_is_key_pressed('w') && dir.y != 1.0f) {
            dir = (vec2f){0.0f, -1.0f};
        }
        if (cplt_is_key_pressed('s') && dir.y != -1.0f) {
            dir = (vec2f){0.0f, 1.0f};
        }
        if (cplt_is_key_pressed('a') && dir.x != 1.0f) {
            dir = (vec2f){-1.0f, 0.0f};
        }
        if (cplt_is_key_pressed('d') && dir.x != -1.0f) {
            dir = (vec2f){1.0f, 0.0f};
        }

        cplt_clear_bg(BLACK);
        cplt_clear(' ', WHITE);

        FOREACH_VEC(vec2f, snake_body, body, &snake) {
            cplt_draw_pixel((i32)body->x, (i32)body->y, "#", GREEN);
        }

        cplt_draw_pixel((i32)apple.x, (i32)apple.y, "O", RED);

        cplt_draw_rect(0, 0, WINDOW_WIDTH, 1, "-", WHITE);
        cplt_draw_rect(0, WINDOW_HEIGHT - 2, WINDOW_WIDTH, 1, "-", WHITE);
        cplt_draw_rect(0, 1, 1, WINDOW_HEIGHT - 3, "|", WHITE);
        cplt_draw_rect(WINDOW_WIDTH - 1, 1, 1, WINDOW_HEIGHT - 3, "|", WHITE);

        i8 fps_str[11];
        snprintf(fps_str, 11, "FPS: %d", fps);
        cplt_draw_text(0, WINDOW_HEIGHT - 1, fps_str, CYAN);

        i8 title[] = "SNAKE";
        cplt_draw_text((WINDOW_WIDTH / 2) - 3, WINDOW_HEIGHT - 1,
                       title, GREEN);

        i8 score_str[11];
        snprintf(score_str, 11, "Score: %d", score);
        cplt_draw_text(WINDOW_WIDTH - strlen(score_str), WINDOW_HEIGHT - 1,
                       score_str, YELLOW);

        cplt_render();
    }
    snake_body_destroy(&snake);
}
