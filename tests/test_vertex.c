#include <stdint.h>
#include <stdio.h>

#include "mygpu/vertex.h"

#include "test.h"

static void test_vertex_layout(void)
{
    struct mygpu_vertex vertex;

    require(sizeof(vertex) == 12);
    require(sizeof(vertex.x) == 4);
    require(sizeof(vertex.y) == 4);
    require(sizeof(vertex.color) == 4);
}

static void test_vertex_values(void)
{
    struct mygpu_vertex vertex;

    vertex.x = 10.0f;
    vertex.y = 20.0f;
    vertex.color = 0xff00ffffu;

    require(vertex.x == 10.0f);
    require(vertex.y == 20.0f);
    require(vertex.color == 0xff00ffffu);
}

int main(void)
{
    run_test(test_vertex_layout);
    run_test(test_vertex_values);

    return test_finish();
}
