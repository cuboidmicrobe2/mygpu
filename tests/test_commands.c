#include <stdint.h>
#include <stdio.h>

#include "../include/mygpu/gpu.h"
#include "../include/mygpu/commands.h"

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
    struct mygpu_command_buffer *buffer;

    buffer = mygpu_command_buffer_create(64);

    check(
        buffer != NULL,
        "create command buffer"
    );

    mygpu_command_buffer_destroy(buffer);

    check(
        1,
        "destroy command buffer"
    );
}

static void test_create_zero_size(void)
{
    struct mygpu_command_buffer *buffer;

    buffer = mygpu_command_buffer_create(0);

    check(
        buffer == NULL,
        "reject zero-size command buffer"
    );
}

static void test_write_command(void)
{
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_clear command;
    int result;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "write command setup");
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
        "write CLEAR command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_write_too_large(void)
{
    struct mygpu_command_buffer *buffer;
    uint8_t data[16];
    int result;

    buffer = mygpu_command_buffer_create(8);

    if (buffer == NULL) {
        check(0, "oversized write setup");
        return;
    }

    result = mygpu_command_buffer_write(
        buffer,
        data,
        sizeof(data)
    );

    check(
        result != 0,
        "reject oversized command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_execute_clear(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_clear command;

    uint32_t color;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "CLEAR GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "CLEAR command buffer setup");
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
        "write CLEAR command for execution"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result == 0,
        "execute CLEAR command"
    );

    result = mygpu_get_pixel(
        gpu,
        10,
        10,
        &color
    );

    check(
        result == 0,
        "read pixel after CLEAR"
    );

    check(
        color == 0xFF0000FF,
        "CLEAR command changes framebuffer"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_invalid_opcode(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "invalid opcode GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "invalid opcode buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    opcode = 0xFFFFFFFF;

    result = mygpu_command_buffer_write(
        buffer,
        &opcode,
        sizeof(opcode)
    );

    check(
        result == 0,
        "write invalid opcode"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result != 0,
        "reject invalid opcode"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_truncated_command(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;

    uint32_t opcode;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "truncated command GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "truncated command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * Write only the opcode.
     *
     * A CLEAR command also requires a color,
     * so the command is incomplete.
     */
    opcode = MYGPU_CMD_CLEAR;

    result = mygpu_command_buffer_write(
        buffer,
        &opcode,
        sizeof(opcode)
    );

    check(
        result == 0,
        "write truncated CLEAR command"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result != 0,
        "reject truncated CLEAR command"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_draw_rect(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect command;

    uint32_t color;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "DRAW_RECT GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "DRAW_RECT command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * Create a 3x2 rectangle starting at (10, 20).
     *
     * The rectangle covers:
     *
     * x = 10, 11, 12
     * y = 20, 21
     */
    command.opcode = MYGPU_CMD_DRAW_RECT;
    command.x = 10;
    command.y = 20;
    command.width = 3;
    command.height = 2;
    command.color = 0xFF0000FF;

    result = mygpu_command_buffer_write(
        buffer,
        &command,
        sizeof(command)
    );

    check(
        result == 0,
        "write DRAW_RECT command"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result == 0,
        "execute DRAW_RECT command"
    );

    /*
     * Check pixels inside the rectangle.
     */
    result = mygpu_get_pixel(
        gpu,
        10,
        20,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT top-left pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        11,
        20,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT middle-top pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        12,
        20,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT top-right pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        10,
        21,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT bottom-left pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        11,
        21,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT bottom-middle pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        12,
        21,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "DRAW_RECT bottom-right pixel"
    );

    /*
     * Check pixels outside the rectangle.
     *
     * The GPU reset state is TEST_CLEAR_COLOR,
     * so pixels outside the rectangle should remain
     * unchanged.
     */
    result = mygpu_get_pixel(
        gpu,
        9,
        20,
        &color
    );

    check(
        result == 0 && color == TEST_CLEAR_COLOR,
        "DRAW_RECT does not modify left pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        13,
        20,
        &color
    );

    check(
        result == 0 && color == TEST_CLEAR_COLOR,
        "DRAW_RECT does not modify right pixel"
    );

    result = mygpu_get_pixel(
        gpu,
        10,
        22,
        &color
    );

    check(
        result == 0 && color == TEST_CLEAR_COLOR,
        "DRAW_RECT does not modify pixel below"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_draw_rect_right_edge(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect command;

    const uint32_t color = 0xFF0000FF;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "right-edge GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "right-edge command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * The framebuffer is 320 pixels wide,
     * so valid X coordinates are 0 through 319.
     *
     * Start the rectangle at X=318 and give it
     * a width of 5:
     *
     *   318 319 320 321 322
     *    |   |   |   |   |
     *    +---+---+---+---+
     *    | V | V | X | X | X
     *
     * Only pixels 318 and 319 are visible.
     */
    command.opcode = MYGPU_CMD_DRAW_RECT;
    command.x = 318;
    command.y = 10;
    command.width = 5;
    command.height = 1;
    command.color = color;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write right-edge DRAW_RECT"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "execute right-edge DRAW_RECT"
    );

    /*
     * Both visible pixels should have been drawn.
     */
    check_pixel(
        gpu,
        318,
        10,
        color,
        "right-edge pixel 318"
    );

    check_pixel(
        gpu,
        319,
        10,
        color,
        "right-edge pixel 319"
    );

    /*
     * Pixels 320 and above are outside the framebuffer,
     * so they cannot be read through the framebuffer API.
     * The successful execution above proves that the
     * out-of-bounds portion was clipped rather than
     * causing the command to fail.
     */

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_draw_rect_bottom_edge(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect command;

    const uint32_t color = 0x00FF00FF;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "bottom-edge GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "bottom-edge command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * The framebuffer is 200 pixels high,
     * so valid Y coordinates are 0 through 199.
     *
     * Start the rectangle at Y=198 and give it
     * a height of 5:
     *
     *   198 199 200 201 202
     *    |   |   |   |   |
     *    +---+---+---+---+
     *    | V | V | X | X | X
     *
     * Only pixels 198 and 199 are visible.
     */
    command.opcode = MYGPU_CMD_DRAW_RECT;
    command.x = 10;
    command.y = 198;
    command.width = 1;
    command.height = 5;
    command.color = color;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write bottom-edge DRAW_RECT"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "execute bottom-edge DRAW_RECT"
    );

    /*
     * Both visible pixels should have been drawn.
     */
    check_pixel(
        gpu,
        10,
        198,
        color,
        "bottom-edge pixel 198"
    );

    check_pixel(
        gpu,
        10,
        199,
        color,
        "bottom-edge pixel 199"
    );

    /*
     * Pixels at Y=200 and beyond are outside the framebuffer,
     * so they cannot be read through the framebuffer API.
     * Successful execution proves that the out-of-bounds
     * portion was clipped.
     */

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_draw_rect_outside(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect command;

    const uint32_t background = TEST_CLEAR_COLOR;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "outside-rectangle GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "outside-rectangle command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * The framebuffer is 320 pixels wide,
     * so X=320 is completely outside the framebuffer.
     *
     * The rectangle therefore has no visible pixels.
     */
    command.opcode = MYGPU_CMD_DRAW_RECT;
    command.x = 320;
    command.y = 10;
    command.width = 10;
    command.height = 10;
    command.color = 0xFFFFFFFF;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write outside DRAW_RECT"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "ignore completely outside DRAW_RECT"
    );

    /*
     * A pixel inside the framebuffer should remain
     * in the GPU's reset state.
     */
    check_pixel(
        gpu,
        100,
        100,
        background,
        "outside rectangle does not modify framebuffer"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_copy(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect draw;
    struct mygpu_cmd_copy copy;

    const uint32_t source_color = 0xFF0000FFu;
    uint32_t color;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "COPY GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(128);

    if (buffer == NULL) {
        check(0, "COPY command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * First draw a 3x2 source rectangle at (10, 20).
     *
     * This gives COPY known source pixels to work with.
     */
    draw.opcode = MYGPU_CMD_DRAW_RECT;
    draw.x = 10;
    draw.y = 20;
    draw.width = 3;
    draw.height = 2;
    draw.color = source_color;

    result = mygpu_command_buffer_write(
        buffer,
        &draw,
        sizeof(draw)
    );

    check(
        result == 0,
        "write source DRAW_RECT for COPY"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result == 0,
        "draw source rectangle for COPY"
    );

    /*
     * Create a COPY command that moves the 3x2 rectangle
     * from (10, 20) to (20, 30).
     */
    copy.opcode = MYGPU_CMD_COPY;
    copy.src_x = 10;
    copy.src_y = 20;
    copy.dst_x = 20;
    copy.dst_y = 30;
    copy.width = 3;
    copy.height = 2;

    /*
     * The command buffer was already executed, so create
     * a new buffer for the COPY command.
     */
    mygpu_command_buffer_destroy(buffer);

    buffer = mygpu_command_buffer_create(128);

    if (buffer == NULL) {
        check(0, "COPY execution buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    result = mygpu_command_buffer_write(
        buffer,
        &copy,
        sizeof(copy)
    );

    check(
        result == 0,
        "write COPY command"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result == 0,
        "execute COPY command"
    );

    /*
     * Verify every pixel in the destination rectangle.
     */
    check_pixel(
        gpu,
        20,
        30,
        source_color,
        "COPY destination top-left pixel"
    );

    check_pixel(
        gpu,
        21,
        30,
        source_color,
        "COPY destination middle-top pixel"
    );

    check_pixel(
        gpu,
        22,
        30,
        source_color,
        "COPY destination top-right pixel"
    );

    check_pixel(
        gpu,
        20,
        31,
        source_color,
        "COPY destination bottom-left pixel"
    );

    check_pixel(
        gpu,
        21,
        31,
        source_color,
        "COPY destination bottom-middle pixel"
    );

    check_pixel(
        gpu,
        22,
        31,
        source_color,
        "COPY destination bottom-right pixel"
    );

    /*
     * Verify that the original source rectangle was not
     * modified by the COPY operation.
     */
    result = mygpu_get_pixel(
        gpu,
        10,
        20,
        &color
    );

    check(
        result == 0 && color == source_color,
        "COPY does not modify source"
    );

    /*
     * Verify that a pixel immediately outside the destination
     * rectangle was not modified.
     */
    check_pixel(
        gpu,
        23,
        30,
        TEST_CLEAR_COLOR,
        "COPY does not modify pixel outside destination"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_copy_overlap(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect draw;
    struct mygpu_cmd_copy copy;

    const uint32_t color = 0x00FF00FFu;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "COPY overlap GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(128);

    if (buffer == NULL) {
        check(0, "COPY overlap command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * Create a 4x1 source rectangle:
     *
     *   x=10  11  12  13
     *       ┌──┬──┬──┬──┐
     *       │  │  │  │  │
     *       └──┴──┴──┴──┘
     *
     * The destination starts at x=12, so source and
     * destination overlap.
     */
    draw.opcode = MYGPU_CMD_DRAW_RECT;
    draw.x = 10;
    draw.y = 20;
    draw.width = 4;
    draw.height = 1;
    draw.color = color;

    check(
        mygpu_command_buffer_write(
            buffer,
            &draw,
            sizeof(draw)
        ) == 0,
        "write overlap source DRAW_RECT"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "draw overlap source rectangle"
    );

    mygpu_command_buffer_destroy(buffer);

    buffer = mygpu_command_buffer_create(128);

    if (buffer == NULL) {
        check(0, "COPY overlap execution buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * Copy:
     *
     * source      = (10, 20)
     * destination = (12, 20)
     * size        = 4x1
     *
     * The temporary buffer in COPY should make this
     * behave correctly even though the regions overlap.
     */
    copy.opcode = MYGPU_CMD_COPY;
    copy.src_x = 10;
    copy.src_y = 20;
    copy.dst_x = 12;
    copy.dst_y = 20;
    copy.width = 4;
    copy.height = 1;

    check(
        mygpu_command_buffer_write(
            buffer,
            &copy,
            sizeof(copy)
        ) == 0,
        "write overlapping COPY command"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "execute overlapping COPY command"
    );

    /*
     * The destination pixels should all contain the
     * original source color.
     */
    check_pixel(
        gpu,
        12,
        20,
        color,
        "overlapping COPY destination pixel 12"
    );

    check_pixel(
        gpu,
        13,
        20,
        color,
        "overlapping COPY destination pixel 13"
    );

    check_pixel(
        gpu,
        14,
        20,
        color,
        "overlapping COPY destination pixel 14"
    );

    check_pixel(
        gpu,
        15,
        20,
        color,
        "overlapping COPY destination pixel 15"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_copy_zero_size(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_copy copy;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "zero-size COPY GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "zero-size COPY buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * A zero-width COPY has no pixels to copy.
     */
    copy.opcode = MYGPU_CMD_COPY;
    copy.src_x = 10;
    copy.src_y = 10;
    copy.dst_x = 20;
    copy.dst_y = 20;
    copy.width = 0;
    copy.height = 10;

    check(
        mygpu_command_buffer_write(
            buffer,
            &copy,
            sizeof(copy)
        ) == 0,
        "write zero-width COPY"
    );

    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) == 0,
        "ignore zero-width COPY"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_copy_outside(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_copy copy;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "out-of-bounds COPY GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "out-of-bounds COPY buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * The framebuffer is 320x200.
     *
     * src_x=319 and width=2 means the source rectangle
     * would extend past the right edge.
     */
    copy.opcode = MYGPU_CMD_COPY;
    copy.src_x = 319;
    copy.src_y = 10;
    copy.dst_x = 100;
    copy.dst_y = 10;
    copy.width = 2;
    copy.height = 1;

    check(
        mygpu_command_buffer_write(
            buffer,
            &copy,
            sizeof(copy)
        ) == 0,
        "write out-of-bounds COPY"
    );

    /*
     * Our first COPY implementation rejects a rectangle
     * when its source or destination extends outside the
     * framebuffer.
     */
    check(
        mygpu_command_buffer_execute(
            gpu,
            buffer
        ) != 0,
        "reject out-of-bounds COPY"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_present(void)
{
    struct mygpu *gpu;
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;
    int result;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "PRESENT GPU setup");
        return;
    }

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "PRESENT command buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    /*
     * A newly created GPU should not have a presented
     * framebuffer yet.
     */
    check(
        mygpu_is_presented(gpu) == 0,
        "framebuffer initially not presented"
    );

    opcode = MYGPU_CMD_PRESENT;

    result = mygpu_command_buffer_write(
        buffer,
        &opcode,
        sizeof(opcode)
    );

    check(
        result == 0,
        "write PRESENT command"
    );

    result = mygpu_command_buffer_execute(
        gpu,
        buffer
    );

    check(
        result == 0,
        "execute PRESENT command"
    );

    /*
     * PRESENT should mark the current framebuffer as
     * presented.
     */
    check(
        mygpu_is_presented(gpu) == 1,
        "PRESENT marks framebuffer presented"
    );

    mygpu_reset(gpu);

    check(
        mygpu_is_presented(gpu) == 0,
        "reset clears presented state"
    );

    mygpu_command_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_validate_null(void)
{
    check(
        mygpu_command_buffer_validate(NULL) != 0,
        "validate NULL command buffer"
    );
}

static void test_validate_clear(void)
{
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_clear command;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate CLEAR setup");
        return;
    }

    command.opcode = MYGPU_CMD_CLEAR;
    command.color = 0xFF0000FFu;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write CLEAR for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) == 0,
        "validate CLEAR command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_draw_rect(void)
{
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_draw_rect command;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate DRAW_RECT setup");
        return;
    }

    command.opcode = MYGPU_CMD_DRAW_RECT;
    command.x = 10;
    command.y = 20;
    command.width = 3;
    command.height = 2;
    command.color = 0xFF0000FFu;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write DRAW_RECT for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) == 0,
        "validate DRAW_RECT command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_copy(void)
{
    struct mygpu_command_buffer *buffer;
    struct mygpu_cmd_copy command;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate COPY setup");
        return;
    }

    command.opcode = MYGPU_CMD_COPY;
    command.src_x = 0;
    command.src_y = 0;
    command.dst_x = 10;
    command.dst_y = 10;
    command.width = 5;
    command.height = 5;

    check(
        mygpu_command_buffer_write(
            buffer,
            &command,
            sizeof(command)
        ) == 0,
        "write COPY for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) == 0,
        "validate COPY command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_present(void)
{
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate PRESENT setup");
        return;
    }

    opcode = MYGPU_CMD_PRESENT;

    check(
        mygpu_command_buffer_write(
            buffer,
            &opcode,
            sizeof(opcode)
        ) == 0,
        "write PRESENT for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) == 0,
        "validate PRESENT command"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_invalid_opcode(void)
{
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate invalid opcode setup");
        return;
    }

    opcode = 0xFFFFFFFFu;

    check(
        mygpu_command_buffer_write(
            buffer,
            &opcode,
            sizeof(opcode)
        ) == 0,
        "write invalid opcode for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) != 0,
        "reject invalid opcode during validation"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_truncated_clear(void)
{
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate truncated CLEAR setup");
        return;
    }

    opcode = MYGPU_CMD_CLEAR;

    check(
        mygpu_command_buffer_write(
            buffer,
            &opcode,
            sizeof(opcode)
        ) == 0,
        "write truncated CLEAR for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) != 0,
        "reject truncated CLEAR during validation"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_truncated_draw_rect(void)
{
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate truncated DRAW_RECT setup");
        return;
    }

    opcode = MYGPU_CMD_DRAW_RECT;

    check(
        mygpu_command_buffer_write(
            buffer,
            &opcode,
            sizeof(opcode)
        ) == 0,
        "write truncated DRAW_RECT for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) != 0,
        "reject truncated DRAW_RECT during validation"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_truncated_copy(void)
{
    struct mygpu_command_buffer *buffer;
    uint32_t opcode;

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate truncated COPY setup");
        return;
    }

    opcode = MYGPU_CMD_COPY;

    check(
        mygpu_command_buffer_write(
            buffer,
            &opcode,
            sizeof(opcode)
        ) == 0,
        "write truncated COPY for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) != 0,
        "reject truncated COPY during validation"
    );

    mygpu_command_buffer_destroy(buffer);
}

static void test_validate_truncated_present(void)
{
    struct mygpu_command_buffer *buffer;
    uint8_t data[2] = { 0 };

    buffer = mygpu_command_buffer_create(64);

    if (buffer == NULL) {
        check(0, "validate truncated PRESENT setup");
        return;
    }

    check(
        mygpu_command_buffer_write(
            buffer,
            data,
            sizeof(data)
        ) == 0,
        "write truncated PRESENT for validation"
    );

    check(
        mygpu_command_buffer_validate(buffer) != 0,
        "reject truncated PRESENT during validation"
    );

    mygpu_command_buffer_destroy(buffer);
}

int main(void)
{
    printf("=== MyGPU Command Tests ===\n\n");

    test_create_destroy();
    test_create_zero_size();
    test_write_command();
    test_write_too_large();
    test_execute_clear();
    test_invalid_opcode();
    test_truncated_command();

    test_draw_rect();
    test_draw_rect_right_edge();
    test_draw_rect_bottom_edge();
    test_draw_rect_outside();

    test_copy();
    test_copy_overlap();
    test_copy_zero_size();
    test_copy_outside();
    
    test_present();

    test_validate_null();
    test_validate_clear();
    test_validate_draw_rect();
    test_validate_copy();
    test_validate_present();
    test_validate_invalid_opcode();
    test_validate_truncated_clear();
    test_validate_truncated_draw_rect();
    test_validate_truncated_copy();
    test_validate_truncated_present();

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