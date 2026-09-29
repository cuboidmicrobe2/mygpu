#include <stdint.h>
#include <stdio.h>

#include "mygpu/buffer.h"
#include "mygpu/commands.h"
#include "mygpu/framebuffer.h"
#include "mygpu/gpu.h"
#include "mygpu/vertex.h"

#include "test.h"

static struct mygpu_buffer *create_vertex_buffer(struct mygpu *gpu, const struct mygpu_vertex *vertices, uint32_t count)
{
    struct mygpu_buffer *buffer;

    buffer = mygpu_buffer_create(gpu, sizeof(*vertices) * count);

    require(buffer != NULL);

    require(mygpu_buffer_write(buffer, 0, vertices, sizeof(*vertices) * count) == 0);

    return buffer;
}

static struct mygpu_command_buffer *create_draw_command(struct mygpu_buffer *vertex_buffer)
{
    struct mygpu_command_buffer *command_buffer;
    struct mygpu_cmd_draw_triangles command;

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    return command_buffer;
}

static void test_basic_triangle(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){12.0f, 2.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){2.0f, 12.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    command_buffer = create_draw_command(vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 3, 3, &color) == 0);

    require(color == 0xff0000ffu);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 12, 12, &color) == 0);

    require(color != 0xff0000ffu);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_reverse_winding(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 12.0f, 0xff00ff00u};

    vertices[1] = (struct mygpu_vertex){12.0f, 2.0f, 0xff00ff00u};

    vertices[2] = (struct mygpu_vertex){2.0f, 2.0f, 0xff00ff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    command_buffer = create_draw_command(vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 3, 3, &color) == 0);

    require(color == 0xff00ff00u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_triangle_at_framebuffer_edge(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){0.0f, 0.0f, 0xffff0000u};

    vertices[1] = (struct mygpu_vertex){10.0f, 0.0f, 0xffff0000u};

    vertices[2] = (struct mygpu_vertex){0.0f, 10.0f, 0xffff0000u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    command_buffer = create_draw_command(vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 1, 1, &color) == 0);

    require(color == 0xffff0000u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_triangle_outside_framebuffer(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    struct mygpu_cmd_clear clear_command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0x12345678u;

    command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    require(mygpu_command_buffer_write(command_buffer, &clear_command, sizeof(clear_command)) == 0);

    vertices[0] = (struct mygpu_vertex){-20.0f, -20.0f, 0xff00ffffu};

    vertices[1] = (struct mygpu_vertex){-10.0f, -20.0f, 0xff00ffffu};

    vertices[2] = (struct mygpu_vertex){-20.0f, -10.0f, 0xff00ffffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    {
        struct mygpu_cmd_draw_triangles draw_command;

        draw_command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
        draw_command.vertex_address = mygpu_buffer_address(vertex_buffer);
        draw_command.vertex_offset = 0;
        draw_command.vertex_count = 3;
        draw_command.first_vertex = 0;

        require(mygpu_command_buffer_write(command_buffer, &draw_command, sizeof(draw_command)) == 0);
    }

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 0, 0, &color) == 0);

    require(color == 0x12345678u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_degenerate_triangle(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    struct mygpu_cmd_clear clear_command;
    struct mygpu_cmd_draw_triangles draw_command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xffffff00u};

    vertices[1] = (struct mygpu_vertex){6.0f, 6.0f, 0xffffff00u};

    vertices[2] = (struct mygpu_vertex){10.0f, 10.0f, 0xffffff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0x12345678u;

    require(mygpu_command_buffer_write(command_buffer, &clear_command, sizeof(clear_command)) == 0);

    draw_command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    draw_command.vertex_address = mygpu_buffer_address(vertex_buffer);
    draw_command.vertex_offset = 0;
    draw_command.vertex_count = 3;
    draw_command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &draw_command, sizeof(draw_command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 5, 5, &color) == 0);

    require(color == 0x12345678u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_multiple_triangles(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[6];

    struct mygpu_cmd_draw_triangles command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){8.0f, 2.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){2.0f, 8.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){20.0f, 20.0f, 0xff00ff00u};

    vertices[4] = (struct mygpu_vertex){26.0f, 20.0f, 0xff00ff00u};

    vertices[5] = (struct mygpu_vertex){20.0f, 26.0f, 0xff00ff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 6);

    command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    command.vertex_address = mygpu_buffer_address(vertex_buffer) + (3u * (uint32_t)sizeof(struct mygpu_vertex));

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 3, 3, &color) == 0);

    require(color == 0xff0000ffu);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 21, 21, &color) == 0);

    require(color == 0xff00ff00u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_vertex_buffer_offset(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[6];

    struct mygpu_cmd_draw_triangles command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){8.0f, 2.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){2.0f, 8.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){20.0f, 20.0f, 0xff00ff00u};

    vertices[4] = (struct mygpu_vertex){26.0f, 20.0f, 0xff00ff00u};

    vertices[5] = (struct mygpu_vertex){20.0f, 26.0f, 0xff00ff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 6);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 3u * (uint32_t)sizeof(struct mygpu_vertex);
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 21, 21, &color) == 0);

    require(color == 0xff00ff00u);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 3, 3, &color) == 0);

    require(color != 0xff0000ffu);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_unknown_vertex_buffer(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *command_buffer;
    struct mygpu_cmd_draw_triangles command;

    gpu = mygpu_create();
    require(gpu != NULL);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = 0xFFFFFFFFu;
    command.vertex_offset = 0;
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_destroy(gpu);
}

