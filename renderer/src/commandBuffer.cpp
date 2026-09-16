#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/vertex.hpp"

extern "C"
{
#include "mygpu/commands.h"
}

namespace myrenderer
{
    CommandBuffer::CommandBuffer(struct mygpu_command_buffer *CommandBuffer)
        : m_commandBuffer(CommandBuffer),
          m_vertexBuffer(nullptr),
          m_indexBuffer(nullptr) {}

    CommandBuffer::~CommandBuffer()
    {
        if (m_commandBuffer != nullptr)
        {
            mygpu_command_buffer_destroy(m_commandBuffer);
            m_commandBuffer = nullptr;
        }
    }

    bool CommandBuffer::Valid() const
    {
        return m_commandBuffer != nullptr;
    }

    bool CommandBuffer::IsEmpty() const
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        return mygpu_command_buffer_is_empty(m_commandBuffer) == 1;
    }

    bool CommandBuffer::Reset()
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        if (mygpu_command_buffer_reset(m_commandBuffer) != 0)
        {
            return false;
        }

        m_vertexBuffer = nullptr;
        m_indexBuffer = nullptr;

        return true;
    }

    bool CommandBuffer::Clear(uint32_t color)
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        struct mygpu_cmd_clear command;

        command.opcode = MYGPU_CMD_CLEAR;
        command.color = color;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::Copy(uint32_t srcX, uint32_t srcY, uint32_t dstX, uint32_t dstY, uint32_t width, uint32_t height)
    {
        if (m_commandBuffer == nullptr || width == 0 || height == 0)
        {
            return false;
        }

        struct mygpu_cmd_copy command;

        command.opcode = MYGPU_CMD_COPY;
        command.src_x = srcX;
        command.src_y = srcY;
        command.dst_x = dstX;
        command.dst_y = dstY;
        command.width = width;
        command.height = height;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::Present()
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        const uint32_t opcode = MYGPU_CMD_PRESENT;

        return mygpu_command_buffer_write(m_commandBuffer, &opcode, sizeof(opcode)) == 0;
    }

    bool CommandBuffer::DrawRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color)
    {
        if (m_commandBuffer == nullptr || width == 0 || height == 0)
        {
            return false;
        }

        struct mygpu_cmd_draw_rect command;

        command.opcode = MYGPU_CMD_DRAW_RECT;
        command.x = x;
        command.y = y;
        command.width = width;
        command.height = height;
        command.color = color;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::BindVertexBuffer(const Buffer &vertexBuffer)
    {
        if (m_commandBuffer == nullptr || !vertexBuffer.Valid())
        {
            return false;
        }

        m_vertexBuffer = &vertexBuffer;

        return true;
    }

    bool CommandBuffer::DrawTriangles(uint32_t vertexCount)
    {
        if (m_commandBuffer == nullptr || m_vertexBuffer == nullptr || vertexCount == 0)
        {
            return false;
        }

        if (vertexCount > SIZE_MAX / sizeof(Vertex))
        {
            return false;
        }

        if (vertexCount % 3 != 0)
        {
            return false;
        }

        const size_t requiredSize = static_cast<size_t>(vertexCount) * sizeof(Vertex);

        if (m_vertexBuffer->Size() < requiredSize)
        {
            return false;
        }

        struct mygpu_cmd_draw_triangles command;

        command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
        command.vertex_address = m_vertexBuffer->Address();
        command.vertex_count = vertexCount;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::BindIndexBuffer(const Buffer &indexBuffer)
    {
        if (m_commandBuffer == nullptr || !indexBuffer.Valid())
        {
            return false;
        }

        m_indexBuffer = &indexBuffer;

        return true;
    }

    bool CommandBuffer::DrawIndexed(uint32_t indexCount)
    {
        if (m_commandBuffer == nullptr ||
            m_vertexBuffer == nullptr ||
            m_indexBuffer == nullptr ||
            indexCount == 0)
        {
            return false;
        }

        if (indexCount % 3 != 0)
        {
            return false;
        }

        if (indexCount > SIZE_MAX / sizeof(uint32_t))
        {
            return false;
        }

        const size_t requiredSize = static_cast<size_t>(indexCount) * sizeof(uint32_t);

        if (m_indexBuffer->Size() < requiredSize)
        {
            return false;
        }

        struct mygpu_cmd_draw_indexed command;

        command.opcode = MYGPU_CMD_DRAW_INDEXED;
        command.vertex_address = m_vertexBuffer->Address();
        command.index_address = m_indexBuffer->Address();
        command.index_count = indexCount;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::CopyBuffer(const Buffer &source, size_t sourceOffset, const Buffer &destination, size_t destinationOffset, size_t size)
    {
        if (m_commandBuffer == nullptr ||
            !source.Valid() ||
            !destination.Valid() ||
            size == 0)
        {
            return false;
        }

        if (sourceOffset > source.Size() ||
            size > source.Size() - sourceOffset)
        {
            return false;
        }

        if (destinationOffset > destination.Size() ||
            size > destination.Size() - destinationOffset)
        {
            return false;
        }

        if (sourceOffset > UINT32_MAX ||
            destinationOffset > UINT32_MAX)
        {
            return false;
        }

        const uint64_t sourceAddress = static_cast<uint64_t>(source.Address()) + sourceOffset;

        const uint64_t destinationAddress = static_cast<uint64_t>(destination.Address()) + destinationOffset;

        if (sourceAddress > UINT32_MAX ||
            destinationAddress > UINT32_MAX)
        {
            return false;
        }

        if (size > UINT32_MAX)
        {
            return false;
        }

        struct mygpu_cmd_buffer_copy command;

        command.opcode = MYGPU_CMD_BUFFER_COPY;
        command.src_address = static_cast<uint32_t>(sourceAddress);
        command.dst_address = static_cast<uint32_t>(destinationAddress);
        command.size = static_cast<uint32_t>(size);

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

} // namespace myrenderer
