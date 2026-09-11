#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "mygpu/framebuffer.h"
#include "mygpu/rasterizer.h"
#include "mygpu/vertex.h"

static void test_triangle(void)
{
    struct mygpu_framebuffer *framebuffer;
    struct mygpu_vertex v0;
    struct mygpu_vertex v1;
    struct mygpu_vertex v2;
    uint32_t color;

    framebuffer = mygpu_framebuffer_create(16, 16);
    assert(framebuffer != NULL);

    mygpu_framebuffer_clear(framebuffer, 0);

    v0.x = 2.0f;
    v0.y = 2.0f;
    v0.color = 0xff0000ffu;

    v1.x = 12.0f;
    v1.y = 2.0f;
    v1.color = 0xff0000ffu;

    v2.x = 2.0f;
    v2.y = 12.0f;
    v2.color = 0xff0000ffu;

    assert(mygpu_rasterize_triangle(
        framebuffer,
        &v0,
        &v1,
        &v2) == 0);

    assert(mygpu_framebuffer_get_pixel(
        framebuffer,
        3,
        3,
        &color) == 0);

    assert(color == 0xff0000ffu);

    mygpu_framebuffer_destroy(framebuffer);
}

int main(void)
{
    test_triangle();

    printf("rasterizer tests passed\n");

    return 0;
}