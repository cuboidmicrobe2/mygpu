#ifndef MYGPU_COMMANDS_H
#define MYGPU_COMMANDS_H

#include <stdint.h>

enum mygpu_opcode
{
    MYGPU_CMD_CLEAR = 0x01,
    MYGPU_CMD_DRAW_RECT = 0x02,
    MYGPU_CMD_COPY = 0x03,
    MYGPU_CMD_PRESENT = 0x04
};

struct mygpu;
struct mygpu_command_buffer;

struct mygpu_cmd_clear {
    uint32_t opcode;
    uint32_t color;
};

struct mygpu_cmd_draw_rect {
    uint32_t opcode;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t color;
};

struct mygpu_cmd_copy {
    uint32_t opcode;
    uint32_t src_x;
    uint32_t src_y;
    uint32_t dst_x;
    uint32_t dst_y;
    uint32_t width;
    uint32_t height;
};

struct mygpu_command_buffer *mygpu_command_buffer_create(uint32_t size);

void mygpu_command_buffer_retain(struct mygpu_command_buffer *buffer);

void mygpu_command_buffer_release(struct mygpu_command_buffer *buffer);

void mygpu_command_buffer_destroy(struct mygpu_command_buffer *buffer);

int mygpu_command_buffer_write(struct mygpu_command_buffer *buffer, const void *data, uint32_t size);

int mygpu_command_buffer_validate(const struct mygpu_command_buffer *buffer);

int mygpu_command_buffer_execute(struct mygpu *gpu, struct mygpu_command_buffer *buffer);

#endif