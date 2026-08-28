#include <stdlib.h>

#include "../include/mygpu/registers.h"

struct mygpu_registers {
    uint32_t status;
    uint32_t command_ptr;
    uint32_t command_size;
    uint32_t framebuffer;
    uint32_t width;
    uint32_t height;
};

struct mygpu_registers *mygpu_registers_create(void)
{
    struct mygpu_registers* registers;

    registers = malloc(sizeof(*registers));

    if (registers == NULL) {
        return NULL;
    }

    mygpu_registers_reset(registers);

    return registers;
}

void mygpu_registers_destroy(struct mygpu_registers *registers)
{
    if (registers == NULL) {
        return;
    }

    free(registers);
}

void mygpu_registers_reset(struct mygpu_registers *registers)
{
    if (registers == NULL) {
        return;
    }

    registers->status = 0;
    registers->command_ptr = 0;
    registers->command_size = 0;
    registers->framebuffer = 0;

    registers->width = 320;
    registers->height = 200;
}

int mygpu_register_read(struct mygpu_registers *registers, uint32_t address, uint32_t *value)
{
    if (registers == NULL || value == NULL) {
        return -1;
    }

    switch (address)
    {
    case MYGPU_REG_STATUS:
        *value = registers->status;
        return 0;

    case MYGPU_REG_COMMAND_PTR:
        *value = registers->command_ptr;
        return 0;
    
    case MYGPU_REG_COMMAND_SIZE:
        *value = registers->command_size;
        return 0;   

    case MYGPU_REG_FRAMEBUFFER:
        *value = registers->framebuffer;
        return 0;

    case MYGPU_REG_WIDTH:
        *value = registers->width;
        return 0;
    
    case MYGPU_REG_HEIGHT:
        *value = registers->height;
        return 0;
    
    default:
        return -1;
    }
}

int mygpu_register_write(struct mygpu_registers *registers, uint32_t address, uint32_t value)
{
    if (registers == NULL) {
        return -1;
    }

    switch (address)
    {
    case MYGPU_REG_STATUS:
        registers->status = value;
        return 0;

    case MYGPU_REG_COMMAND_PTR:
        registers->command_ptr = value;
        return 0;

    case MYGPU_REG_COMMAND_SIZE:
        registers->command_size = value;
        return 0;

    case MYGPU_REG_FRAMEBUFFER:
        registers->framebuffer = value;
        return 0;

    case MYGPU_REG_WIDTH:
        registers->width = value;
        return 0;

    case MYGPU_REG_HEIGHT:
        registers->height = value;
        return 0;

    default:
        return -1;
    }
}