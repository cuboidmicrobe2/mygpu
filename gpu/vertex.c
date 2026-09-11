#include <stdint.h>

#include "mygpu/buffer.h"
#include "mygpu/vertex.h"

int mygpu_vertex_fetch (struct mygpu_buffer *buffer, uint32_t index, struct mygpu_vertex *vertex) {
    size_t offset;

    if (buffer == NULL || vertex == NULL) {
        return -1;
    }

    offset = (size_t)index * sizeof(*vertex);

    if (offset > mygpu_buffer_size(buffer)) {
        return -1;
    }

    if (sizeof(*vertex) > mygpu_buffer_size(buffer) - offset) {
        return -1;
    }

    return mygpu_buffer_read(buffer, offset, vertex, sizeof(*vertex));
}