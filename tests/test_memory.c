#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../gpu/memory.h"

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
    struct mygpu_memory *memory;

    memory = mygpu_memory_create();

    check(memory != NULL, "create memory");

    mygpu_memory_destroy(memory);
}

static void test_memory_is_zero_initialized(void)
{
    struct mygpu_memory *memory;
    uint8_t data[16];
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "zero initialization");
        return;
    }

    memset(data, 0xFF, sizeof(data));

    result = mygpu_memory_read(
        memory,
        0,
        data,
        sizeof(data)
    );

    check(result == 0, "read zero-initialized memory");

    check(
        data[0] == 0 &&
        data[1] == 0 &&
        data[2] == 0 &&
        data[3] == 0 &&
        data[15] == 0,
        "memory starts at zero"
    );

    mygpu_memory_destroy(memory);
}

static void test_write_and_read(void)
{
    struct mygpu_memory *memory;
    uint32_t write_value = 0x12345678;
    uint32_t read_value = 0;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "write and read setup");
        return;
    }

    result = mygpu_memory_write(
        memory,
        100,
        &write_value,
        sizeof(write_value)
    );

    check(result == 0, "write 32-bit value");

    result = mygpu_memory_read(
        memory,
        100,
        &read_value,
        sizeof(read_value)
    );

    check(result == 0, "read 32-bit value");

    check(
        read_value == write_value,
        "read value matches written value"
    );

    mygpu_memory_destroy(memory);
}

static void test_write_bytes(void)
{
    struct mygpu_memory *memory;
    uint8_t write_data[] = {
        0x10,
        0x20,
        0x30,
        0x40,
        0x50
    };

    uint8_t read_data[sizeof(write_data)];
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "byte write setup");
        return;
    }

    result = mygpu_memory_write(
        memory,
        200,
        write_data,
        sizeof(write_data)
    );

    check(result == 0, "write byte array");

    memset(read_data, 0, sizeof(read_data));

    result = mygpu_memory_read(
        memory,
        200,
        read_data,
        sizeof(read_data)
    );

    check(result == 0, "read byte array");

    check(
        memcmp(write_data, read_data, sizeof(write_data)) == 0,
        "byte array matches"
    );

    mygpu_memory_destroy(memory);
}

static void test_first_byte_boundary(void)
{
    struct mygpu_memory *memory;
    uint8_t write_value = 0xAA;
    uint8_t read_value = 0;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "first byte boundary setup");
        return;
    }

    result = mygpu_memory_write(
        memory,
        0,
        &write_value,
        sizeof(write_value)
    );

    check(result == 0, "write first byte");

    result = mygpu_memory_read(
        memory,
        0,
        &read_value,
        sizeof(read_value)
    );

    check(result == 0, "read first byte");

    check(
        read_value == write_value,
        "first byte contains correct value"
    );

    mygpu_memory_destroy(memory);
}

static void test_last_byte_boundary(void)
{
    struct mygpu_memory *memory;
    uint8_t write_value = 0xBB;
    uint8_t read_value = 0;
    uint32_t address;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "last byte boundary setup");
        return;
    }

    address = MYGPU_MEMORY_SIZE - 1;

    result = mygpu_memory_write(
        memory,
        address,
        &write_value,
        sizeof(write_value)
    );

    check(result == 0, "write last byte");

    result = mygpu_memory_read(
        memory,
        address,
        &read_value,
        sizeof(read_value)
    );

    check(result == 0, "read last byte");

    check(
        read_value == write_value,
        "last byte contains correct value"
    );

    mygpu_memory_destroy(memory);
}

static void test_write_out_of_bounds(void)
{
    struct mygpu_memory *memory;
    uint8_t data = 0xFF;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "out-of-bounds write setup");
        return;
    }

    result = mygpu_memory_write(
        memory,
        MYGPU_MEMORY_SIZE,
        &data,
        sizeof(data)
    );

    check(
        result != 0,
        "reject write at end of memory"
    );

    result = mygpu_memory_write(
        memory,
        MYGPU_MEMORY_SIZE - 1,
        &data,
        2
    );

    check(
        result != 0,
        "reject write past end of memory"
    );

    mygpu_memory_destroy(memory);
}

static void test_read_out_of_bounds(void)
{
    struct mygpu_memory *memory;
    uint8_t data;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "out-of-bounds read setup");
        return;
    }

    result = mygpu_memory_read(
        memory,
        MYGPU_MEMORY_SIZE,
        &data,
        sizeof(data)
    );

    check(
        result != 0,
        "reject read at end of memory"
    );

    result = mygpu_memory_read(
        memory,
        MYGPU_MEMORY_SIZE - 1,
        &data,
        2
    );

    check(
        result != 0,
        "reject read past end of memory"
    );

    mygpu_memory_destroy(memory);
}

static void test_null_arguments(void)
{
    struct mygpu_memory *memory;
    uint8_t data = 0;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "NULL argument setup");
        return;
    }

    check(
        mygpu_memory_read(
            NULL,
            0,
            &data,
            sizeof(data)
        ) != 0,
        "reject NULL memory on read"
    );

    check(
        mygpu_memory_write(
            NULL,
            0,
            &data,
            sizeof(data)
        ) != 0,
        "reject NULL memory on write"
    );

    check(
        mygpu_memory_read(
            memory,
            0,
            NULL,
            sizeof(data)
        ) != 0,
        "reject NULL data on read"
    );

    check(
        mygpu_memory_write(
            memory,
            0,
            NULL,
            sizeof(data)
        ) != 0,
        "reject NULL data on write"
    );

    mygpu_memory_destroy(memory);
}

static void test_zero_size_operations(void)
{
    struct mygpu_memory *memory;
    uint8_t data = 0;
    int result;

    memory = mygpu_memory_create();

    if (memory == NULL) {
        check(0, "zero-size operation setup");
        return;
    }

    result = mygpu_memory_read(
        memory,
        0,
        &data,
        0
    );

    check(
        result == 0,
        "zero-size read"
    );

    result = mygpu_memory_write(
        memory,
        0,
        &data,
        0
    );

    check(
        result == 0,
        "zero-size write"
    );

    mygpu_memory_destroy(memory);
}

int main(void)
{
    printf("=== MyGPU Memory Tests ===\n\n");

    test_create_destroy();
    test_memory_is_zero_initialized();
    test_write_and_read();
    test_write_bytes();
    test_first_byte_boundary();
    test_last_byte_boundary();
    test_write_out_of_bounds();
    test_read_out_of_bounds();
    test_null_arguments();
    test_zero_size_operations();

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