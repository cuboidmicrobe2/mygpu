#include <stdint.h>
#include <stdio.h>

#include "../include/mygpu/gpu.h"
#include "../gpu/commands.h"
#include "../gpu/fence.h"
#include "../gpu/queue.h"

#define TEST_CLEAR_COLOR 0x000000FFu

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

static void test_create_destroy(void)
{
    struct mygpu_queue *queue;

    queue = mygpu_queue_create();

    check(
        queue != NULL,
        "create queue"
    );

    mygpu_queue_destroy(queue);

    check(
        1,
        "destroy queue"
    );
}

static void test_invalid_arguments(void)
{
    struct mygpu_queue *queue;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu *gpu;

    queue = mygpu_queue_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(1);
    gpu = mygpu_create();

    if (queue == NULL ||
        buffer == NULL ||
        fence == NULL ||
        gpu == NULL) {

        check(0, "invalid argument queue setup");

        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);
        mygpu_destroy(gpu);

        return;
    }

    check(
        mygpu_queue_submit(NULL, buffer, fence) != 0,
        "reject NULL queue"
    );

    check(
        mygpu_queue_submit(queue, NULL, fence) != 0,
        "reject NULL command buffer"
    );

    check(
        mygpu_queue_submit(queue, buffer, NULL) != 0,
        "reject NULL fence"
    );

    check(
        mygpu_queue_process(NULL, queue) != 0,
        "reject NULL GPU"
    );

    check(
        mygpu_queue_process(gpu, NULL) != 0,
        "reject NULL queue"
    );

    mygpu_queue_destroy(queue);
    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_destroy(gpu);
}

