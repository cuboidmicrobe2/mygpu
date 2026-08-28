#ifndef MYGPU_BUFFER_H
#define MYGPU_BUFFER_H

#include <stddef.h>
#include <stdint.h>

struct mygpu;

struct mygpu_buffer;

struct mygpu_buffer *mygpu_buffer_create(struct mygpu *gpu, size_t size);

void mygpu_buffer_destroy(struct mygpu_buffer *buffer);

int mygpu_buffer_read(struct mygpu_buffer *buffer, size_t offset, void *data, size_t size);

int mygpu_buffer_write(struct mygpu_buffer *buffer, size_t offset, const void *data, size_t size);

size_t mygpu_buffer_size(const struct mygpu_buffer *buffer);

#endif