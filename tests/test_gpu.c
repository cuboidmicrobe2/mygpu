#include <stdio.h>

#include "mygpu/gpu.h"
#include "../gpu/commands.h"
#include "../gpu/fence.h"

static int tests_run = 0;
static int tests_passed = 0;

static void check(int condition, const char *name)
{
    tests_run++;

    if (condition) {
        printf("PASS: %s\n", name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", name);
    }
}

static void test_create(void)
{
    struct mygpu *gpu;

    gpu = mygpu_create();

    check(
        gpu != NULL,
        "create GPU"
    );

    mygpu_destroy(gpu);
}

static void test_destroy_null(void)
{
    mygpu_destroy(NULL);

    check(
        1,
        "destroy NULL GPU safely"
    );
}

static void test_reset(void)
{
    struct mygpu *gpu;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "reset test setup");
        return;
    }

    mygpu_reset(gpu);

    check(
        1,
        "reset GPU"
    );

    mygpu_destroy(gpu);
}

static void test_multiple_gpus(void)
{
    struct mygpu *gpu1;
    struct mygpu *gpu2;

    gpu1 = mygpu_create();
    gpu2 = mygpu_create();

    check(
        gpu1 != NULL,
        "create first GPU"
    );

    check(
        gpu2 != NULL,
        "create second GPU"
    );

    mygpu_destroy(gpu1);
    mygpu_destroy(gpu2);
}

static void test_multiple_resets(void)
{
    struct mygpu *gpu;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "multiple reset setup");
        return;
    }

    mygpu_reset(gpu);
    mygpu_reset(gpu);
    mygpu_reset(gpu);

    check(
        1,
        "GPU can be reset multiple times"
    );

    mygpu_destroy(gpu);
}

static void test_clear_integration(void)
{
    struct mygpu *gpu;
    uint32_t color;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "clear integration setup");
        return;
    }

    result = mygpu_clear(
        gpu,
        0xFF0000FF
    );

    check(
        result == 0,
        "GPU clear succeeds"
    );

    result = mygpu_get_pixel(
        gpu,
        10,
        10,
        &color
    );

    check(
        result == 0,
        "GPU can read framebuffer pixel"
    );

    check(
        color == 0xFF0000FF,
        "GPU clear reaches framebuffer"
    );

    mygpu_destroy(gpu);
}

static void check_pixel(
    struct mygpu *gpu,
    uint32_t x,
    uint32_t y,
    uint32_t expected,
    const char *name
)
{
    uint32_t color;
    int result;

    result = mygpu_get_pixel(
        gpu,
        x,
        y,
        &color
    );

    check(
        result == 0 && color == expected,
        name
    );
}