static void test_submit_and_process_clear(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_cmd_clear command;

    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(10);

    if (gpu == NULL ||
        queue == NULL ||
        buffer == NULL ||
        fence == NULL) {

        check(0, "CLEAR queue setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);

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
        "write CLEAR command for queue"
    );

    result = mygpu_queue_submit(
        queue,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit command buffer"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "fence initially unsignaled"
    );

    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result == 0,
        "process queue"
    );

    check_pixel(
        gpu,
        10,
        10,
        0xFF0000FF,
        "queued CLEAR changes framebuffer"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "fence signals after CLEAR"
    );

    check(
        mygpu_fence_id(fence) == 10,
        "queued fence retains ID"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_fifo_order(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;

    struct mygpu_command_buffer *buffer1;
    struct mygpu_command_buffer *buffer2;

    struct mygpu_fence *fence1;
    struct mygpu_fence *fence2;

    struct mygpu_cmd_clear command1;
    struct mygpu_cmd_clear command2;

    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();

    buffer1 = mygpu_command_buffer_create(64);
    buffer2 = mygpu_command_buffer_create(64);

    fence1 = mygpu_fence_create(1);
    fence2 = mygpu_fence_create(2);

    if (gpu == NULL ||
        queue == NULL ||
        buffer1 == NULL ||
        buffer2 == NULL ||
        fence1 == NULL ||
        fence2 == NULL) {

        check(0, "FIFO queue setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer1);
        mygpu_command_buffer_destroy(buffer2);
        mygpu_fence_destroy(fence1);
        mygpu_fence_destroy(fence2);

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
        "write first FIFO command"
    );

    result = mygpu_command_buffer_write(
        buffer2,
        &command2,
        sizeof(command2)
    );

    check(
        result == 0,
        "write second FIFO command"
    );

    result = mygpu_queue_submit(
        queue,
        buffer1,
        fence1
    );

    check(
        result == 0,
        "submit first FIFO command"
    );

    result = mygpu_queue_submit(
        queue,
        buffer2,
        fence2
    );

    check(
        result == 0,
        "submit second FIFO command"
    );

    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result == 0,
        "process FIFO commands"
    );

    /*
     * The second CLEAR should execute after
     * the first CLEAR, so the final color
     * should be command2's color.
     */
    check_pixel(
        gpu,
        50,
        50,
        0x00FF00FF,
        "FIFO executes commands in submission order"
    );

    check(
        mygpu_fence_is_signaled(fence1) == 1,
        "first FIFO fence signals"
    );

    check(
        mygpu_fence_is_signaled(fence2) == 1,
        "second FIFO fence signals"
    );

    mygpu_command_buffer_destroy(buffer1);
    mygpu_command_buffer_destroy(buffer2);

    mygpu_fence_destroy(fence1);
    mygpu_fence_destroy(fence2);

    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_empty_queue(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;

    gpu = mygpu_create();
    queue = mygpu_queue_create();

    if (gpu == NULL || queue == NULL) {
        check(0, "empty queue setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);

        return;
    }

    check(
        mygpu_queue_process(gpu, queue) == 0,
        "process empty queue"
    );

    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_present_fence(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;

    uint32_t opcode;
    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(20);

    if (gpu == NULL ||
        queue == NULL ||
        buffer == NULL ||
        fence == NULL) {

        check(0, "PRESENT queue setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);

        return;
    }

    /*
     * PRESENT has no additional data.
     * The command consists only of its opcode.
     */
    opcode = MYGPU_CMD_PRESENT;

    result = mygpu_command_buffer_write(
        buffer,
        &opcode,
        sizeof(opcode)
    );

    check(
        result == 0,
        "write queued PRESENT command"
    );

    result = mygpu_queue_submit(
        queue,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit PRESENT command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "PRESENT fence initially unsignaled"
    );

    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result == 0,
        "process queued PRESENT command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "queued PRESENT signals fence"
    );

    /*
     * PRESENT should also update the GPU's
     * presentation state.
     */
    check(
        mygpu_is_presented(gpu) != 0,
        "queued PRESENT marks framebuffer presented"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_invalid_command_fence(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;

    uint32_t invalid_opcode;
    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(30);

    if (gpu == NULL ||
        queue == NULL ||
        buffer == NULL ||
        fence == NULL) {

        check(0, "invalid command fence setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);

        return;
    }

    invalid_opcode = 0xFFFFFFFFu;

    result = mygpu_command_buffer_write(
        buffer,
        &invalid_opcode,
        sizeof(invalid_opcode)
    );

    check(
        result == 0,
        "write invalid queued command"
    );

    result = mygpu_queue_submit(
        queue,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit invalid command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "invalid command fence initially unsignaled"
    );

    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result != 0,
        "reject invalid queued command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "invalid command fence remains unsignaled"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);
    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_failed_command_stops_queue(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;

    struct mygpu_command_buffer *bad_buffer;
    struct mygpu_command_buffer *good_buffer;

    struct mygpu_fence *bad_fence;
    struct mygpu_fence *good_fence;

    struct mygpu_cmd_clear clear_command;
    uint32_t invalid_opcode;

    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();

    bad_buffer = mygpu_command_buffer_create(64);
    good_buffer = mygpu_command_buffer_create(64);

    bad_fence = mygpu_fence_create(40);
    good_fence = mygpu_fence_create(41);

    if (gpu == NULL ||
        queue == NULL ||
        bad_buffer == NULL ||
        good_buffer == NULL ||
        bad_fence == NULL ||
        good_fence == NULL) {

        check(0, "failed queue setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(bad_buffer);
        mygpu_command_buffer_destroy(good_buffer);
        mygpu_fence_destroy(bad_fence);
        mygpu_fence_destroy(good_fence);

        return;
    }

    /*
     * Create an invalid command.
     */
    invalid_opcode = 0xFFFFFFFFu;

    result = mygpu_command_buffer_write(
        bad_buffer,
        &invalid_opcode,
        sizeof(invalid_opcode)
    );

    check(
        result == 0,
        "write failed queue command"
    );

    /*
     * Create a valid CLEAR command.
     */
    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0xFF0000FFu;

    result = mygpu_command_buffer_write(
        good_buffer,
        &clear_command,
        sizeof(clear_command)
    );

    check(
        result == 0,
        "write command after failed command"
    );

    /*
     * Submit the invalid command first.
     */
    result = mygpu_queue_submit(
        queue,
        bad_buffer,
        bad_fence
    );

    check(
        result == 0,
        "submit failed command first"
    );

    /*
     * Submit the valid command second.
     */
    result = mygpu_queue_submit(
        queue,
        good_buffer,
        good_fence
    );

    check(
        result == 0,
        "submit valid command second"
    );

    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result != 0,
        "failed command stops queue"
    );

    /*
     * Neither command should have completed.
     */
    check(
        mygpu_fence_is_signaled(bad_fence) == 0,
        "failed command fence remains unsignaled"
    );

    check(
        mygpu_fence_is_signaled(good_fence) == 0,
        "following command fence remains unsignaled"
    );

    /*
     * The CLEAR command should not have executed.
     * A newly created GPU contains TEST_CLEAR_COLOR.
     */
    check_pixel(
        gpu,
        50,
        50,
        TEST_CLEAR_COLOR,
        "command after failure does not execute"
    );

    mygpu_command_buffer_destroy(bad_buffer);
    mygpu_command_buffer_destroy(good_buffer);

    mygpu_fence_destroy(bad_fence);
    mygpu_fence_destroy(good_fence);

    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_queue_recovery_after_failure(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;

    struct mygpu_command_buffer *bad_buffer;
    struct mygpu_command_buffer *good_buffer;

    struct mygpu_fence *bad_fence;
    struct mygpu_fence *good_fence;

    struct mygpu_cmd_clear clear_command;
    uint32_t invalid_opcode;

    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();

    bad_buffer = mygpu_command_buffer_create(64);
    good_buffer = mygpu_command_buffer_create(64);

    bad_fence = mygpu_fence_create(50);
    good_fence = mygpu_fence_create(51);

    if (gpu == NULL ||
        queue == NULL ||
        bad_buffer == NULL ||
        good_buffer == NULL ||
        bad_fence == NULL ||
        good_fence == NULL) {

        check(0, "queue recovery setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(bad_buffer);
        mygpu_command_buffer_destroy(good_buffer);
        mygpu_fence_destroy(bad_fence);
        mygpu_fence_destroy(good_fence);

        return;
    }

    /*
     * Create an invalid command.
     */
    invalid_opcode = 0xFFFFFFFFu;

    result = mygpu_command_buffer_write(
        bad_buffer,
        &invalid_opcode,
        sizeof(invalid_opcode)
    );

    check(
        result == 0,
        "write recovery invalid command"
    );

    /*
     * Create a valid CLEAR command.
     */
    clear_command.opcode = MYGPU_CMD_CLEAR;
    clear_command.color = 0xFF0000FFu;

    result = mygpu_command_buffer_write(
        good_buffer,
        &clear_command,
        sizeof(clear_command)
    );

    check(
        result == 0,
        "write recovery valid command"
    );

    /*
     * Submit the invalid command first.
     */
    check(
        mygpu_queue_submit(
            queue,
            bad_buffer,
            bad_fence
        ) == 0,
        "submit recovery invalid command"
    );

    /*
     * Submit the valid command second.
     */
    check(
        mygpu_queue_submit(
            queue,
            good_buffer,
            good_fence
        ) == 0,
        "submit recovery valid command"
    );

    /*
     * The first command should fail.
     */
    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result != 0,
        "recovery process reports failure"
    );

    /*
     * The failed command must not signal its fence.
     */
    check(
        mygpu_fence_is_signaled(bad_fence) == 0,
        "failed recovery fence remains unsignaled"
    );

    /*
     * The failed command was removed from the queue,
     * so the valid command can now be processed.
     */
    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result == 0,
        "queue recovers after failed command"
    );

    check(
        mygpu_fence_is_signaled(good_fence) == 1,
        "recovery command fence signals"
    );

    check_pixel(
        gpu,
        50,
        50,
        0xFF0000FFu,
        "recovery command executes"
    );

    mygpu_command_buffer_destroy(bad_buffer);
    mygpu_command_buffer_destroy(good_buffer);

    mygpu_fence_destroy(bad_fence);
    mygpu_fence_destroy(good_fence);

    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

static void test_queue_does_not_own_resources(void)
{
    struct mygpu *gpu;
    struct mygpu_queue *queue;
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_cmd_clear command;

    int result;

    gpu = mygpu_create();
    queue = mygpu_queue_create();
    buffer = mygpu_command_buffer_create(64);
    fence = mygpu_fence_create(60);

    if (gpu == NULL ||
        queue == NULL ||
        buffer == NULL ||
        fence == NULL) {

        check(0, "queue ownership setup");

        mygpu_destroy(gpu);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(buffer);
        mygpu_fence_destroy(fence);

        return;
    }

    command.opcode = MYGPU_CMD_CLEAR;
    command.color = 0x12345678u;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write ownership test command"
    );

    result = mygpu_queue_submit(
        queue,
        buffer,
        fence
    );

    check(
        result == 0,
        "submit ownership test command"
    );

    /*
     * Process the queue while the caller still owns
     * the command buffer and fence.
     */
    result = mygpu_queue_process(
        gpu,
        queue
    );

    check(
        result == 0,
        "process ownership test command"
    );

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "ownership test fence signals"
    );

    check_pixel(
        gpu,
        10,
        10,
        0x12345678u,
        "ownership test command executes"
    );

    /*
     * The queue must not destroy either resource.
     * The caller remains responsible for cleanup.
     */
    mygpu_command_buffer_destroy(buffer);
    mygpu_fence_destroy(fence);

    check(
        1,
        "caller destroys submitted resources"
    );

    mygpu_queue_destroy(queue);
    mygpu_destroy(gpu);
}

int main(void)
{
    printf("=== MyGPU Queue Tests ===\n\n");

    test_create_destroy();
    test_invalid_arguments();
    test_submit_and_process_clear();
    test_fifo_order();
    test_empty_queue();
    test_present_fence();
    test_invalid_command_fence();
    test_failed_command_stops_queue();
    test_queue_recovery_after_failure();
    test_queue_does_not_own_resources();

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