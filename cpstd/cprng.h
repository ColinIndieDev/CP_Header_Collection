#pragma once

#include "cpbase.h"
#include "cpmath.h"
#include <time.h>

typedef struct {
    u64 state;
    u64 inc;
} cprng_state;

static cprng_state rng_state = {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL};

u32 cprng_randr(cprng_state *rng) {
    u64 oldState = rng->state;
    rng->state = (oldState * 6364136223846793005ULL) + rng->inc;
    u32 xorshifted = ((oldState >> 18u) ^ oldState) >> 27u;
    u32 rot = oldState >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void cprng_rand_seedr(cprng_state *rng) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    u64 seed = ((u64)ts.tv_sec << 32) ^ ts.tv_nsec;

    rng->state = 0;
    rng->inc = (seed << 1u) | 1u;

    cprng_randr(rng);
    rng->state += seed;
    cprng_randr(rng);
}

void cprng_rand_seed() { cprng_rand_seedr(&rng_state); }

f32 cprng_randfr(cprng_state *rng) {
    return (f32)cprng_randr(rng) / (f32)CPM_U32_MAX;
}

void cprng_seedr(cprng_state *rng, u64 state, u64 seq) {
    rng->state = 0U;
    rng->inc = (seq << 1u) | 1u;
    cprng_randr(rng);
    rng->state += state;
    cprng_randr(rng);
}

void cprng_seed(u64 state, u64 seq) { cprng_seedr(&rng_state, state, seq); }

u32 cprng_rand() { return cprng_randr(&rng_state); }

u32 cprnr_rand_range(u32 min, u32 max) {
    return min + (cprng_rand() % (max - min + 1));
}

f32 cprng_randf_norm() { return cprng_randfr(&rng_state); }

f32 cprng_randf() { return cpm_floorf((f32)cprng_rand() / 100000.0f) + cprng_randf_norm(); }

f32 cprng_randf_range(f32 min, f32 max) {
    return min + (cprng_randf_norm() * (max - min));
}

i32 cprng_rand_range(i32 min, i32 max) {
    return min + (i32)(cprng_rand() % (max - min + 1));
}
