#include <stdlib.h>
#include <string.h>

#include "mygpu/buffer.h"

struct mygpu_buffer {
    uint8_t *data;
    size_t size;
    uint32_t address;
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

    buffer->data = malloc(size);

    if (buffer->data == NULL) {
        free(buffer);
        return NULL;
    }

    buffer->size = size;
    buffer->address = 0;

    memset(buffer->data, 0, size);

    return buffer;
}

void mygpu_buffer_destroy(struct mygpu_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    free(buffer->data);
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

    memcpy(data, buffer->data + offset, size);

    return 0;
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

    memcpy(buffer->data + offset, data, size);

    return 0;
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