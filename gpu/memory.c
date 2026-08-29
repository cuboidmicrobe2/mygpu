#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mygpu/memory.h"

struct mygpu_memory {
    uint8_t* data;
    size_t size;
    size_t next_free;
};

struct mygpu_memory *mygpu_memory_create(void)
{
    struct mygpu_memory* memory;

    memory = malloc(sizeof(*memory));

    if(memory == NULL) {
        return NULL;
    }

    memory->data = malloc(MYGPU_MEMORY_SIZE);

    if(memory->data == NULL) {
        free(memory);
        return NULL;
    }

    memory->size = MYGPU_MEMORY_SIZE;
    memory->next_free = 0;

    memset(memory->data, 0, memory->size);

    return memory;
}

void mygpu_memory_destroy(struct mygpu_memory *memory)
{
    if(memory == NULL) {
        return;
    }

    free(memory->data);
    free(memory);
}

int mygpu_memory_read(struct mygpu_memory *memory, uint32_t address, void *data, size_t size)
{
    if(memory == NULL || data == NULL) {
        return -1;
    }

    if((uint64_t)address + size > memory->size) {
        return -1;
    }

    memcpy(data, memory->data + address, size);

    return 0;
}

int mygpu_memory_write(struct mygpu_memory *memory, uint32_t address, const void *data, size_t size)
{
    if(memory == NULL || data == NULL) {
        return -1;
    }

    if((uint64_t)address + size > memory->size) {
        return -1;
    }

    memcpy(memory->data + address, data, size);

    return 0;
}

int mygpu_memory_alloc(struct mygpu_memory *memory, size_t size, uint32_t *address)
{
    if (memory == NULL || address == NULL || size == 0) {
        return -1;
    }

    if (size > memory->size - memory->next_free) {
        return -1;
    }

    *address = (uint32_t)memory->next_free;

    memory->next_free += size;

    return 0;
}