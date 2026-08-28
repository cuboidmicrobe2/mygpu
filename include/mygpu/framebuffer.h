#ifndef MYGPU_FRAMEBUFFER_H
#define MYGPU_FRAMEBUFFER_H

#include <stdint.h>

struct mygpu_framebuffer;

struct mygpu_framebuffer *mygpu_framebuffer_create(uint32_t width, uint32_t height);

void mygpu_framebuffer_destroy(struct mygpu_framebuffer *framebuffer);

void mygpu_framebuffer_clear(struct mygpu_framebuffer *framebuffer, uint32_t color);

int mygpu_framebuffer_set_pixel(struct mygpu_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t color);

int mygpu_framebuffer_get_pixel(struct mygpu_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t *color);

uint32_t mygpu_framebuffer_width(const struct mygpu_framebuffer *framebuffer);

uint32_t mygpu_framebuffer_height(const struct mygpu_framebuffer *framebuffer);

#endif