#ifndef MYGPU_GPU_INTERNAL_H
#define MYGPU_GPU_INTERNAL_H

#include "memory.h"
#include "registers.h"
#include "framebuffer.h"

struct mygpu
{
    int initialized;
    int presented;

    struct mygpu_memory *memory;
    struct mygpu_registers *registers;
    struct mygpu_framebuffer *framebuffer;
};

#endif