#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/mygpu/memory.h"

struct mygpu_memory {
    uint8_t* data;
    size_t size;
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