#include <stdlib.h>

#include "queue.h"
#include "commands.h"
#include "fence.h"
#include "gpu_internal.h"

struct mygpu_queue_entry {
    struct mygpu_command_buffer *buffer;
    struct mygpu_fence *fence;
    struct mygpu_queue_entry *next;
};

struct mygpu_queue {
    struct mygpu_queue_entry *head;
    struct mygpu_queue_entry *tail;
};

struct mygpu_queue *mygpu_queue_create(void)
{
    struct mygpu_queue *queue;

    queue = calloc(1, sizeof(*queue));

    if (queue == NULL) {
        return NULL;
    }

    return queue;
}

void mygpu_queue_destroy(struct mygpu_queue *queue)
{
    struct mygpu_queue_entry *entry;
    struct mygpu_queue_entry *next;

    if (queue == NULL) {
        return;
    }

    entry = queue->head;

    while (entry != NULL) {
        next = entry->next;

        free(entry);

        entry = next;
    }

    free(queue);
}

int mygpu_queue_submit(struct mygpu_queue *queue, struct mygpu_command_buffer *buffer, struct mygpu_fence *fence)
{
    struct mygpu_queue_entry *entry;

    if (queue == NULL || buffer == NULL || fence == NULL) {
        return -1;
    }

    entry = malloc(sizeof(*entry));

    if (entry == NULL) {
        return -1;
    }

    entry->buffer = buffer;
    entry->fence = fence;
    entry->next = NULL;

    if (queue->tail == NULL) {
        queue->head = entry;
        queue->tail = entry;
    } else {
        queue->tail->next = entry;
        queue->tail = entry;
    }

    return 0;
}

int mygpu_queue_process(struct mygpu *gpu, struct mygpu_queue *queue)
{
    struct mygpu_queue_entry *entry;
    int result;

    if (gpu == NULL || queue == NULL) {
        return -1;
    }

    while (queue->head != NULL) {
        entry = queue->head;

        queue->head = entry->next;

        if (queue->head == NULL) {
            queue->tail = NULL;
        }

        result = mygpu_command_buffer_execute(gpu, entry->buffer);

        if (result == 0) {
            mygpu_fence_signal(entry->fence);
        }

        free(entry);

        if (result != 0) {
            return -1;
        }
    }

    return 0;
}