#ifndef MYGPU_MEMORY_H
#define MYGPU_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define MYGPU_MEMORY_SIZE (64u * 1024u)

struct mygpu_memory;

struct mygpu_memory *mygpu_memory_create(void);

void mygpu_memory_destroy(struct mygpu_memory *memory);

int mygpu_memory_read(struct mygpu_memory *memory, uint32_t address, void *data, size_t size);
int mygpu_memory_write(struct mygpu_memory *memory, uint32_t address, const void *data, size_t size);

#endif