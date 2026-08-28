#ifndef MYGPU_GPU_INTERNAL_H
#define MYGPU_GPU_INTERNAL_H

#include "memory.h"
#include "registers.h"
#include "framebuffer.h"
#include "queue.h"

struct mygpu
{
    int initialized;
    int presented;

    struct mygpu_memory *memory;
    struct mygpu_registers *registers;
    struct mygpu_framebuffer *framebuffer;
    struct mygpu_queue *queue;
};

#endif