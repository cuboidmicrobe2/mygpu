#include <stdlib.h>

#include "fence.h"

struct mygpu_fence {
    uint64_t id;
    int signaled;
};

struct mygpu_fence *mygpu_fence_create(uint64_t id)
{
    struct mygpu_fence *fence;

    fence = calloc(1, sizeof(*fence));

    if (fence == NULL) {
        return NULL;
    }

    fence->id = id;
    fence->signaled = 0;

    return fence;
}

void mygpu_fence_destroy(struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return;
    }

    free(fence);
}

int mygpu_fence_is_signaled(const struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return 0;   
    }

    return fence->signaled;
}

void mygpu_fence_signal(struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return;
    }

    fence->signaled = 1;
}

void mygpu_fence_reset(struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return;
    }

    fence->signaled = 0;
}

uint64_t mygpu_fence_id(const struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return 0;
    }

    return fence->id;
}