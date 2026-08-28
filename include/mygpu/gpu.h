#ifndef MYGPU_GPU_H
#define MYGPU_GPU_H

#include <stdint.h>

struct mygpu;

struct mygpu *mygpu_create(void);

void mygpu_destroy(struct mygpu *gpu);

void mygpu_reset(struct mygpu *gpu);

int mygpu_clear(struct mygpu *gpu, uint32_t color);

int mygpu_set_pixel(struct mygpu *gpu, uint32_t x, uint32_t y, uint32_t color);

int mygpu_get_pixel(struct mygpu *gpu, uint32_t x, uint32_t y, uint32_t *color);

int mygpu_is_presented(const struct mygpu *gpu);

#endif