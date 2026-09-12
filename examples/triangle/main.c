#include <stdio.h>
#include <stdint.h>

#include "mygpu/gpu.h"
#include "mygpu/buffer.h"
#include "mygpu/commands.h"
#include "mygpu/vertex.h"
#include "mygpu/queue.h"
#include "mygpu/fence.h"
#include "mygpu/framebuffer.h"

int main(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *vertex_buffer;
    struct mygpu_command_buffer *command_buffer;
    struct mygpu_queue *queue;
    struct mygpu_fence *fence;
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;

    struct mygpu_vertex vertices[3] = {
        {
            .x = 2.0f,
            .y = 2.0f,
            .color = 0xFFFFFFFFu
        },
        {
            .x = 10.0f,
            .y = 2.0f,
            .color = 0xFFFFFFFFu
        },
        {
            .x = 6.0f,
            .y = 8.0f,
            .color = 0xFFFFFFFFu
        }
    };

    struct mygpu_cmd_draw_triangles draw_command;

    gpu = mygpu_create();

    if (gpu == NULL) {
        fprintf(stderr, "failed to create GPU\n");
        return 1;
    }

    vertex_buffer = mygpu_buffer_create(
        gpu,
        sizeof(vertices));

    if (vertex_buffer == NULL) {
        fprintf(stderr, "failed to create vertex buffer\n");

        mygpu_destroy(gpu);
        return 1;
    }

    if (mygpu_buffer_write(
            vertex_buffer,
            0,
            vertices,
            sizeof(vertices)) != 0) {

        fprintf(stderr, "failed to write vertex buffer\n");

        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    command_buffer = mygpu_command_buffer_create(64);

    if (command_buffer == NULL) {
        fprintf(stderr, "failed to create command buffer\n");

        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    draw_command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
    draw_command.vertex_address =
        mygpu_buffer_address(vertex_buffer);
    draw_command.vertex_count = 3;

    if (mygpu_command_buffer_write(
            command_buffer,
            &draw_command,
            sizeof(draw_command)) != 0) {

        fprintf(stderr, "failed to write command buffer\n");

        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (mygpu_command_buffer_validate(command_buffer) != 0) {
        fprintf(stderr, "command buffer validation failed\n");

        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    queue = mygpu_queue_create();

    if (queue == NULL) {
        fprintf(stderr, "failed to create queue\n");

        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    fence = mygpu_fence_create(1);

    if (fence == NULL) {
        fprintf(stderr, "failed to create fence\n");

        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (mygpu_queue_submit(
            queue,
            command_buffer,
            fence) != 0) {

        fprintf(stderr, "failed to submit command buffer\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (mygpu_fence_is_signaled(fence)) {
        fprintf(stderr, "fence signaled before queue processing\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (mygpu_queue_process(gpu, queue) != 0) {
        fprintf(stderr, "failed to process queue\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (!mygpu_fence_is_signaled(fence)) {
        fprintf(stderr, "fence was not signaled\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }
    
    framebuffer = mygpu_get_framebuffer(gpu);

    if (framebuffer == NULL) {
        fprintf(stderr, "failed to get framebuffer\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (mygpu_framebuffer_get_pixel(
        framebuffer,
        6,
        4,
        &color) != 0) {

        fprintf(stderr, "failed to read framebuffer pixel\n");

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    if (color != 0xFFFFFFFFu) {
        fprintf(
            stderr, "triangle pixel has wrong color: 0x%08X\n",
            color);

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(command_buffer);
        mygpu_buffer_destroy(vertex_buffer);
        mygpu_destroy(gpu);

        return 1;
    }

    printf("triangle rendered successfully\n");

    mygpu_fence_destroy(fence);
    mygpu_queue_destroy(queue);
    mygpu_command_buffer_destroy(command_buffer);
    mygpu_buffer_destroy(vertex_buffer);
    mygpu_destroy(gpu);

    return 0;
}