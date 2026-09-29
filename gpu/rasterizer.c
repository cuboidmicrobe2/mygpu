#include <stdint.h>
#include <stddef.h>

#include "mygpu/framebuffer.h"
#include "mygpu/rasterizer.h"
#include "mygpu/vertex.h"

static float edge_function(float ax, float ay, float bx, float by, float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

int mygpu_rasterize_triangle(struct mygpu_framebuffer *framebuffer, const struct mygpu_rect *clip,
                             const struct mygpu_vertex *v0, const struct mygpu_vertex *v1,
                             const struct mygpu_vertex *v2)
{
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;

    /* Pixels that may be written: [clip_left, clip_right) x [clip_top, clip_bottom). */
    uint64_t clip_left;
    uint64_t clip_top;
    uint64_t clip_right;
    uint64_t clip_bottom;

    float min_x;
    float min_y;
    float max_x;
    float max_y;

    int32_t start_x;
    int32_t start_y;
    int32_t end_x;
    int32_t end_y;

    float area;

    if (framebuffer == NULL || clip == NULL || v0 == NULL || v1 == NULL || v2 == NULL) {

        return -1;
    }

    framebuffer_width = mygpu_framebuffer_width(framebuffer);
    framebuffer_height = mygpu_framebuffer_height(framebuffer);

    /* 64-bit so x + width can't wrap around; then keep the rect inside the framebuffer. */
    clip_left = clip->x;
    clip_top = clip->y;
    clip_right = (uint64_t)clip->x + clip->width;
    clip_bottom = (uint64_t)clip->y + clip->height;

    if (clip_right > framebuffer_width)
        clip_right = framebuffer_width;
    if (clip_bottom > framebuffer_height)
        clip_bottom = framebuffer_height;

    if (clip_left >= clip_right || clip_top >= clip_bottom) {
        return 0;
    }

    area = edge_function(v0->x, v0->y, v1->x, v1->y, v2->x, v2->y);

    /* Degenerate triangle */
    if (area == 0.0f) {
        return 0;
    }

    min_x = v0->x;
    if (v1->x < min_x)
        min_x = v1->x;
    if (v2->x < min_x)
        min_x = v2->x;

    min_y = v0->y;
    if (v1->y < min_y)
        min_y = v1->y;
    if (v2->y < min_y)
        min_y = v2->y;

    max_x = v0->x;
    if (v1->x > max_x)
        max_x = v1->x;
    if (v2->x > max_x)
        max_x = v2->x;

    max_y = v0->y;
    if (v1->y > max_y)
        max_y = v1->y;
    if (v2->y > max_y)
        max_y = v2->y;

    if (max_x < (float)clip_left || max_y < (float)clip_top || min_x >= (float)clip_right ||
        min_y >= (float)clip_bottom) {
        return 0;
    }

    start_x = (int32_t)min_x;
    start_y = (int32_t)min_y;
    end_x = (int32_t)max_x;
    end_y = (int32_t)max_y;

    if (start_x < (int32_t)clip_left)
        start_x = (int32_t)clip_left;
    if (start_y < (int32_t)clip_top)
        start_y = (int32_t)clip_top;

    if (end_x >= (int32_t)clip_right) {
        end_x = (int32_t)clip_right - 1;
    }

    if (end_y >= (int32_t)clip_bottom) {
        end_y = (int32_t)clip_bottom - 1;
    }

    for (int32_t y = start_y; y <= end_y; y++) {
        for (int32_t x = start_x; x <= end_x; x++) {
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;

            float w0 = edge_function(v1->x, v1->y, v2->x, v2->y, px, py);

            float w1 = edge_function(v2->x, v2->y, v0->x, v0->y, px, py);

            float w2 = edge_function(v0->x, v0->y, v1->x, v1->y, px, py);

            if ((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) || (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)) {

                if (mygpu_framebuffer_set_pixel(framebuffer, (uint32_t)x, (uint32_t)y, v0->color) != 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}
