#include <stdlib.h>

#include "gpu_internal.h"
#include "mygpu/gpu.h"

#define MYGPU_WIDTH 320u
#define MYGPU_HEIGHT 200u

struct mygpu *mygpu_create(void)
{
    struct mygpu *gpu;

    gpu = malloc(sizeof(*gpu));

    if (gpu == NULL) {
        return NULL;
    }

    gpu->memory = NULL;
    gpu->registers = NULL;
    gpu->framebuffer = NULL;

    gpu->memory = mygpu_memory_create();

    if (gpu->memory == NULL) {
        free(gpu);
        return NULL;
    }

    gpu->registers = mygpu_registers_create();

    if(gpu->registers == NULL) {
        mygpu_memory_destroy(gpu->memory);
        free(gpu);
        return NULL;
    }

    gpu->framebuffer = mygpu_framebuffer_create(MYGPU_WIDTH, MYGPU_HEIGHT);

    if(gpu->framebuffer == NULL) {
        mygpu_registers_destroy(gpu->registers);
        mygpu_memory_destroy(gpu->memory);
        free(gpu);
        return NULL;
    }

    mygpu_reset(gpu);

    return gpu;
}

void mygpu_destroy(struct mygpu *gpu)
{
    if (gpu == NULL) {
        return;
    }

    mygpu_framebuffer_destroy(gpu->framebuffer);
    mygpu_registers_destroy(gpu->registers);
    mygpu_memory_destroy(gpu->memory);

    free(gpu);
}

void mygpu_reset(struct mygpu *gpu)
{
    if (gpu == NULL) {
        return;
    }

    mygpu_registers_reset(gpu->registers);
    mygpu_framebuffer_clear(gpu->framebuffer, 0x000000FF);

    gpu->presented = 0;
}

int mygpu_clear(struct mygpu *gpu, uint32_t color)
{
    if (gpu == NULL) {
        return -1;
    }

    mygpu_framebuffer_clear(gpu->framebuffer, color);

    return 0;
}

int mygpu_set_pixel(struct mygpu *gpu, uint32_t x, uint32_t y, uint32_t color)
{
    if (gpu == NULL) {
        return -1;
    }

    return mygpu_framebuffer_set_pixel(gpu->framebuffer, x, y, color);
}

int mygpu_get_pixel(struct mygpu *gpu, uint32_t x, uint32_t y, uint32_t *color)
{
    if (gpu == NULL) {
        return -1;
    }

    return mygpu_framebuffer_get_pixel(gpu->framebuffer, x, y, color);
}

int mygpu_is_presented(const struct mygpu *gpu)
{
    if (gpu == NULL) {
        return 0;
    }

    return gpu->presented;
}