static void test_vertex_buffer_too_small(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[2];
    struct mygpu_cmd_draw_triangles command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){12.0f, 2.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 2);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_vertex_buffer_offset_too_small(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[4];
    struct mygpu_cmd_draw_triangles command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){2.0f, 2.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){12.0f, 2.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){2.0f, 12.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){12.0f, 12.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 4);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer) + (3u * (uint32_t)sizeof(struct mygpu_vertex));
    command.vertex_offset = 0;
    command.vertex_count = 3;
    command.first_vertex = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_triangle(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 20, 15, &color) == 0);

    require(color == 0xff0000ffu);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_first_index(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[6];
    uint32_t indices[6];

    struct mygpu_cmd_draw_indexed command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){50.0f, 10.0f, 0xff00ff00u};

    vertices[4] = (struct mygpu_vertex){70.0f, 10.0f, 0xff00ff00u};

    vertices[5] = (struct mygpu_vertex){60.0f, 30.0f, 0xff00ff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 6);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;
    indices[4] = 4;
    indices[5] = 5;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 3;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 60, 15, &color) == 0);

    require(color == 0xff00ff00u);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 20, 15, &color) == 0);

    require(color != 0xff0000ffu);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_unknown_vertex_buffer(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = 0;
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_unknown_index_buffer(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = 0;
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_index_buffer_too_small(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[2];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_vertex_index_out_of_bounds(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 3;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_vertex_buffer_offset(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[6];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    uint32_t color;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){50.0f, 10.0f, 0xff00ff00u};

    vertices[4] = (struct mygpu_vertex){70.0f, 10.0f, 0xff00ff00u};

    vertices[5] = (struct mygpu_vertex){60.0f, 30.0f, 0xff00ff00u};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 6);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 3u * (uint32_t)sizeof(struct mygpu_vertex);
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 60, 15, &color) == 0);

    require(color == 0xff00ff00u);

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), 20, 15, &color) == 0);

    require(color != 0xff0000ffu);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_vertex_offset_past_end(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 4u * (uint32_t)sizeof(struct mygpu_vertex);
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_vertex_offset_index_out_of_bounds(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[4];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};

    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};

    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertices[3] = (struct mygpu_vertex){40.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 4);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    /* index 3 is valid without the offset, but not after skipping one vertex */
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 3;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = (uint32_t)sizeof(struct mygpu_vertex);
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_first_index_overflow(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *decoy_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};
    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};
    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    vertex_buffer = create_vertex_buffer(gpu, vertices, 3);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    /* valid indices placed directly before the index buffer, so a wrapped index address reads a drawable triangle */
    decoy_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(decoy_buffer != NULL);

    require(mygpu_buffer_write(decoy_buffer, 0, indices, sizeof(indices)) == 0);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    require(mygpu_buffer_address(index_buffer) == mygpu_buffer_address(decoy_buffer) + sizeof(indices));

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    /* first_index + index_count wraps to 0 in 32-bit arithmetic */
    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = UINT32_MAX - 2u;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(decoy_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_indexed_partial_vertex_out_of_bounds(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_buffer *index_buffer;
    struct mygpu_command_buffer *command_buffer;

    struct mygpu_vertex vertices[3];
    uint32_t indices[3];

    struct mygpu_cmd_draw_indexed command;

    gpu = mygpu_create();
    require(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){10.0f, 10.0f, 0xff0000ffu};
    vertices[1] = (struct mygpu_vertex){30.0f, 10.0f, 0xff0000ffu};
    vertices[2] = (struct mygpu_vertex){20.0f, 30.0f, 0xff0000ffu};

    /* room for three vertices plus four bytes: vertex 3 starts inside the buffer but does not fit */
    vertex_buffer = mygpu_buffer_create(gpu, sizeof(vertices) + 4u);

    require(vertex_buffer != NULL);

    require(mygpu_buffer_write(vertex_buffer, 0, vertices, sizeof(vertices)) == 0);

    index_buffer = mygpu_buffer_create(gpu, sizeof(indices));

    require(index_buffer != NULL);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 3;

    require(mygpu_buffer_write(index_buffer, 0, indices, sizeof(indices)) == 0);

    command_buffer = mygpu_command_buffer_create(64);
    require(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_INDEXED;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_offset = 0;
    command.index_address = mygpu_buffer_address(index_buffer);
    command.index_count = 3;
    command.first_index = 0;

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);

    require(mygpu_command_buffer_validate(command_buffer) == 0);

    require(mygpu_command_buffer_execute(gpu, command_buffer) != 0);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(index_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

int main(void)
{
    run_test(test_basic_triangle);
    run_test(test_reverse_winding);
    run_test(test_triangle_at_framebuffer_edge);
    run_test(test_triangle_outside_framebuffer);
    run_test(test_degenerate_triangle);
    run_test(test_multiple_triangles);

    run_test(test_vertex_buffer_offset);

    run_test(test_unknown_vertex_buffer);
    run_test(test_vertex_buffer_too_small);
    run_test(test_vertex_buffer_offset_too_small);

    run_test(test_indexed_triangle);
    run_test(test_indexed_first_index);
    run_test(test_indexed_unknown_vertex_buffer);
    run_test(test_indexed_unknown_index_buffer);
    run_test(test_indexed_index_buffer_too_small);
    run_test(test_indexed_vertex_index_out_of_bounds);
    run_test(test_indexed_first_index_overflow);
    run_test(test_indexed_partial_vertex_out_of_bounds);

    run_test(test_indexed_vertex_buffer_offset);
    run_test(test_indexed_vertex_offset_past_end);
    run_test(test_indexed_vertex_offset_index_out_of_bounds);

    return test_finish();
}
