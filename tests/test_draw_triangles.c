#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "mygpu/buffer.h"
#include "mygpu/commands.h"
#include "mygpu/framebuffer.h"
#include "mygpu/gpu.h"
#include "mygpu/vertex.h"

static struct mygpu_buffer *create_vertex_buffer(
    struct mygpu *gpu,
    const struct mygpu_vertex *vertices,
    uint32_t count)
{
    struct mygpu_buffer *buffer;

    buffer = mygpu_buffer_create(
        gpu,
        sizeof(*vertices) * count);

    assert(buffer != NULL);

    assert(mygpu_buffer_write(
        buffer,
        0,
        vertices,
        sizeof(*vertices) * count) == 0);

    return buffer;
}

static struct mygpu_command_buffer *create_draw_command(
    struct mygpu_buffer *vertex_buffer)
{
    struct mygpu_command_buffer *command_buffer;
    struct mygpu_cmd_draw_triangles command;

    command_buffer = mygpu_command_buffer_create(64);
    assert(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_count = 3;

    assert(mygpu_command_buffer_write(
        command_buffer,
        &command,
        sizeof(command)) == 0);

    assert(mygpu_command_buffer_validate(command_buffer) == 0);

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
    assert(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){
        2.0f, 2.0f, 0xff0000ffu
    };

    vertices[1] = (struct mygpu_vertex){
        12.0f, 2.0f, 0xff0000ffu
    };

    vertices[2] = (struct mygpu_vertex){
        2.0f, 12.0f, 0xff0000ffu
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        3);

    command_buffer = create_draw_command(vertex_buffer);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        3,
        3,
        &color) == 0);

    assert(color == 0xff0000ffu);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        12,
        12,
        &color) == 0);

    assert(color != 0xff0000ffu);

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
    assert(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){
        2.0f, 12.0f, 0xff00ff00u
    };

    vertices[1] = (struct mygpu_vertex){
        12.0f, 2.0f, 0xff00ff00u
    };

    vertices[2] = (struct mygpu_vertex){
        2.0f, 2.0f, 0xff00ff00u
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        3);

    command_buffer = create_draw_command(vertex_buffer);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        3,
        3,
        &color) == 0);

    assert(color == 0xff00ff00u);

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
    assert(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){
        0.0f, 0.0f, 0xffff0000u
    };

    vertices[1] = (struct mygpu_vertex){
        10.0f, 0.0f, 0xffff0000u
    };

    vertices[2] = (struct mygpu_vertex){
        0.0f, 10.0f, 0xffff0000u
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        3);

    command_buffer = create_draw_command(vertex_buffer);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        1,
        1,
        &color) == 0);

    assert(color == 0xffff0000u);

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
    assert(gpu != NULL);

    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0x12345678u;

    command_buffer = mygpu_command_buffer_create(128);
    assert(command_buffer != NULL);

    assert(mygpu_command_buffer_write(
        command_buffer,
        &clear_command,
        sizeof(clear_command)) == 0);

    vertices[0] = (struct mygpu_vertex){
        -20.0f, -20.0f, 0xff00ffffu
    };

    vertices[1] = (struct mygpu_vertex){
        -10.0f, -20.0f, 0xff00ffffu
    };

    vertices[2] = (struct mygpu_vertex){
        -20.0f, -10.0f, 0xff00ffffu
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        3);

    {
        struct mygpu_cmd_draw_triangles draw_command;

        draw_command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
        draw_command.vertex_address =
            mygpu_buffer_address(vertex_buffer);
        draw_command.vertex_count = 3;

        assert(mygpu_command_buffer_write(
            command_buffer,
            &draw_command,
            sizeof(draw_command)) == 0);
    }

    assert(mygpu_command_buffer_validate(command_buffer) == 0);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        0,
        0,
        &color) == 0);

    assert(color == 0x12345678u);

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
    assert(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){
        2.0f, 2.0f, 0xffffff00u
    };

    vertices[1] = (struct mygpu_vertex){
        6.0f, 6.0f, 0xffffff00u
    };

    vertices[2] = (struct mygpu_vertex){
        10.0f, 10.0f, 0xffffff00u
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        3);

    command_buffer = mygpu_command_buffer_create(128);
    assert(command_buffer != NULL);

    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0x12345678u;

    assert(mygpu_command_buffer_write(
        command_buffer,
        &clear_command,
        sizeof(clear_command)) == 0);

    draw_command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    draw_command.vertex_address =
        mygpu_buffer_address(vertex_buffer);
    draw_command.vertex_count = 3;

    assert(mygpu_command_buffer_write(
        command_buffer,
        &draw_command,
        sizeof(draw_command)) == 0);

    assert(mygpu_command_buffer_validate(command_buffer) == 0);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        5,
        5,
        &color) == 0);

    assert(color == 0x12345678u);

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
    assert(gpu != NULL);

    vertices[0] = (struct mygpu_vertex){
        2.0f, 2.0f, 0xff0000ffu
    };

    vertices[1] = (struct mygpu_vertex){
        8.0f, 2.0f, 0xff0000ffu
    };

    vertices[2] = (struct mygpu_vertex){
        2.0f, 8.0f, 0xff0000ffu
    };

    vertices[3] = (struct mygpu_vertex){
        20.0f, 20.0f, 0xff00ff00u
    };

    vertices[4] = (struct mygpu_vertex){
        26.0f, 20.0f, 0xff00ff00u
    };

    vertices[5] = (struct mygpu_vertex){
        20.0f, 26.0f, 0xff00ff00u
    };

    vertex_buffer = create_vertex_buffer(
        gpu,
        vertices,
        6);

    command_buffer = mygpu_command_buffer_create(128);
    assert(command_buffer != NULL);

    command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    command.vertex_address = mygpu_buffer_address(vertex_buffer);
    command.vertex_count = 3;

    assert(mygpu_command_buffer_write(
        command_buffer,
        &command,
        sizeof(command)) == 0);

    command.vertex_address =
        mygpu_buffer_address(vertex_buffer) +
        (3u * (uint32_t)sizeof(struct mygpu_vertex));

    assert(mygpu_command_buffer_write(
        command_buffer,
        &command,
        sizeof(command)) == 0);

    assert(mygpu_command_buffer_validate(command_buffer) == 0);

    assert(mygpu_command_buffer_execute(
        gpu,
        command_buffer) == 0);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        3,
        3,
        &color) == 0);

    assert(color == 0xff0000ffu);

    assert(mygpu_framebuffer_get_pixel(
        mygpu_get_framebuffer(gpu),
        21,
        21,
        &color) == 0);

    assert(color == 0xff00ff00u);

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

int main(void)
{
    test_basic_triangle();
    test_reverse_winding();
    test_triangle_at_framebuffer_edge();
    test_triangle_outside_framebuffer();
    test_degenerate_triangle();
    test_multiple_triangles();

    printf("draw triangles tests passed\n");

    return 0;
}