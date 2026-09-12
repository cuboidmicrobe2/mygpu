#include <stdint.h>

#include "mygpu/memory.h"
#include "mygpu/vertex.h"

int mygpu_vertex_fetch (struct mygpu_memory *memory, uint32_t address, uint32_t index, struct mygpu_vertex *vertex) {
    uint32_t vertex_address;

    if (memory == NULL || vertex == NULL) {
        return -1;
    }

    vertex_address = address + (index * (uint32_t)sizeof(*vertex));

    return mygpu_memory_read(
        memory, 
        vertex_address, 
        vertex, 
        sizeof(*vertex));
}