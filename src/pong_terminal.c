#include "../cplterminal/cplt.h"
#include "../cpstd/cpmath.h"
#include "../cpstd/cprng.h"

typedef struct {
    vec2f pos;
    vec2f size;
} Player;

void UpdateBall(Player p1, Player p2, vec2f *ballPos, vec2f *ballDir,
                vec2f originBallPos, f32 ballSpeed, i32 borderBegin,
                i32 borderEnd, i32 *p1Score, i32 *p2Score);

void Draw(Player *p1, Player *p2, vec2f ballPos, i32 borderBegin, i32 borderEnd,
          i32 p1Score, i32 p2Score);

int main() {
    cplt_init(75, 50);
    cprng_rand_seed();

    Player p1 = {{1.0f, (f32)height / 2.0f}, {2.0f, 7.0f}};
    Player p2 = {{(f32)width - 3.0f, (f32)height / 2.0f}, {2.0f, 7.0f}};
    i32 p1Score = 0;
    i32 p2Score = 0;

    vec2f originBallPos = {(f32)width / 2.0f, (f32)height / 2.0f};

    f32 ballSpeed = 25.0f;
    vec2f ballPos = originBallPos;
    vec2f ballDir = {cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed,
                     cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed};

    i32 borderBegin = 5;
    i32 borderEnd = height - 1 - 5;

    bool start = false;

    while (running) {
        cplt_calc_fps();
        cplt_calc_dt();
        cplt_update_input();

        if (cplt_is_key_pressed('q') && !start) {
            start = true;
        }
        if (start) {
            f32 speed = 10.0f;
            if (cplt_is_key_down('w')) {
                p1.pos.y -= speed * cplt_get_dt();
            }
            if (cplt_is_key_down('s')) {
                p1.pos.y += speed * cplt_get_dt();
            }
            p1.pos.y =
                CPM_CLAMP(p1.pos.y, borderBegin + 1, borderEnd - p1.size.y);

            if (cplt_is_key_down('h')) {
                p2.pos.y -= speed * cplt_get_dt();
            }
            if (cplt_is_key_down('n')) {
                p2.pos.y += speed * cplt_get_dt();
            }
            p2.pos.y =
                CPM_CLAMP(p2.pos.y, borderBegin + 1, borderEnd - p2.size.y);

            UpdateBall(p1, p2, &ballPos, &ballDir, originBallPos, ballSpeed,
                       borderBegin, borderEnd, &p1Score, &p2Score);
        }

        Draw(&p1, &p2, ballPos, borderBegin, borderEnd, p1Score, p2Score);
    }
}

void UpdateBall(Player p1, Player p2, vec2f *ballPos, vec2f *ballDir,
                vec2f originBallPos, f32 ballSpeed, i32 borderBegin,
                i32 borderEnd, i32 *p1Score, i32 *p2Score) {
    vec2f ballDirDT = {ballDir->x * cplt_get_dt(), ballDir->y * cplt_get_dt()};
    *ballPos = vec2f_add(ballPos, &ballDirDT);

    if (ballPos->y > (f32)borderEnd) {
        ballDir->y *= -1.0f;
        ballPos->y = (f32)borderEnd - 1.0f;
    }
    if (ballPos->y < (f32)borderBegin + 1) {
        ballDir->y *= -1.0f;
        ballPos->y = (f32)borderBegin + 2.0f;
    }

    rect ballCollider = {ballPos->x, ballPos->y, 1.0f, 1.0f};
    rect p1Collider = {p1.pos.x, p1.pos.y, p1.size.x, p1.size.y};
    rect p2Collider = {p2.pos.x, p2.pos.y, p2.size.x, p2.size.y};

    if (cplt_check_collision_rects(&ballCollider, &p1Collider)) {
        ballPos->x = p1.pos.x + 2.0f;
        ballDir->x *= -1.0f;
    }
    if (cplt_check_collision_rects(&ballCollider, &p2Collider)) {
        ballPos->x = p2.pos.x - 1.0f;
        ballDir->x *= -1.0f;
    }

    if (ballPos->x >= (f32)width) {
        (*p1Score)++;
        *ballPos = originBallPos;
        ballDir->x = cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed;
        ballDir->y = cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed;
    }
    if (ballPos->x < 0.0f) {
        (*p2Score)++;
        *ballPos = originBallPos;
        ballDir->x = cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed;
        ballDir->y = cprng_rand() % 2 == 0 ? -ballSpeed : ballSpeed;
    }
}

void Draw(Player *p1, Player *p2, vec2f ballPos, i32 borderBegin, i32 borderEnd,
          i32 p1Score, i32 p2Score) {
    cplt_clear_bg(BLACK);
    cplt_clear(' ', WHITE);

    cplt_draw_rect(0, borderBegin, width, 1, "-", WHITE);
    cplt_draw_rect(0, borderEnd, width, 1, "-", WHITE);
    cplt_draw_rect(width / 2, borderBegin + 1, 1, CPM_ABS(borderBegin - borderEnd) - 1, "|", WHITE);

    cplt_draw_rect((i32)p1->pos.x, (i32)p1->pos.y, (i32)p1->size.x,
                   (i32)p1->size.y, "#", BLUE);
    cplt_draw_rect((i32)p2->pos.x, (i32)p2->pos.y, (i32)p2->size.x,
                   (i32)p2->size.y, "#", RED);

    cplt_draw_pixel((i32)ballPos.x, (i32)ballPos.y, "O", WHITE);

    char fpsStr[25];
    snprintf(fpsStr, sizeof(fpsStr), "FPS: %d", fps);
    cplt_draw_text(0, 0, fpsStr, WHITE);

    char p1ScoreStr[25];
    snprintf(p1ScoreStr, sizeof(p1ScoreStr), "P1: %d", p1Score);
    cplt_draw_text(0, borderBegin - 1, p1ScoreStr, BLUE);

    char p2ScoreStr[25];
    int p2ScoreStrLen =
        snprintf(p2ScoreStr, sizeof(p2ScoreStr), "P2: %d", p2Score);
    cplt_draw_text(width - p2ScoreStrLen, borderBegin - 1, p2ScoreStr, RED);

    cplt_render();
}
