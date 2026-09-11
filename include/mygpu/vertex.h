#ifndef MYGPU_VERTEX_H
#define MYGPU_VERTEX_H

#include <stdint.h>

struct mygpu_buffer;

struct mygpu_vertex
{
    float x;
    float y;
    uint32_t color;
};

int mygpu_vertex_fetch(struct mygpu_buffer *buffer, uint32_t index, struct mygpu_vertex *vertex);

#endif