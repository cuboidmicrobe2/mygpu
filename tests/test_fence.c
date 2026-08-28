#include <stdint.h>
#include <stdio.h>

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

static void test_create_destroy(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(42);

    check(
        fence != NULL,
        "create fence"
    );

    mygpu_fence_destroy(fence);

    check(
        1,
        "destroy fence"
    );
}

static void test_initial_state(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(100);

    if (fence == NULL) {
        check(0, "initial state fence setup");
        return;
    }

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "fence initially unsignaled"
    );

    mygpu_fence_destroy(fence);
}

static void test_id(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(12345);

    if (fence == NULL) {
        check(0, "fence ID setup");
        return;
    }

    check(
        mygpu_fence_id(fence) == 12345,
        "fence returns correct ID"
    );

    mygpu_fence_destroy(fence);
}

static void test_signal(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(200);

    if (fence == NULL) {
        check(0, "signal fence setup");
        return;
    }

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "fence starts unsignaled"
    );

    mygpu_fence_signal(fence);

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "signal changes fence state"
    );

    mygpu_fence_destroy(fence);
}

static void test_reset(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(300);

    if (fence == NULL) {
        check(0, "reset fence setup");
        return;
    }

    mygpu_fence_signal(fence);

    check(
        mygpu_fence_is_signaled(fence) == 1,
        "fence signaled before reset"
    );

    mygpu_fence_reset(fence);

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "reset returns fence to unsignaled"
    );

    check(
        mygpu_fence_id(fence) == 300,
        "reset preserves fence ID"
    );

    mygpu_fence_destroy(fence);
}

static void test_multiple_fences(void)
{
    struct mygpu_fence *fence1;
    struct mygpu_fence *fence2;

    fence1 = mygpu_fence_create(1);
    fence2 = mygpu_fence_create(2);

    if (fence1 == NULL || fence2 == NULL) {
        check(0, "multiple fence setup");

        mygpu_fence_destroy(fence1);
        mygpu_fence_destroy(fence2);

        return;
    }

    mygpu_fence_signal(fence1);

    check(
        mygpu_fence_is_signaled(fence1) == 1,
        "first fence can be signaled"
    );

    check(
        mygpu_fence_is_signaled(fence2) == 0,
        "second fence remains unsignaled"
    );

    check(
        mygpu_fence_id(fence1) == 1,
        "first fence has correct ID"
    );

    check(
        mygpu_fence_id(fence2) == 2,
        "second fence has correct ID"
    );

    mygpu_fence_destroy(fence1);
    mygpu_fence_destroy(fence2);
}

static void test_null_arguments(void)
{
    struct mygpu_fence *fence;

    fence = mygpu_fence_create(400);

    if (fence == NULL) {
        check(0, "NULL argument fence setup");
        return;
    }

    check(
        mygpu_fence_is_signaled(NULL) == 0,
        "NULL fence is not signaled"
    );

    mygpu_fence_signal(NULL);

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "signal NULL fence has no effect"
    );

    mygpu_fence_reset(NULL);

    check(
        mygpu_fence_is_signaled(fence) == 0,
        "reset NULL fence has no effect"
    );

    check(
        mygpu_fence_id(NULL) == 0,
        "NULL fence ID returns zero"
    );

    mygpu_fence_destroy(NULL);

    check(
        1,
        "destroy NULL fence is safe"
    );

    mygpu_fence_destroy(fence);
}

int main(void)
{
    printf("=== MyGPU Fence Tests ===\n\n");

    test_create_destroy();
    test_initial_state();
    test_id();
    test_signal();
    test_reset();
    test_multiple_fences();
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