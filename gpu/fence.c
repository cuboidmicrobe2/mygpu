#include <stdlib.h>

#include "mygpu/fence.h"

struct mygpu_fence {
    uint64_t id;
    int signaled;
    uint32_t refcount;
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
    fence->refcount = 1;

    return fence;
}

void mygpu_fence_retain(struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return;
    }

    if (fence->refcount == UINT32_MAX) {
        return;
    }

    fence->refcount++;
}

void mygpu_fence_release(struct mygpu_fence *fence)
{
    if (fence == NULL) {
        return;
    }

    if (fence->refcount == 0) {
        return;
    }

    fence->refcount--;

    if (fence->refcount == 0) {
        free(fence);
    }
}

void mygpu_fence_destroy(struct mygpu_fence *fence)
{
    mygpu_fence_release(fence);
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