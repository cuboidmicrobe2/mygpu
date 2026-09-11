#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "mygpu/vertex.h"

static void test_vertex_layout(void)
{
    struct mygpu_vertex vertex;

    assert(sizeof(vertex) == 12);
    assert(sizeof(vertex.x) == 4);
    assert(sizeof(vertex.y) == 4);
    assert(sizeof(vertex.color) == 4);
}

static void test_vertex_values(void)
{
    struct mygpu_vertex vertex;

    vertex.x = 10.0f;
    vertex.y = 20.0f;
    vertex.color = 0xff00ffffu;

    assert(vertex.x == 10.0f);
    assert(vertex.y == 20.0f);
    assert(vertex.color == 0xff00ffffu);
}

int main(void)
{
    test_vertex_layout();
    test_vertex_values();

    printf("vertex tests passed\n");

    return 0;
}