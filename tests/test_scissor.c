#include <stdint.h>
#include <stdio.h>

#include "mygpu/buffer.h"
#include "mygpu/commands.h"
#include "mygpu/framebuffer.h"
#include "mygpu/gpu.h"
#include "mygpu/rasterizer.h"
#include "mygpu/vertex.h"

#include "test.h"

#define BACKGROUND 0x12345678u
#define TRIANGLE 0xff0000ffu

/* One triangle covering the square from (0, 0) to (40, 40) and more. */
static struct mygpu_buffer *create_big_triangle(struct mygpu *gpu)
{
    struct mygpu_vertex vertices[3] = {{0.0f, 0.0f, TRIANGLE}, {80.0f, 0.0f, TRIANGLE}, {0.0f, 80.0f, TRIANGLE}};
    struct mygpu_buffer *buffer;

    buffer = mygpu_buffer_create(gpu, sizeof(vertices));
    require(buffer != NULL);
    require(mygpu_buffer_write(buffer, 0, vertices, sizeof(vertices)) == 0);

    return buffer;
}

static void write_clear(struct mygpu_command_buffer *command_buffer)
{
    struct mygpu_cmd_clear command = {MYGPU_CMD_CLEAR, BACKGROUND};

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);
}

static void write_scissor(struct mygpu_command_buffer *command_buffer, uint32_t x, uint32_t y, uint32_t width,
                          uint32_t height)
{
    struct mygpu_cmd_set_scissor command = {MYGPU_CMD_SET_SCISSOR, x, y, width, height};

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);
}

static void write_draw(struct mygpu_command_buffer *command_buffer, struct mygpu_buffer *vertex_buffer)
{
    struct mygpu_cmd_draw_triangles command = {MYGPU_CMD_DRAW_TRIANGLES, mygpu_buffer_address(vertex_buffer), 0, 3, 0};

    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command)) == 0);
}

static uint32_t pixel(struct mygpu *gpu, uint32_t x, uint32_t y)
{
    uint32_t color;

    require(mygpu_framebuffer_get_pixel(mygpu_get_framebuffer(gpu), x, y, &color) == 0);

    return color;
}

static void test_draw_limited_to_scissor(void)
{
    struct mygpu *gpu = mygpu_create();
    require(gpu != NULL);

    struct mygpu_buffer *vertex_buffer = create_big_triangle(gpu);
    struct mygpu_command_buffer *command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    write_clear(command_buffer);
    write_scissor(command_buffer, 10, 10, 10, 10);
    write_draw(command_buffer, vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    check(pixel(gpu, 10, 10) == TRIANGLE, "top-left pixel inside scissor is drawn");
    check(pixel(gpu, 15, 15) == TRIANGLE, "pixel inside scissor is drawn");
    check(pixel(gpu, 19, 19) == TRIANGLE, "bottom-right pixel inside scissor is drawn");
    check(pixel(gpu, 5, 5) == BACKGROUND, "pixel above-left of scissor is untouched");
    check(pixel(gpu, 20, 15) == BACKGROUND, "pixel right of scissor is untouched");
    check(pixel(gpu, 15, 20) == BACKGROUND, "pixel below scissor is untouched");

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_scissor_resets_between_executions(void)
{
    struct mygpu *gpu = mygpu_create();
    require(gpu != NULL);

    struct mygpu_buffer *vertex_buffer = create_big_triangle(gpu);
    struct mygpu_command_buffer *first = mygpu_command_buffer_create(64);
    struct mygpu_command_buffer *second = mygpu_command_buffer_create(64);
    require(first != NULL);
    require(second != NULL);

    write_clear(first);
    write_scissor(first, 10, 10, 10, 10);
    require(mygpu_command_buffer_execute(gpu, first) == 0);

    write_draw(second, vertex_buffer);
    require(mygpu_command_buffer_execute(gpu, second) == 0);

    check(pixel(gpu, 30, 30) == TRIANGLE, "scissor from an earlier execution no longer applies");

    mygpu_command_buffer_destroy(second);
    mygpu_command_buffer_destroy(first);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_oversized_scissor_is_clamped(void)
{
    struct mygpu *gpu = mygpu_create();
    require(gpu != NULL);

    struct mygpu_buffer *vertex_buffer = create_big_triangle(gpu);
    struct mygpu_command_buffer *command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    write_clear(command_buffer);
    write_scissor(command_buffer, 5, 5, UINT32_MAX, UINT32_MAX);
    write_draw(command_buffer, vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    check(pixel(gpu, 30, 30) == TRIANGLE, "x + width overflowing 32 bits still draws");
    check(pixel(gpu, 2, 2) == BACKGROUND, "pixel before scissor start is untouched");

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_empty_scissor_draws_nothing(void)
{
    struct mygpu *gpu = mygpu_create();
    require(gpu != NULL);

    struct mygpu_buffer *vertex_buffer = create_big_triangle(gpu);
    struct mygpu_command_buffer *command_buffer = mygpu_command_buffer_create(128);
    require(command_buffer != NULL);

    write_clear(command_buffer);
    write_scissor(command_buffer, 10, 10, 0, 10);
    write_draw(command_buffer, vertex_buffer);

    require(mygpu_command_buffer_execute(gpu, command_buffer) == 0);

    check(pixel(gpu, 10, 10) == BACKGROUND, "zero-width scissor draws nothing");

    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);
}

static void test_truncated_scissor_rejected(void)
{
    struct mygpu_command_buffer *command_buffer = mygpu_command_buffer_create(64);
    struct mygpu_cmd_set_scissor command = {MYGPU_CMD_SET_SCISSOR, 0, 0, 10, 10};

    require(command_buffer != NULL);
    require(mygpu_command_buffer_write(command_buffer, &command, sizeof(command) - 1) == 0);

    check(mygpu_command_buffer_validate(command_buffer) != 0, "truncated SET_SCISSOR is rejected");

    mygpu_command_buffer_destroy(command_buffer);
}

static void test_rasterizer_clip_rect(void)
{
    struct mygpu_framebuffer *framebuffer = mygpu_framebuffer_create(16, 16);
    struct mygpu_vertex v0 = {0.0f, 0.0f, TRIANGLE};
    struct mygpu_vertex v1 = {32.0f, 0.0f, TRIANGLE};
    struct mygpu_vertex v2 = {0.0f, 32.0f, TRIANGLE};
    struct mygpu_rect clip = {4, 4, 4, 4};
    uint32_t color;

    require(framebuffer != NULL);
    mygpu_framebuffer_clear(framebuffer, BACKGROUND);

    require(mygpu_rasterize_triangle(framebuffer, &clip, &v0, &v1, &v2) == 0);

    require(mygpu_framebuffer_get_pixel(framebuffer, 5, 5, &color) == 0);
    check(color == TRIANGLE, "rasterizer draws inside clip rect");

    require(mygpu_framebuffer_get_pixel(framebuffer, 8, 5, &color) == 0);
    check(color == BACKGROUND, "rasterizer skips pixels right of clip rect");

    require(mygpu_rasterize_triangle(framebuffer, NULL, &v0, &v1, &v2) != 0);

    mygpu_framebuffer_destroy(framebuffer);
}

int main(void)
{
    run_test(test_draw_limited_to_scissor);
    run_test(test_scissor_resets_between_executions);
    run_test(test_oversized_scissor_is_clamped);
    run_test(test_empty_scissor_draws_nothing);
    run_test(test_truncated_scissor_rejected);
    run_test(test_rasterizer_clip_rect);

    return test_finish();
}
