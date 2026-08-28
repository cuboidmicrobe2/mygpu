#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "commands.h"
#include "gpu_internal.h"

struct mygpu_command_buffer
{
    uint8_t *data;
    uint32_t size;
    uint32_t used;
    uint32_t refcount;
};

struct mygpu_command_buffer *mygpu_command_buffer_create(uint32_t size)
{
    if (size == 0) {
        return NULL;
    }

    struct mygpu_command_buffer *buffer;

    buffer = malloc(sizeof(*buffer));

    if (buffer == NULL) {
        return NULL;
    }

    buffer->data = malloc(size);

    if (buffer->data == NULL) {
        free(buffer);
        return NULL;
    }

    buffer->size = size;
    buffer->used = 0;
    buffer->refcount = 1;

    return buffer;
}

void mygpu_command_buffer_retain(struct mygpu_command_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    if (buffer->refcount == UINT32_MAX) {
        return;
    }

    buffer->refcount++;
}

void mygpu_command_buffer_release(struct mygpu_command_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    if (buffer->refcount == 0) {
        return;
    }

    buffer->refcount--;

    if (buffer->refcount == 0) {
        free(buffer->data);
        free(buffer);
    }
}

void mygpu_command_buffer_destroy(struct mygpu_command_buffer *buffer)
{
    mygpu_command_buffer_release(buffer);
}

int mygpu_command_buffer_write(struct mygpu_command_buffer *buffer, const void *data, uint32_t size)
{
    if (buffer == NULL || data == NULL) {
        return -1;
    }

    if (size > buffer->size - buffer->used) {
        return -1;
    }

    memcpy(buffer->data + buffer->used, data, size);

    buffer->used += size;

    return 0;
}

int mygpu_command_buffer_validate(const struct mygpu_command_buffer *buffer)
{
    uint32_t offset = 0;

    if (buffer == NULL) {
        return -1;
    }

    while (offset < buffer->used) {
        uint32_t opcode;

        if (buffer->used - offset < sizeof(opcode)) {
            return -1;
        }

        memcpy(
            &opcode,
            buffer->data + offset,
            sizeof(opcode)
        );

        switch (opcode)
        {
        case MYGPU_CMD_CLEAR:
            if (buffer->used - offset < sizeof(struct mygpu_cmd_clear)) {
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_clear);
            break;
        
        case MYGPU_CMD_DRAW_RECT:
            if (buffer->used - offset <
                sizeof(struct mygpu_cmd_draw_rect)) {
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_draw_rect);
            break;

        case MYGPU_CMD_COPY:
            if (buffer->used - offset <
                sizeof(struct mygpu_cmd_copy)) {
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_copy);
            break;

        case MYGPU_CMD_PRESENT:
            offset += sizeof(uint32_t);
            break;

        default:
            return -1;
        }
    }

    return 0;
}

