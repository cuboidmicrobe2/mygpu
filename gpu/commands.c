#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mygpu/commands.h"
#include "mygpu/gpu_internal.h"
#include "mygpu/rasterizer.h"
#include "mygpu/vertex.h"

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

int mygpu_command_buffer_reset(struct mygpu_command_buffer *buffer)
{
    if (buffer == NULL) {
        return -1;
    }

    buffer->used = 0;

    return 0;
}

int mygpu_command_buffer_is_empty(const struct mygpu_command_buffer *buffer)
{
    if (buffer == NULL) {
        return -1;
    }

    return buffer->used == 0;
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

    if (buffer->used == 0) {
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
        case MYGPU_CMD_CLEAR: {
            if (buffer->used - offset < sizeof(struct mygpu_cmd_clear)) {
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_clear);
            break;
        }
        
        case MYGPU_CMD_DRAW_RECT: {
            struct mygpu_cmd_draw_rect command;

            if (buffer->used - offset < sizeof(struct mygpu_cmd_draw_rect)) {
                return -1;
            }

            memcpy(
                &command,
                buffer->data + offset,
                sizeof(command)
            );

            if (command.width == 0 || command.height == 0) {
                return -1;
            }

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_COPY: {
            if (buffer->used - offset <
                sizeof(struct mygpu_cmd_copy)) {
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_copy);
            break;
        }

        case MYGPU_CMD_PRESENT: {
            offset += sizeof(uint32_t);
            break;
        }

        case MYGPU_CMD_BUFFER_COPY: {
            if (buffer->used - offset <
                sizeof(struct mygpu_cmd_buffer_copy)) {
            
                return -1;
            }

            offset += sizeof(struct mygpu_cmd_buffer_copy);
            break;
        }

        case MYGPU_CMD_DRAW_TRIANGLES: {
            struct mygpu_cmd_draw_triangles command;

            if (buffer->used - offset < 
                sizeof(struct mygpu_cmd_draw_triangles)) {
                    
                return -1;
            }

            memcpy(
                &command, 
                buffer->data + offset, 
                sizeof(command))
            ;

            if (command.vertex_count == 0 ||
                command.vertex_count % 3 != 0) {
                return -1;
            }

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_DRAW_INDEXED: {
            struct mygpu_cmd_draw_indexed command;

            if (buffer->used - offset <
                sizeof(struct mygpu_cmd_draw_indexed)) {

                return -1;
            }

            memcpy(
                &command,
                buffer->data + offset,
                sizeof(command)
            );

            if (command.index_count == 0 ||
                command.index_count % 3 != 0) {

                return -1;
            }

            offset += sizeof(command);
            break;
        }
        
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
                command.y >= framebuffer_height) {

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

        case MYGPU_CMD_BUFFER_COPY: {
            struct mygpu_cmd_buffer_copy command;
            struct mygpu_buffer *source_buffer;
            struct mygpu_buffer *destination_buffer;
            size_t source_offset;
            size_t destination_offset;
            uint8_t *data;

            memcpy(
                &command,
                buffer->data + offset,
                sizeof(command)
            );

            if (command.size == 0) {
                offset += sizeof(command);
                break;
            }

            source_buffer = mygpu_buffer_lookup(gpu, command.src_address);
            if (source_buffer == NULL) {
                return -1;
            }

            destination_buffer = mygpu_buffer_lookup(gpu, command.dst_address);
            if (destination_buffer == NULL) {
                return -1;
            }

            source_offset = mygpu_buffer_offset(source_buffer, command.src_address);

            destination_offset = mygpu_buffer_offset(destination_buffer, command.dst_address);

            if (source_offset == SIZE_MAX ||
                destination_offset == SIZE_MAX) {
                    
                return -1;
            }

            if ((size_t)command.size >
                    mygpu_buffer_size(source_buffer) - source_offset ||
                (size_t)command.size >
                    mygpu_buffer_size(destination_buffer) - destination_offset) {

                return -1;
            }

            data = malloc(command.size);

            if (data == NULL) {
                return -1;
            }

            if (mygpu_memory_read(
                gpu->memory,
                command.src_address,
                data,
                command.size) != 0) {

                free(data);
                return -1;
            }

            if (mygpu_memory_write(
                gpu->memory,
                command.dst_address,
                data,
                command.size) != 0) {
                
                free(data);
                return -1;
            }

            free(data);

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_DRAW_TRIANGLES: {
            struct mygpu_cmd_draw_triangles command;

            struct mygpu_buffer *vertex_buffer;
            size_t buffer_offset;
            size_t required_size;
            uint64_t last_vertex;
            uint32_t vertex_address;

            memcpy(
                &command, 
                buffer->data + offset, 
                sizeof(command)
            );

            vertex_buffer = mygpu_buffer_lookup(gpu, command.vertex_address);

            if (vertex_buffer == NULL) {
                return -1;
            }

            buffer_offset = mygpu_buffer_offset(vertex_buffer, command.vertex_address);

            if (buffer_offset == SIZE_MAX) {
                return -1;
            }

            if (command.vertex_offset > mygpu_buffer_size(vertex_buffer) - buffer_offset) {
                return -1;
            }

            buffer_offset += command.vertex_offset;

            if (command.vertex_address > UINT32_MAX - command.vertex_offset) {
                return -1;
            }

            vertex_address = command.vertex_address + command.vertex_offset;

            last_vertex = (uint64_t)command.first_vertex + (uint64_t)command.vertex_count;

            if (last_vertex > SIZE_MAX / sizeof(struct mygpu_vertex)) {
                return -1;
            }

            required_size = (size_t)last_vertex * sizeof(struct mygpu_vertex);

            if (required_size > mygpu_buffer_size(vertex_buffer) - buffer_offset) {
                return -1;
            }

            for (uint32_t i = 0; i < command.vertex_count; i += 3) {
                uint32_t vertex_index = command.first_vertex + i;

                struct mygpu_vertex v0;
                struct mygpu_vertex v1;
                struct mygpu_vertex v2;

                if (mygpu_vertex_fetch(
                    gpu->memory, 
                    vertex_address, 
                    vertex_index, 
                    &v0) != 0) {

                    return -1;
                }

                if (mygpu_vertex_fetch(
                    gpu->memory, 
                    vertex_address, 
                    vertex_index + 1, 
                    &v1) != 0) {

                    return -1;
                }

                if (mygpu_vertex_fetch(
                    gpu->memory, 
                    vertex_address, 
                    vertex_index + 2, 
                    &v2) != 0) {

                    return -1;
                }

                if (mygpu_rasterize_triangle(
                    gpu->framebuffer,
                    &v0,
                    &v1,
                    &v2) != 0) {

                    return -1;
                }
            }

            offset += sizeof(command);
            break;
        }

        case MYGPU_CMD_DRAW_INDEXED: {
            struct mygpu_cmd_draw_indexed command;
            struct mygpu_buffer *vertex_buffer;
            struct mygpu_buffer *index_buffer;
            size_t vertex_offset;
            size_t index_offset;
            size_t required_index_size;
            uint64_t last_index;

            memcpy(
                &command,
                buffer->data + offset,
                sizeof(command)
            );

            vertex_buffer = mygpu_buffer_lookup(gpu, command.vertex_address);
            if (vertex_buffer == NULL) {
                return -1;
            }

            index_buffer = mygpu_buffer_lookup(gpu, command.index_address);
            if (index_buffer == NULL) {
                return -1;
            }

            vertex_offset = mygpu_buffer_offset(vertex_buffer, command.vertex_address);
            if (vertex_offset == SIZE_MAX) {
                return -1;
            }

            index_offset = mygpu_buffer_offset(index_buffer, command.index_address);
            if (index_offset == SIZE_MAX) {
                return -1;
            }

            last_index = command.first_index + command.index_count;

            if (last_index > UINT32_MAX) {
                return -1;
            }

            required_index_size = (size_t)last_index * sizeof(uint32_t);

            if (required_index_size > mygpu_buffer_size(index_buffer) - index_offset) {
                return -1;
            }

            for (uint32_t i = 0; i < command.index_count; i += 3) {
                uint32_t index_position = command.first_index + i;
                uint32_t index0;
                uint32_t index1;
                uint32_t index2;

                struct mygpu_vertex v0;
                struct mygpu_vertex v1;
                struct mygpu_vertex v2;

                if (mygpu_memory_read(
                    gpu->memory,
                    command.index_address + (index_position * sizeof(uint32_t)),
                    &index0,
                    sizeof(index0)) != 0) {
                    
                    return -1;
                }

                if (mygpu_memory_read(
                    gpu->memory,
                    command.index_address + ((index_position + 1) * sizeof(uint32_t)),
                    &index1,
                    sizeof(index1)) != 0) {
                    
                    return -1;
                }

                if (mygpu_memory_read(
                    gpu->memory,
                    command.index_address + ((index_position + 2) * sizeof(uint32_t)),
                    &index2,
                    sizeof(index2)) != 0) {
                    
                    return -1;
                }

                if ((size_t)index0 * sizeof(struct mygpu_vertex) >=
                    mygpu_buffer_size(vertex_buffer) - vertex_offset) {
                    
                    return -1;
                }

                if ((size_t)index1 * sizeof(struct mygpu_vertex) >=
                    mygpu_buffer_size(vertex_buffer) - vertex_offset) {
                    
                    return -1;
                }

                if ((size_t)index2 * sizeof(struct mygpu_vertex) >=
                    mygpu_buffer_size(vertex_buffer) - vertex_offset) {
                    
                    return -1;
                }

                if (mygpu_vertex_fetch(
                    gpu->memory,
                    command.vertex_address,
                    index0,
                    &v0) != 0) {

                    return -1;
                }

                if (mygpu_vertex_fetch(
                    gpu->memory,
                    command.vertex_address,
                    index1,
                    &v1) != 0) {

                    return -1;
                }

                if (mygpu_vertex_fetch(
                    gpu->memory,
                    command.vertex_address,
                    index2,
                    &v2) != 0) {

                    return -1;
                }

                if (mygpu_rasterize_triangle(
                    gpu->framebuffer,
                    &v0,
                    &v1,
                    &v2) != 0) {

                    return -1;
                }
            }

            offset += sizeof(command);
            break;
        }
        
        default:
            return -1;
        }
    }

    return 0;
}