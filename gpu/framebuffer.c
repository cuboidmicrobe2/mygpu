#include <stdlib.h>
#include <stdint.h>

#include "../include/mygpu/framebuffer.h"

struct mygpu_framebuffer {
    uint32_t width;
    uint32_t height;
    uint32_t *pixels;
};

struct mygpu_framebuffer *mygpu_framebuffer_create(uint32_t width, uint32_t height)
{
    struct mygpu_framebuffer* framebuffer;
    size_t pixel_count;

    if (width == 0 || height == 0) {
        return NULL;
    }

    if ((size_t)width > SIZE_MAX / height) {
        return NULL;
    }

    pixel_count = (size_t)width * height;

    framebuffer = malloc(sizeof(*framebuffer));

    if (framebuffer == NULL) {
        return NULL;
    }

    framebuffer->pixels = calloc(pixel_count, sizeof(*framebuffer->pixels));

    if (framebuffer->pixels == NULL) {
        free(framebuffer);
        return NULL;
    }

    framebuffer->width = width;
    framebuffer->height = height;

    return framebuffer;
}

void mygpu_framebuffer_destroy(struct mygpu_framebuffer *framebuffer)
{
    if (framebuffer == NULL) {
        return;
    }

    free(framebuffer->pixels);
    free(framebuffer);
}

void mygpu_framebuffer_clear(struct mygpu_framebuffer *framebuffer, uint32_t color)
{
    if (framebuffer == NULL) {
        return;
    }

    size_t pixel_count = (size_t)framebuffer->width * framebuffer->height;

    for (size_t i = 0; i < pixel_count; i++) {
        framebuffer->pixels[i] = color;
    }
}

int mygpu_framebuffer_set_pixel(struct mygpu_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t color)
{
    if (framebuffer == NULL) {
        return -1;
    }

    if (x >= framebuffer->width || y >= framebuffer->height) {
        return -1;
    }

    size_t index = (size_t)y * framebuffer->width + x;

    framebuffer->pixels[index] = color;

    return 0;
}

int mygpu_framebuffer_get_pixel(struct mygpu_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t *color)
{
    if (framebuffer == NULL || color == NULL) {
        return -1;
    }

    if (x >= framebuffer->width || y >= framebuffer->height) {
        return -1;
    }

    size_t index = (size_t)y * framebuffer->width + x;

    *color = framebuffer->pixels[index];

    return 0;
}

uint32_t mygpu_framebuffer_width(const struct mygpu_framebuffer *framebuffer)
{
    if (framebuffer == NULL) {
        return 0;
    }

    return framebuffer->width;
}

uint32_t mygpu_framebuffer_height(const struct mygpu_framebuffer *framebuffer)
{
    if (framebuffer == NULL) {
        return 0;
    }

    return framebuffer->height;
}