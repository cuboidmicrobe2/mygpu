#ifndef MYGPU_FENCE_H
#define MYGPU_FENCE_H

#include <stdint.h>

struct mygpu_fence;

struct mygpu_fence *mygpu_fence_create(uint64_t id);

void mygpu_fence_retain(struct mygpu_fence *fence);

void mygpu_fence_release(struct mygpu_fence *fence);

void mygpu_fence_destroy(struct mygpu_fence *fence);

int mygpu_fence_is_signaled(const struct mygpu_fence *fence);

void mygpu_fence_signal(struct mygpu_fence *fence);

void mygpu_fence_reset(struct mygpu_fence *fence);

uint64_t mygpu_fence_id(const struct mygpu_fence *fence);

#endif