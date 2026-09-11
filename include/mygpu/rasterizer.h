#ifndef MYGPU_RASTERIZER_H
#define MYGPU_RASTERIZER_H

#include <stdint.h>

struct mygpu_framebuffer;
struct mygpu_vertex;

int mygpu_rasterize_triangle(
    struct mygpu_framebuffer *framebuffer,
    const struct mygpu_vertex *v0,
    const struct mygpu_vertex *v1,
    const struct mygpu_vertex *v2);

#endif