static void test_submit(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_cmd_clear command;

    int result;

    gpu = mygpu_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(100);

    if (gpu == NULL || buffer == NULL || fence == NULL) {
        check(0, "submit test setup");

        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    command.opcode = MYGPU_CMD_CLEAR;
    command.color = 0xFF0000FF;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write command for GPU submit"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "GPU submit fence initially unsignaled"
    );

    result = mygpu_submit(
        gpu,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit command through GPU"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "GPU submit does not execute immediately"
    );

    result = mygpu_process(gpu);

    check(
        result == 0,
        "process submitted GPU command"
    );

    check_pixel(
        gpu,
        10,
        10,
        0xFF0000FF,
        "GPU submit command executes"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "GPU submit fence signals after execution"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_submit_invalid_arguments(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;

    gpu = mygpu_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(1);

    if (gpu == NULL || buffer == NULL || fence == NULL) {
        check(0, "submit invalid argument setup");

        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    check(
        mygpu_submit(NULL, buffer, fence) != 0,
        "reject NULL GPU in submit"
    );

    check(
        mygpu_submit(gpu, NULL, fence) != 0,
        "reject NULL command buffer in submit"
    );

    check(
        mygpu_submit(gpu, buffer, NULL) != 0,
        "reject NULL fence in submit"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_submit_invalid_command(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;

    uint32_t invalid_opcode = 0xFFFFFFFFu;

    int result;

    gpu = mygpu_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(2);

    if (gpu == NULL || buffer == NULL || fence == NULL) {
        check(0, "submit invalid command setup");

        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    result = mygpu_command_buffer_write(
        buffer,
        &invalid_opcode,
        sizeof(invalid_opcode)
    );

    check(
        result == 0,
        "write invalid command for GPU submit"
    );

    result = mygpu_submit(
        gpu,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit invalid command through GPU"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "invalid command fence initially unsignaled"
    );

    result = mygpu_process(gpu);

    check(
        result != 0,
        "GPU process rejects invalid submitted command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "invalid submitted command fence remains unsignaled"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_submit_fifo(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer1;
    struct mygpu_command_buffer *buffer2;
    struct mygpu_fence *fence1;
    struct mygpu_fence *fence2;

    struct mygpu_cmd_clear command1;
    struct mygpu_cmd_clear command2;

    int result;

    gpu = mygpu_create();
    buffer1 = mygpu_command_buffer_create(64);
    buffer2 = mygpu_command_buffer_create(64);
    fence1 = mygpu_fence_create(1);
    fence2 = mygpu_fence_create(2);

    if (gpu == NULL ||
        buffer1 == NULL ||
        buffer2 == NULL ||
        fence1 == NULL ||
        fence2 == NULL) {

        check(0, "GPU submit FIFO setup");

        mygpu_command_buffer_destroy(buffer1);
        mygpu_command_buffer_destroy(buffer2);
        mygpu_fence_destroy(fence1);
        mygpu_fence_destroy(fence2);
        mygpu_destroy(gpu);

        return;
    }

    command1.opcode = MYGPU_CMD_CLEAR;
    command1.color = 0xFF0000FF;

    command2.opcode = MYGPU_CMD_CLEAR;
    command2.color = 0x00FF00FF;

    result = mygpu_command_buffer_write(
        buffer1,
        &command1,
        sizeof(command1)
    );

    check(
        result == 0,
        "write first GPU submit FIFO command"
    );

    result = mygpu_command_buffer_write(
        buffer2,
        &command2,
        sizeof(command2)
    );

    check(
        result == 0,
        "write second GPU submit FIFO command"
    );

    result = mygpu_submit(
        gpu,
        buffer1,
        fence1
    );

    check(
        result == 0,
        "submit first GPU FIFO command"
    );

    result = mygpu_submit(
        gpu,
        buffer2,
        fence2
    );

    check(
        result == 0,
        "submit second GPU FIFO command"
    );

    check(
        mygpu_fence_is_signaled(fence1) == 0,
        "first GPU FIFO fence initially unsignaled"
    );

    check(
        mygpu_fence_is_signaled(fence2) == 0,
        "second GPU FIFO fence initially unsignaled"
    );

    result = mygpu_process(gpu);

    check(
        result == 0,
        "process GPU FIFO submissions"
    );

    check_pixel(
        gpu,
        50,
        50,
        0x00FF00FF,
        "GPU submit preserves FIFO order"
    );

    check(
        mygpu_fence_is_signaled(fence1) == 1,
        "first GPU FIFO fence signals"
    );

    check(
        mygpu_fence_is_signaled(fence2) == 1,
        "second GPU FIFO fence signals"
    );

    mygpu_command_buffer_destroy(buffer1);
    mygpu_command_buffer_destroy(buffer2);
    mygpu_fence_destroy(fence1);
    mygpu_fence_destroy(fence2);
    mygpu_destroy(gpu);
}

static void test_fence_wait(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_cmd_clear command;

    int result;

    gpu = mygpu_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(100);

    if (gpu == NULL || buffer == NULL || fence == NULL) {
        check(0, "fence wait setup");

        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    command.opcode = MYGPU_CMD_CLEAR;
    command.color = 0xFF0000FF;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write command for fence wait"
    );

    result = mygpu_submit(
        gpu,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit command for fence wait"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "wait fence initially unsignaled"
    );

    result = mygpu_fence_wait(
        gpu,
        fence
    );

    check(
        result == 0,
        "wait for submitted fence"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "wait signals fence"
    );

    check_pixel(
        gpu,
        10,
        10,
        0xFF0000FF,
        "wait executes submitted command"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_fence_wait_already_signaled(void)
{
    struct mygpu *gpu;
    struct mygpu_fence *fence;

    int result;

    gpu = mygpu_create();
    fence = mygpu_fence_create(101);

    if (gpu == NULL || fence == NULL) {
        check(0, "already-signaled fence wait setup");

        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    mygpu_fence_signal(fence);

    result = mygpu_fence_wait(
        gpu,
        fence
    );

    check(
        result == 0,
        "wait on already-signaled fence"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "already-signaled fence remains signaled"
    );

    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_fence_wait_invalid_arguments(void)
{
    struct mygpu *gpu;
    struct mygpu_fence *fence;

    gpu = mygpu_create();
    fence = mygpu_fence_create(102);

    if (gpu == NULL || fence == NULL) {
        check(0, "fence wait invalid argument setup");

        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    check(
        mygpu_fence_wait(NULL, fence) != 0,
        "reject NULL GPU in fence wait"
    );

    check(
        mygpu_fence_wait(gpu, NULL) != 0,
        "reject NULL fence in fence wait"
    );

    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_fence_reuse(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_cmd_clear command;
    int result;

    gpu = mygpu_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(200);

    if (gpu == NULL || buffer == NULL || fence == NULL) {
        check(0, "fence reuse setup");

        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    command.opcode = MYGPU_CMD_CLEAR;
    command.color = 0x11223344;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write first fence reuse command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "reuse fence initially unsignaled"
    );

    result = mygpu_submit(gpu, buffer, fence);

    check(
        result == 0,
        "submit first fence reuse command"
    );

    result = mygpu_process(gpu);

    check(
        result == 0,
        "process first fence reuse command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "reuse fence signals after first process"
    );

    mygpu_fence_reset(fence);

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "reuse fence is unsignaled after reset"
    );

    command.color = 0x55667788;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write second fence reuse command"
    );

    result = mygpu_submit(gpu, buffer, fence);

    check(
        result == 0,
        "submit second fence reuse command"
    );

    result = mygpu_process(gpu);

    check(
        result == 0,
        "process second fence reuse command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "reuse fence signals after second process"
    );

    check_pixel(
        gpu,
        0,
        0,
        0x55667788,
        "second fence reuse command executes"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

int main(void)
{
    printf("=== MyGPU Tests ===\n\n");

    test_create();
    test_destroy_null();
    test_reset();
    test_multiple_gpus();
    test_multiple_resets();
    test_clear_integration();
    test_submit();
    test_submit_invalid_arguments();
    test_submit_invalid_command();
    test_submit_fifo();
    test_fence_wait();
    test_fence_wait_already_signaled();
    test_fence_wait_invalid_arguments();
    test_fence_reuse();

    printf("\n=== Results ===\n");

    printf(
        "%d/%d tests passed\n",
        tests_passed,
        tests_run
    );

    if (tests_passed == tests_run) {
        printf("All tests passed!\n");
        return 0;
    }

    printf("Some tests failed!\n");
    return 1;
}