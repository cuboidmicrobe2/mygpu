#ifndef MYGPU_QUEUE_H
#define MYGPU_QUEUE_H

struct mygpu;
struct mygpu_queue;
struct mygpu_command_buffer;
struct mygpu_fence;

struct mygpu_queue *mygpu_queue_create(void);

void mygpu_queue_destroy(struct mygpu_queue *queue);

int mygpu_queue_submit(struct mygpu_queue *queue, struct mygpu_command_buffer *buffer, struct mygpu_fence *fence);

int mygpu_queue_process(struct mygpu *gpu, struct mygpu_queue *queue);

#endif