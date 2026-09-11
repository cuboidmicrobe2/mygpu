#include <stdint.h>
#include <stddef.h>

#include "mygpu/framebuffer.h"
#include "mygpu/rasterizer.h"
#include "mygpu/vertex.h"

static float edge_function(
    float ax,
    float ay,
    float bx,
    float by,
    float px,
    float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

int mygpu_rasterize_triangle(
    struct mygpu_framebuffer *framebuffer,
    const struct mygpu_vertex *v0,
    const struct mygpu_vertex *v1,
    const struct mygpu_vertex *v2)
{
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;

    float min_x;
    float min_y;
    float max_x;
    float max_y;

    int32_t start_x;
    int32_t start_y;
    int32_t end_x;
    int32_t end_y;

    float area;

    if (framebuffer == NULL ||
        v0 == NULL ||
        v1 == NULL ||
        v2 == NULL) {

        return -1;
    }

    framebuffer_width = mygpu_framebuffer_width(framebuffer);
    framebuffer_height = mygpu_framebuffer_height(framebuffer);

    area = edge_function(
        v0->x, v0->y,
        v1->x, v1->y,
        v2->x, v2->y
    );

    /* Degenerate triangle */
    if (area == 0.0f) {
        return 0;
    }

    min_x = v0->x;
    if (v1->x < min_x) min_x = v1->x;
    if (v2->x < min_x) min_x = v2->x;

    min_y = v0->y;
    if (v1->y < min_y) min_y = v1->y;
    if (v2->y < min_y) min_y = v2->y;

    max_x = v0->x;
    if (v1->x > max_x) max_x = v1->x;
    if (v2->x > max_x) max_x = v2->x;

    max_y = v0->y;
    if (v1->y > max_y) max_y = v1->y;
    if (v2->y > max_y) max_y = v2->y;

    if (max_x < 0.0f ||
        max_y < 0.0f ||
        min_x >= (float)framebuffer_width ||
        min_y >= (float)framebuffer_height) {
            return 0;
    }

    start_x = (int32_t)min_x;
    start_y = (int32_t)min_y;
    end_x = (int32_t)max_x;
    end_y = (int32_t)max_y;

    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;

    if (end_x >= (int32_t)framebuffer_width) {
        end_x = (int32_t)framebuffer_width - 1;
    }

    if (end_y >= (int32_t)framebuffer_height) {
        end_y = (int32_t)framebuffer_height - 1;
    }

    for (int32_t y = start_y; y <= end_y; y++) {
        for (int32_t x = start_x; x <= end_x; x++) {
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;

            float w0 = edge_function(v1->x, v1->y, v2->x, v2->y, px, py);

            float w1 = edge_function(v2->x, v2->y, v0->x, v0->y, px, py);

            float w2 = edge_function(v0->x, v0->y, v1->x, v1->y, px, py);

            if ((
                w0 >= 0.0f &&
                w1 >= 0.0f &&
                w2 >= 0.0f) ||
               (w0 <= 0.0f &&
                w1 <= 0.0f &&
                w2 <= 0.0f)) {
                
                if (mygpu_framebuffer_set_pixel(framebuffer, (uint32_t)x, (uint32_t)y, v0->color) != 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}