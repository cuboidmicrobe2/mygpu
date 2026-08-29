#include <stdint.h>
#include <stdio.h>

#include "mygpu/gpu.h"
#include "mygpu/buffer.h"

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

static void test_create_destroy(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    gpu = mygpu_create();

    check(
        gpu != NULL,
        "create GPU for buffer"
    );

    if (gpu == NULL) {
        return;
    }

    buffer = mygpu_buffer_create(gpu, 64);

    check(
        buffer != NULL,
        "create buffer"
    );

    mygpu_buffer_destroy(buffer);

    check(
        1,
        "destroy buffer"
    );

    mygpu_destroy(gpu);
}

static void test_create_zero_size(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "zero-size buffer GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 0);

    check(
        buffer == NULL,
        "reject zero-size buffer"
    );

    mygpu_destroy(gpu);
}

static void test_create_null_gpu(void)
{
    struct mygpu_buffer *buffer;

    buffer = mygpu_buffer_create(NULL, 64);

    check(
        buffer == NULL,
        "reject buffer with NULL GPU"
    );
}

static void test_size(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "buffer size GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 128);

    if (buffer == NULL) {
        check(0, "buffer size buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_size(buffer) == 128,
        "buffer reports correct size"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_write_read(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint32_t write_value = 0x12345678;
    uint32_t read_value = 0;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "write/read GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, sizeof(write_value));

    if (buffer == NULL) {
        check(0, "write/read buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_write(
            buffer,
            0,
            &write_value,
            sizeof(write_value)
        ) == 0,
        "write buffer"
    );

    check(
        mygpu_buffer_read(
            buffer,
            0,
            &read_value,
            sizeof(read_value)
        ) == 0,
        "read buffer"
    );

    check(
        read_value == write_value,
        "buffer data matches"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_new_buffer_is_zeroed(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint32_t value = 0xFFFFFFFF;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "zeroed buffer GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, sizeof(value));

    if (buffer == NULL) {
        check(0, "zeroed buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_read(
            buffer,
            0,
            &value,
            sizeof(value)
        ) == 0,
        "read newly-created buffer"
    );

    check(
        value == 0,
        "new buffer is zeroed"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_partial_write_read(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint8_t write_data[] = {
        0x10,
        0x20,
        0x30,
        0x40
    };

    uint8_t read_data[4] = { 0 };

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "partial write/read GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 16);

    if (buffer == NULL) {
        check(0, "partial write/read buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_write(
            buffer,
            4,
            write_data,
            sizeof(write_data)
        ) == 0,
        "write buffer at offset"
    );

    check(
        mygpu_buffer_read(
            buffer,
            4,
            read_data,
            sizeof(read_data)
        ) == 0,
        "read buffer at offset"
    );

    check(
        read_data[0] == 0x10 &&
        read_data[1] == 0x20 &&
        read_data[2] == 0x30 &&
        read_data[3] == 0x40,
        "partial buffer data matches"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_write_past_end(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint8_t data[4] = {
        1, 2, 3, 4
    };

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "write past end GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 8);

    if (buffer == NULL) {
        check(0, "write past end buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_write(
            buffer,
            6,
            data,
            sizeof(data)
        ) != 0,
        "reject write past buffer end"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_read_past_end(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint8_t data[4];

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "read past end GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 8);

    if (buffer == NULL) {
        check(0, "read past end buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_read(
            buffer,
            6,
            data,
            sizeof(data)
        ) != 0,
        "reject read past buffer end"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_offset_past_end(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint8_t data = 0;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "offset past end GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 8);

    if (buffer == NULL) {
        check(0, "offset past end buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_write(
            buffer,
            9,
            &data,
            1
        ) != 0,
        "reject write with offset past end"
    );

    check(
        mygpu_buffer_read(
            buffer,
            9,
            &data,
            1
        ) != 0,
        "reject read with offset past end"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_null_arguments(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    uint32_t value = 0;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "NULL argument GPU setup");
        return;
    }

    buffer = mygpu_buffer_create(gpu, 16);

    if (buffer == NULL) {
        check(0, "NULL argument buffer setup");
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_write(
            NULL,
            0,
            &value,
            sizeof(value)
        ) != 0,
        "reject NULL buffer on write"
    );

    check(
        mygpu_buffer_read(
            NULL,
            0,
            &value,
            sizeof(value)
        ) != 0,
        "reject NULL buffer on read"
    );

    check(
        mygpu_buffer_write(
            buffer,
            0,
            NULL,
            sizeof(value)
        ) != 0,
        "reject NULL write data"
    );

    check(
        mygpu_buffer_read(
            buffer,
            0,
            NULL,
            sizeof(value)
        ) != 0,
        "reject NULL read data"
    );

    check(
        mygpu_buffer_size(NULL) == 0,
        "NULL buffer size is zero"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_multiple_buffers(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer_a;
    struct mygpu_buffer *buffer_b;

    uint32_t value_a = 0xAAAAAAAA;
    uint32_t value_b = 0xBBBBBBBB;

    uint32_t read_a = 0;
    uint32_t read_b = 0;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "multiple buffer GPU setup");
        return;
    }

    buffer_a = mygpu_buffer_create(gpu, 16);
    buffer_b = mygpu_buffer_create(gpu, 16);

    check(
        buffer_a != NULL,
        "create first buffer"
    );

    check(
        buffer_b != NULL,
        "create second buffer"
    );

    if (buffer_a == NULL || buffer_b == NULL) {
        mygpu_buffer_destroy(buffer_a);
        mygpu_buffer_destroy(buffer_b);
        mygpu_destroy(gpu);
        return;
    }

    mygpu_buffer_write(
        buffer_a,
        0,
        &value_a,
        sizeof(value_a)
    );

    mygpu_buffer_write(
        buffer_b,
        0,
        &value_b,
        sizeof(value_b)
    );

    mygpu_buffer_read(
        buffer_a,
        0,
        &read_a,
        sizeof(read_a)
    );

    mygpu_buffer_read(
        buffer_b,
        0,
        &read_b,
        sizeof(read_b)
    );

    check(
        read_a == value_a,
        "first buffer keeps its own data"
    );

    check(
        read_b == value_b,
        "second buffer keeps its own data"
    );

    mygpu_buffer_destroy(buffer_a);
    mygpu_buffer_destroy(buffer_b);
    mygpu_destroy(gpu);
}

static void test_address(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer;

    check(
        mygpu_buffer_address(NULL) == 0,
        "NULL buffer address is zero"
    );

    gpu = mygpu_create();

    check(
        gpu != NULL,
        "buffer address GPU setup"
    );

    if (gpu == NULL) {
        return;
    }

    buffer = mygpu_buffer_create(gpu, 64);

    check(
        buffer != NULL,
        "create buffer for address"
    );

    if (buffer == NULL) {
        mygpu_destroy(gpu);
        return;
    }

    check(
        mygpu_buffer_address(buffer) == 0,
        "buffer has initial GPU address"
    );

    mygpu_buffer_destroy(buffer);
    mygpu_destroy(gpu);
}

static void test_buffer_addresses_and_isolation(void)
{
    struct mygpu *gpu;
    struct mygpu_buffer *buffer_a;
    struct mygpu_buffer *buffer_b;

    uint32_t value_a = 0xAAAAAAAA;
    uint32_t value_b = 0xBBBBBBBB;

    uint32_t read_a = 0;
    uint32_t read_b = 0;

    uint32_t address_a;
    uint32_t address_b;

    gpu = mygpu_create();

    if (gpu == NULL) {
        check(0, "buffer isolation GPU setup");
        return;
    }

    buffer_a = mygpu_buffer_create(gpu, 64);
    buffer_b = mygpu_buffer_create(gpu, 64);

    check(
        buffer_a != NULL,
        "create first buffer for isolation"
    );

    check(
        buffer_b != NULL,
        "create second buffer for isolation"
    );

    if (buffer_a == NULL || buffer_b == NULL) {
        mygpu_buffer_destroy(buffer_a);
        mygpu_buffer_destroy(buffer_b);
        mygpu_destroy(gpu);
        return;
    }

    address_a = mygpu_buffer_address(buffer_a);
    address_b = mygpu_buffer_address(buffer_b);

    check(
        address_a != address_b,
        "buffers have different GPU addresses"
    );

    check(
        address_b >= address_a + mygpu_buffer_size(buffer_a),
        "buffer addresses do not overlap"
    );

    check(
        mygpu_buffer_write(
            buffer_a,
            0,
            &value_a,
            sizeof(value_a)
        ) == 0,
        "write first buffer"
    );

    check(
        mygpu_buffer_write(
            buffer_b,
            0,
            &value_b,
            sizeof(value_b)
        ) == 0,
        "write second buffer"
    );

    check(
        mygpu_buffer_read(
            buffer_a,
            0,
            &read_a,
            sizeof(read_a)
        ) == 0,
        "read first buffer"
    );

    check(
        mygpu_buffer_read(
            buffer_b,
            0,
            &read_b,
            sizeof(read_b)
        ) == 0,
        "read second buffer"
    );

    check(
        read_a == value_a,
        "first buffer data is isolated"
    );

    check(
        read_b == value_b,
        "second buffer data is isolated"
    );

    mygpu_buffer_destroy(buffer_a);
    mygpu_buffer_destroy(buffer_b);
    mygpu_destroy(gpu);
}

int main(void)
{
    printf("=== MyGPU Buffer Tests ===\n\n");

    test_create_destroy();
    test_create_zero_size();
    test_create_null_gpu();
    test_size();
    test_write_read();
    test_new_buffer_is_zeroed();
    test_partial_write_read();
    test_write_past_end();
    test_read_past_end();
    test_offset_past_end();
    test_null_arguments();
    test_multiple_buffers();
    test_address();
    test_buffer_addresses_and_isolation();

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