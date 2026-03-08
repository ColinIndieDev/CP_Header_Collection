#pragma once

#include "../cplibrary/cpl.h"

f32 alpha_logo = 255.0f;
f32 alpha_bg = 255.0f;

b8 fading = true;
texture *logo;

void start_anim_init() {
    logo = malloc(sizeof(texture));
    cpl_load_texture(logo, "assets/images/icon.png", CPL_FILTER_LINEAR);
}

void start_anim_update() {
    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);
    cpl_draw_rect(&(vec2f){0.0f, 0.0f},
                  &(vec2f){cpl_get_screen_width(), cpl_get_screen_height()},
                  &(vec4f){0.0f, 0.0f, 0.0f, alpha_bg}, 0.0f);

    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, false);
    vec2f logo_pos = {(cpl_get_screen_width() / 2.0f) - (logo->size.x / 2.0f),
                      (cpl_get_screen_height() / 2.0f) - (logo->size.y / 2.0f)};
    cpl_draw_texture2D(logo, &logo_pos, &(vec2f){250.0f, 250.0f},
                       &(vec4f){0.0f, 0.0f, 0.0f, alpha_logo}, 0.0f);

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
