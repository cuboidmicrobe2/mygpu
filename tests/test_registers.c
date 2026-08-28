#include <stdint.h>
#include <stdio.h>

#include "../gpu/registers.h"

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
    struct mygpu_registers *registers;

    registers = mygpu_registers_create();

    check(
        registers != NULL,
        "create registers"
    );

    mygpu_registers_destroy(registers);
}

static void test_reset_values(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "reset values setup");
        return;
    }

    result = mygpu_register_read(
        registers,
        MYGPU_REG_STATUS,
        &value
    );

    check(
        result == 0 && value == 0,
        "STATUS resets to zero"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_PTR,
        &value
    );

    check(
        result == 0 && value == 0,
        "COMMAND_PTR resets to zero"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_SIZE,
        &value
    );

    check(
        result == 0 && value == 0,
        "COMMAND_SIZE resets to zero"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_FRAMEBUFFER,
        &value
    );

    check(
        result == 0 && value == 0,
        "FRAMEBUFFER resets to zero"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_WIDTH,
        &value
    );

    check(
        result == 0 && value == 320,
        "WIDTH resets to 320"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_HEIGHT,
        &value
    );

    check(
        result == 0 && value == 200,
        "HEIGHT resets to 200"
    );

    mygpu_registers_destroy(registers);
}

static void test_write_and_read(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "write/read setup");
        return;
    }

    result = mygpu_register_write(
        registers,
        MYGPU_REG_COMMAND_PTR,
        0x1000
    );

    check(
        result == 0,
        "write COMMAND_PTR"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_PTR,
        &value
    );

    check(
        result == 0,
        "read COMMAND_PTR"
    );

    check(
        value == 0x1000,
        "COMMAND_PTR contains written value"
    );

    mygpu_registers_destroy(registers);
}

static void test_all_writable_registers(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "writable register setup");
        return;
    }

    result = mygpu_register_write(
        registers,
        MYGPU_REG_STATUS,
        1
    );

    check(
        result == 0,
        "write STATUS"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_STATUS,
        &value
    );

    check(
        result == 0 && value == 1,
        "STATUS contains written value"
    );

    result = mygpu_register_write(
        registers,
        MYGPU_REG_COMMAND_PTR,
        0x12345678
    );

    check(
        result == 0,
        "write COMMAND_PTR"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_PTR,
        &value
    );

    check(
        result == 0 && value == 0x12345678,
        "COMMAND_PTR contains written value"
    );

    result = mygpu_register_write(
        registers,
        MYGPU_REG_COMMAND_SIZE,
        4096
    );

    check(
        result == 0,
        "write COMMAND_SIZE"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_SIZE,
        &value
    );

    check(
        result == 0 && value == 4096,
        "COMMAND_SIZE contains written value"
    );

    result = mygpu_register_write(
        registers,
        MYGPU_REG_FRAMEBUFFER,
        0x8000
    );

    check(
        result == 0,
        "write FRAMEBUFFER"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_FRAMEBUFFER,
        &value
    );

    check(
        result == 0 && value == 0x8000,
        "FRAMEBUFFER contains written value"
    );

    result = mygpu_register_write(
        registers,
        MYGPU_REG_WIDTH,
        640
    );

    check(
        result == 0,
        "write WIDTH"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_WIDTH,
        &value
    );

    check(
        result == 0 && value == 640,
        "WIDTH contains written value"
    );

    result = mygpu_register_write(
        registers,
        MYGPU_REG_HEIGHT,
        480
    );

    check(
        result == 0,
        "write HEIGHT"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_HEIGHT,
        &value
    );

    check(
        result == 0 && value == 480,
        "HEIGHT contains written value"
    );

    mygpu_registers_destroy(registers);
}

static void test_invalid_register_read(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "invalid read setup");
        return;
    }

    result = mygpu_register_read(
        registers,
        0xFF,
        &value
    );

    check(
        result != 0,
        "reject invalid register read"
    );

    result = mygpu_register_read(
        registers,
        0x01,
        &value
    );

    check(
        result != 0,
        "reject unaligned register read"
    );

    mygpu_registers_destroy(registers);
}

static void test_invalid_register_write(void)
{
    struct mygpu_registers *registers;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "invalid write setup");
        return;
    }

    result = mygpu_register_write(
        registers,
        0xFF,
        123
    );

    check(
        result != 0,
        "reject invalid register write"
    );

    result = mygpu_register_write(
        registers,
        0x01,
        123
    );

    check(
        result != 0,
        "reject unaligned register write"
    );

    mygpu_registers_destroy(registers);
}

static void test_null_arguments(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "NULL argument setup");
        return;
    }

    result = mygpu_register_read(
        NULL,
        MYGPU_REG_STATUS,
        &value
    );

    check(
        result != 0,
        "reject NULL registers on read"
    );

    result = mygpu_register_write(
        NULL,
        MYGPU_REG_STATUS,
        1
    );

    check(
        result != 0,
        "reject NULL registers on write"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_STATUS,
        NULL
    );

    check(
        result != 0,
        "reject NULL value on read"
    );

    mygpu_registers_destroy(registers);
}

static void test_reset(void)
{
    struct mygpu_registers *registers;
    uint32_t value;
    int result;

    registers = mygpu_registers_create();

    if (registers == NULL) {
        check(0, "reset test setup");
        return;
    }

    /*
     * Change some registers.
     */
    mygpu_register_write(
        registers,
        MYGPU_REG_COMMAND_PTR,
        0x1234
    );

    mygpu_register_write(
        registers,
        MYGPU_REG_COMMAND_SIZE,
        512
    );

    /*
     * Reset the GPU registers.
     */
    mygpu_registers_reset(registers);

    /*
     * COMMAND_PTR should be back to zero.
     */
    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_PTR,
        &value
    );

    check(
        result == 0 && value == 0,
        "reset clears COMMAND_PTR"
    );

    /*
     * COMMAND_SIZE should be back to zero.
     */
    result = mygpu_register_read(
        registers,
        MYGPU_REG_COMMAND_SIZE,
        &value
    );

    check(
        result == 0 && value == 0,
        "reset clears COMMAND_SIZE"
    );

    /*
     * Width and height should return to
     * their hardware defaults.
     */
    result = mygpu_register_read(
        registers,
        MYGPU_REG_WIDTH,
        &value
    );

    check(
        result == 0 && value == 320,
        "reset restores WIDTH"
    );

    result = mygpu_register_read(
        registers,
        MYGPU_REG_HEIGHT,
        &value
    );

    check(
        result == 0 && value == 200,
        "reset restores HEIGHT"
    );

    mygpu_registers_destroy(registers);
}

int main(void)
{
    printf("=== MyGPU Register Tests ===\n\n");

    test_create_destroy();
    test_reset_values();
    test_write_and_read();
    test_all_writable_registers();
    test_invalid_register_read();
    test_invalid_register_write();
    test_null_arguments();
    test_reset();

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