#ifndef MYGPU_REGISTERS_H
#define MYGPU_REGISTERS_H

#include <stdint.h>

enum mygpu_register
{
    MYGPU_REG_STATUS = 0x00,
    MYGPU_REG_COMMAND_PTR = 0x04,
    MYGPU_REG_COMMAND_SIZE = 0x08,
    MYGPU_REG_FRAMEBUFFER = 0x0C,
    MYGPU_REG_WIDTH = 0x10,
    MYGPU_REG_HEIGHT = 0x14
};

struct mygpu_registers;

struct mygpu_registers *mygpu_registers_create(void);

void mygpu_registers_destroy(struct mygpu_registers *registers);

void mygpu_registers_reset(struct mygpu_registers *registers);

int mygpu_register_read(struct mygpu_registers *registers, uint32_t address, uint32_t *value);

int mygpu_register_write(struct mygpu_registers *registers, uint32_t address, uint32_t value);

#endif