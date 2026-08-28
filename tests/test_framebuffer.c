#include <stdint.h>
#include <stdio.h>

#include "../include/mygpu/framebuffer.h"

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
    struct mygpu_framebuffer *framebuffer;

    framebuffer = mygpu_framebuffer_create(320, 200);

    check(
        framebuffer != NULL,
        "create framebuffer"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_dimensions(void)
{
    struct mygpu_framebuffer *framebuffer;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "dimensions setup");
        return;
    }

    check(
        mygpu_framebuffer_width(framebuffer) == 320,
        "framebuffer width is 320"
    );

    check(
        mygpu_framebuffer_height(framebuffer) == 200,
        "framebuffer height is 200"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_initial_pixels_are_zero(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "initial pixel setup");
        return;
    }

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        0,
        &color
    );

    check(
        result == 0,
        "read initial pixel"
    );

    check(
        color == 0,
        "initial pixel is zero"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        319,
        199,
        &color
    );

    check(
        result == 0,
        "read initial last pixel"
    );

    check(
        color == 0,
        "initial last pixel is zero"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_set_and_get_pixel(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "set/get pixel setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        10,
        20,
        0xFF0000FF
    );

    check(
        result == 0,
        "set pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        10,
        20,
        &color
    );

    check(
        result == 0,
        "get pixel"
    );

    check(
        color == 0xFF0000FF,
        "pixel contains correct color"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_multiple_pixels(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "multiple pixels setup");
        return;
    }

    mygpu_framebuffer_set_pixel(
        framebuffer,
        0,
        0,
        0xFF0000FF
    );

    mygpu_framebuffer_set_pixel(
        framebuffer,
        1,
        0,
        0x00FF00FF
    );

    mygpu_framebuffer_set_pixel(
        framebuffer,
        0,
        1,
        0x0000FFFF
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        0,
        &color
    );

    check(
        result == 0 && color == 0xFF0000FF,
        "first pixel is correct"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        1,
        0,
        &color
    );

    check(
        result == 0 && color == 0x00FF00FF,
        "second pixel is correct"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        1,
        &color
    );

    check(
        result == 0 && color == 0x0000FFFF,
        "pixel on second row is correct"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_clear(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "clear setup");
        return;
    }

    mygpu_framebuffer_clear(
        framebuffer,
        0x12345678
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        0,
        &color
    );

    check(
        result == 0 && color == 0x12345678,
        "clear first pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        100,
        100,
        &color
    );

    check(
        result == 0 && color == 0x12345678,
        "clear middle pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        319,
        199,
        &color
    );

    check(
        result == 0 && color == 0x12345678,
        "clear last pixel"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_clear_overwrites_existing_pixels(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "clear overwrite setup");
        return;
    }

    mygpu_framebuffer_set_pixel(
        framebuffer,
        10,
        10,
        0xFFFFFFFF
    );

    mygpu_framebuffer_set_pixel(
        framebuffer,
        100,
        100,
        0xFFFFFFFF
    );

    mygpu_framebuffer_clear(
        framebuffer,
        0x000000FF
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        10,
        10,
        &color
    );

    check(
        result == 0 && color == 0x000000FF,
        "clear overwrites existing pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        100,
        100,
        &color
    );

    check(
        result == 0 && color == 0x000000FF,
        "clear overwrites another pixel"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_first_pixel_boundary(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "first boundary setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        0,
        0,
        0xAABBCCDD
    );

    check(
        result == 0,
        "set first pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        0,
        &color
    );

    check(
        result == 0 && color == 0xAABBCCDD,
        "first pixel has correct color"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_last_pixel_boundary(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "last boundary setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        319,
        199,
        0x11223344
    );

    check(
        result == 0,
        "set last pixel"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        319,
        199,
        &color
    );

    check(
        result == 0 && color == 0x11223344,
        "last pixel has correct color"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_x_out_of_bounds(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "x bounds setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        320,
        0,
        0xFFFFFFFF
    );

    check(
        result != 0,
        "reject x equal to width"
    );

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        1000,
        0,
        0xFFFFFFFF
    );

    check(
        result != 0,
        "reject x greater than width"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        320,
        0,
        &color
    );

    check(
        result != 0,
        "reject out-of-bounds x read"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_y_out_of_bounds(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "y bounds setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        0,
        200,
        0xFFFFFFFF
    );

    check(
        result != 0,
        "reject y equal to height"
    );

    result = mygpu_framebuffer_set_pixel(
        framebuffer,
        0,
        1000,
        0xFFFFFFFF
    );

    check(
        result != 0,
        "reject y greater than height"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        200,
        &color
    );

    check(
        result != 0,
        "reject out-of-bounds y read"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

static void test_zero_dimensions(void)
{
    struct mygpu_framebuffer *framebuffer;

    framebuffer = mygpu_framebuffer_create(0, 200);

    check(
        framebuffer == NULL,
        "reject zero width"
    );

    framebuffer = mygpu_framebuffer_create(320, 0);

    check(
        framebuffer == NULL,
        "reject zero height"
    );

    framebuffer = mygpu_framebuffer_create(0, 0);

    check(
        framebuffer == NULL,
        "reject zero width and height"
    );
}

static void test_null_arguments(void)
{
    struct mygpu_framebuffer *framebuffer;
    uint32_t color;
    int result;

    framebuffer = mygpu_framebuffer_create(320, 200);

    if (framebuffer == NULL) {
        check(0, "NULL argument setup");
        return;
    }

    result = mygpu_framebuffer_set_pixel(
        NULL,
        0,
        0,
        0xFFFFFFFF
    );

    check(
        result != 0,
        "reject NULL framebuffer on set"
    );

    result = mygpu_framebuffer_get_pixel(
        NULL,
        0,
        0,
        &color
    );

    check(
        result != 0,
        "reject NULL framebuffer on get"
    );

    result = mygpu_framebuffer_get_pixel(
        framebuffer,
        0,
        0,
        NULL
    );

    check(
        result != 0,
        "reject NULL color pointer"
    );

    mygpu_framebuffer_clear(
        NULL,
        0xFFFFFFFF
    );

    check(
        1,
        "clear safely handles NULL framebuffer"
    );

    check(
        mygpu_framebuffer_width(NULL) == 0,
        "NULL framebuffer width returns zero"
    );

    check(
        mygpu_framebuffer_height(NULL) == 0,
        "NULL framebuffer height returns zero"
    );

    mygpu_framebuffer_destroy(NULL);

    check(
        1,
        "destroy safely handles NULL framebuffer"
    );

    mygpu_framebuffer_destroy(framebuffer);
}

int main(void)
{
    printf("=== MyGPU Framebuffer Tests ===\n\n");

    test_create_destroy();
    test_dimensions();
    test_initial_pixels_are_zero();
    test_set_and_get_pixel();
    test_multiple_pixels();
    test_clear();
    test_clear_overwrites_existing_pixels();
    test_first_pixel_boundary();
    test_last_pixel_boundary();
    test_x_out_of_bounds();
    test_y_out_of_bounds();
    test_zero_dimensions();
    test_null_arguments();

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