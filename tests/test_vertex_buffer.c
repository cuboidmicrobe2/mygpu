#include <stdint.h>
#include <stdio.h>

#include "mygpu/buffer.h"
#include "mygpu/gpu.h"
#include "mygpu/vertex.h"

#include "test.h"

static void test_vertex_fetch(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;
    struct mygpu_vertex vertices[3];
    struct mygpu_vertex vertex;

    gpu = mygpu_create();
    require(gpu != NULL);

    buffer = mygpu_buffer_create(gpu, sizeof(vertices));

    require(buffer != NULL);

    vertices[0].x = 10.0f;
    vertices[0].y = 20.0f;
    vertices[0].color = 0xff0000ffu;

    vertices[1].x = 30.0f;
    vertices[1].y = 40.0f;
    vertices[1].color = 0x00ff00ffu;

    vertices[2].x = 50.0f;
    vertices[2].y = 60.0f;
    vertices[2].color = 0x0000ffffu;

    require(mygpu_buffer_write(buffer, 0, vertices, sizeof(vertices)) == 0);

    require(mygpu_vertex_fetch(mygpu_get_memory(gpu), mygpu_buffer_address(buffer), 0, &vertex) == 0);
    require(vertex.x == 10.0f);
    require(vertex.y == 20.0f);
    require(vertex.color == 0xff0000ffu);

    require(mygpu_vertex_fetch(mygpu_get_memory(gpu), mygpu_buffer_address(buffer), 1, &vertex) == 0);
    require(vertex.x == 30.0f);
    require(vertex.y == 40.0f);
    require(vertex.color == 0x00ff00ffu);

    require(mygpu_vertex_fetch(mygpu_get_memory(gpu), mygpu_buffer_address(buffer), 2, &vertex) == 0);
    require(vertex.x == 50.0f);
    require(vertex.y == 60.0f);
    require(vertex.color == 0x0000ffffu);

    require(mygpu_vertex_fetch(mygpu_get_memory(gpu), mygpu_buffer_address(buffer), 3, &vertex) == 0);

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

int main(void)
{
    run_test(test_vertex_fetch);

    return test_finish();
}