int mygpu_command_buffer_execute(struct mygpu *gpu, struct mygpu_command_buffer *buffer)
{
    uint32_t offset = 0;

    if (gpu == NULL || buffer == NULL) {
        return -1;
    }

    if (mygpu_command_buffer_validate(buffer) != 0) {
        return -1;
    }

    while (offset < buffer->used) {
        uint32_t opcode;

        memcpy(&opcode, buffer->data + offset, sizeof(opcode));

        switch (opcode)
        {
        case MYGPU_CMD_CLEAR: {
            struct mygpu_cmd_clear command;

            memcpy(&command, buffer->data + offset, sizeof(command));

            mygpu_framebuffer_clear(gpu->framebuffer, command.color);

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_DRAW_RECT: {
            struct mygpu_cmd_draw_rect command;

            uint32_t framebuffer_width; 
            uint32_t framebuffer_height;

            uint64_t end_x;
            uint64_t end_y;

            memcpy(&command, buffer->data + offset, sizeof(command));

            framebuffer_width = mygpu_framebuffer_width(gpu->framebuffer);

            framebuffer_height = mygpu_framebuffer_height(gpu->framebuffer);

            /* Rectangle has no visible pixels */
            if (command.x >= framebuffer_width ||
                command.y >= framebuffer_height ||
                command.width == 0 ||
                command.height == 0) {

                    offset += sizeof(command);
                    break;
                }
            
            end_x = (uint64_t)command.x + command.width;
            end_y = (uint64_t)command.y + command.height;
            
            /* Clip ending coordinates */
            if (end_x > framebuffer_width) {
                end_x = framebuffer_width;
            }

            if (end_y > framebuffer_height) {
                end_y = framebuffer_height;
            }

            /* Draw visible portion */
            for (uint32_t y = command.y; y < end_y; y++) {
                for(uint32_t x = command.x; x < end_x; x++) {
                    int result = mygpu_framebuffer_set_pixel(gpu->framebuffer, x, y, command.color);

                    if (result != 0) {
                        return -1;
                    }
                }
            }

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_COPY: {
            struct mygpu_cmd_copy command;

            uint32_t framebuffer_width;
            uint32_t framebuffer_height;

            uint64_t src_end_x;
            uint64_t src_end_y;
            uint64_t dst_end_x;
            uint64_t dst_end_y;

            uint32_t *pixels;
            size_t pixel_count;

            memcpy(&command, buffer->data + offset, sizeof(command));

            if (command.width == 0 || command.height == 0) {
                offset += sizeof(command);
                break;
            }

            framebuffer_width = mygpu_framebuffer_width(gpu->framebuffer);
            framebuffer_height = mygpu_framebuffer_height(gpu->framebuffer);

            if (command.src_x >= framebuffer_width ||
                command.src_y >= framebuffer_height ||
                command.dst_x >= framebuffer_width ||
                command.dst_y >= framebuffer_height) {

                offset += sizeof(command);
                break;
            }

            src_end_x = (uint64_t)command.src_x + command.width;
            src_end_y = (uint64_t)command.src_y + command.height;
            dst_end_x = (uint64_t)command.dst_x + command.width;
            dst_end_y = (uint64_t)command.dst_y + command.height;

            if (src_end_x > framebuffer_width) {
                src_end_x = framebuffer_width;
            }

            if (src_end_y > framebuffer_height) {
                src_end_y = framebuffer_height;
            }

            if (dst_end_x > framebuffer_width) {
                dst_end_x = framebuffer_width;
            }

            if (dst_end_y > framebuffer_height) {
                dst_end_y = framebuffer_height;
            }

            if ((src_end_x - command.src_x) != (dst_end_x - command.dst_x)) {
                return -1;
            }

            if ((src_end_y - command.src_y) != (dst_end_y - command.dst_y)) {
                return -1;
            }

            if ((size_t)command.width > SIZE_MAX / (size_t)command.height) {
                return -1;
            }

            pixel_count = (size_t)command.width * command.height;

            if (pixel_count > SIZE_MAX / sizeof(*pixels)) {
                return -1;
            }

            pixels = malloc(pixel_count * sizeof(*pixels));

            if (pixels == NULL) {
                return -1;
            }

            for (uint32_t y = 0; y < command.height; y++) {
                for (uint32_t x = 0; x < command.width; x++) {
                    uint32_t color;
                    size_t index = (size_t)y * command.width + x;

                    if (mygpu_framebuffer_get_pixel(
                            gpu->framebuffer,
                            command.src_x + x,
                            command.src_y + y,
                            &color) != 0) {

                        free(pixels);
                        return -1;
                    }

                    pixels[index] = color;
                }
            }

            for (uint32_t y = 0; y < command.height; y++) {
                for (uint32_t x = 0; x < command.width; x++) {
                    size_t index = (size_t)y * command.width + x;

                    if (mygpu_framebuffer_set_pixel(
                        gpu->framebuffer,
                        command.dst_x + x,
                        command.dst_y + y,
                        pixels[index]) != 0) {
                        
                        free(pixels);
                        return -1;
                    }
                }
            }

            free(pixels);

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_PRESENT: {
            gpu->presented = 1;

            offset += sizeof(uint32_t);
            break;
        }
        
        default:
            return -1;
        }
    }

    return 0;
}