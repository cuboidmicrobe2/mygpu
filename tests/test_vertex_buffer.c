#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "mygpu/buffer.h"
#include "mygpu/gpu.h"
#include "mygpu/vertex.h"

static void test_vertex_fetch(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;
    struct mygpu_vertex vertices[3];
    struct mygpu_vertex vertex;

    gpu = mygpu_create();
    assert(gpu != NULL);

    buffer = mygpu_buffer_create(
        gpu,
        sizeof(vertices));

    assert(buffer != NULL);

    vertices[0].x = 10.0f;
    vertices[0].y = 20.0f;
    vertices[0].color = 0xff0000ffu;

    vertices[1].x = 30.0f;
    vertices[1].y = 40.0f;
    vertices[1].color = 0x00ff00ffu;

    vertices[2].x = 50.0f;
    vertices[2].y = 60.0f;
    vertices[2].color = 0x0000ffffu;

    assert(mygpu_buffer_write(
        buffer,
        0,
        vertices,
        sizeof(vertices)) == 0);

    assert(mygpu_vertex_fetch(buffer, 0, &vertex) == 0);
    assert(vertex.x == 10.0f);
    assert(vertex.y == 20.0f);
    assert(vertex.color == 0xff0000ffu);

    assert(mygpu_vertex_fetch(buffer, 1, &vertex) == 0);
    assert(vertex.x == 30.0f);
    assert(vertex.y == 40.0f);
    assert(vertex.color == 0x00ff00ffu);

    assert(mygpu_vertex_fetch(buffer, 2, &vertex) == 0);
    assert(vertex.x == 50.0f);
    assert(vertex.y == 60.0f);
    assert(vertex.color == 0x0000ffffu);

    assert(mygpu_vertex_fetch(buffer, 3, &vertex) != 0);

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

int main(void)
{
    test_vertex_fetch();

    printf("vertex buffer tests passed\n");

    return 0;
}