#include <stdlib.h>

#include "mygpu/buffer.h"
#include "mygpu/gpu_internal.h"
#include "mygpu/memory.h"

struct mygpu_buffer {
    struct mygpu *gpu;
    uint32_t address;
    size_t size;
};

struct mygpu_buffer *mygpu_buffer_create(struct mygpu *gpu, size_t size)
{
    struct mygpu_buffer *buffer;

    if (gpu == NULL || size == 0) {
        return NULL;
    }

    buffer = malloc(sizeof(*buffer));

    if (buffer == NULL) {
        return NULL;
    }

    if (mygpu_memory_alloc(gpu->memory, size, &buffer->address) != 0) {
        free(buffer);
        return NULL;
    }

    buffer->gpu = gpu;
    buffer->size = size;

    return buffer;
}

void mygpu_buffer_destroy(struct mygpu_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    free(buffer);
}

int mygpu_buffer_read(struct mygpu_buffer *buffer, size_t offset, void *data, size_t size)
{
    if (buffer == NULL || data == NULL) {
        return -1;
    }

    if (offset > buffer->size) {
        return -1;
    }

    if (size > buffer->size - offset) {
        return -1;
    }

    return mygpu_memory_read(buffer->gpu->memory, buffer->address + offset, data, size);
}

int mygpu_buffer_write(struct mygpu_buffer *buffer, size_t offset, const void *data, size_t size)
{
    if (buffer == NULL || data == NULL) {
        return -1;
    }

    if (offset > buffer->size) {
        return -1;
    }

    if (size > buffer->size - offset) {
        return -1;
    }

    return mygpu_memory_write(buffer->gpu->memory, buffer->address + offset, data, size);
}

size_t mygpu_buffer_size(const struct mygpu_buffer *buffer)
{
    if (buffer == NULL) {
        return 0;
    }

    return buffer->size;
}

uint32_t mygpu_buffer_address(const struct mygpu_buffer *buffer)
{
    if (buffer == NULL) {
        return 0;
    }

    return buffer->address;
}