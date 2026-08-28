#include <stdio.h>

#include "mygpu/gpu.h"

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

int main(void)
{
    printf("=== MyGPU Tests ===\n\n");

    test_create();
    test_destroy_null();
    test_reset();
    test_multiple_gpus();
    test_multiple_resets();
    test_clear_integration();

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