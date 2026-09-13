#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

extern "C"
{
#include "mygpu/buffer.h"
#include "mygpu/commands.h"
#include "mygpu/fence.h"
#include "mygpu/framebuffer.h"
#include "mygpu/gpu.h"
#include "mygpu/queue.h"
}

namespace myrenderer
{

    Renderer::Renderer()
        : m_gpu(mygpu_create()),
          m_queue(nullptr),
          m_fence(nullptr),
          m_frameActive(false)
    {
        if (m_gpu != nullptr)
        {
            m_queue = mygpu_queue_create();
        }

        if (m_queue != nullptr)
        {
            m_fence = mygpu_fence_create(1);
        }
    }

    Renderer::~Renderer()
    {
        if (m_fence != nullptr)
        {
            mygpu_fence_destroy(m_fence);
            m_fence = nullptr;
        }

        if (m_queue != nullptr)
        {
            mygpu_queue_destroy(m_queue);
            m_queue = nullptr;
        }

        if (m_gpu != nullptr)
        {
            mygpu_destroy(m_gpu);
            m_gpu = nullptr;
        }
    }

    bool Renderer::Valid() const
    {
        return m_gpu != nullptr && m_queue != nullptr && m_fence != nullptr;
    }

    uint32_t Renderer::Width() const
    {
        if (m_gpu == nullptr)
        {
            return 0;
        }

        struct mygpu_framebuffer *framebuffer = mygpu_get_framebuffer(m_gpu);

        if (framebuffer == nullptr)
        {
            return 0;
        }

        return mygpu_framebuffer_width(framebuffer);
    }

    uint32_t Renderer::Height() const
    {
        if (m_gpu == nullptr)
        {
            return 0;
        }

        struct mygpu_framebuffer *framebuffer = mygpu_get_framebuffer(m_gpu);

        if (framebuffer == nullptr)
        {
            return 0;
        }

        return mygpu_framebuffer_height(framebuffer);
    }

    bool Renderer::Clear(uint32_t color)
    {
        if (m_gpu == nullptr)
        {
            return false;
        }

        struct mygpu_command_buffer *commandBuffer = mygpu_command_buffer_create(sizeof(struct mygpu_cmd_clear));

        if (commandBuffer == nullptr)
        {
            return false;
        }

        struct mygpu_cmd_clear clearCommand;

        clearCommand.opcode = MYGPU_CMD_CLEAR;
        clearCommand.color = color;

        if (mygpu_command_buffer_write(commandBuffer, &clearCommand, sizeof(clearCommand)) != 0)
        {
            mygpu_command_buffer_destroy(commandBuffer);
            return false;
        }

        if (mygpu_command_buffer_validate(commandBuffer) != 0)
        {
            mygpu_command_buffer_destroy(commandBuffer);
            return false;
        }

        struct mygpu_queue *queue = mygpu_queue_create();

        if (queue == nullptr)
        {
            mygpu_queue_destroy(queue);
            return false;
        }

        struct mygpu_fence *fence = mygpu_fence_create(1);

        if (fence == nullptr)
        {
            mygpu_queue_destroy(queue);
            mygpu_command_buffer_destroy(commandBuffer);
            return false;
        }

        if (mygpu_queue_submit(queue, commandBuffer, fence) != 0)
        {
            mygpu_fence_destroy(fence);
            mygpu_queue_destroy(queue);
            mygpu_command_buffer_destroy(commandBuffer);

            return false;
        }

        if (mygpu_queue_process(m_gpu, queue) != 0)
        {
            mygpu_fence_destroy(fence);
            mygpu_queue_destroy(queue);
            mygpu_command_buffer_destroy(commandBuffer);

            return false;
        }

        bool success = mygpu_fence_is_signaled(fence) != 0;

        mygpu_fence_destroy(fence);
        mygpu_queue_destroy(queue);
        mygpu_command_buffer_destroy(commandBuffer);

        return success;
    }

    bool Renderer::BeginFrame()
    {
        if (!Valid() || m_frameActive)
        {
            return false;
        }

        m_frameActive = true;

        return true;
    }

    bool Renderer::EndFrame(const CommandBuffer &commandBuffer)
    {
        if (!Valid() || !m_frameActive || !commandBuffer.Valid())
        {
            return false;
        }

        const bool success = Submit(commandBuffer);

        m_frameActive = false;

        return success;
    }

    bool Renderer::GetPixel(uint32_t x, uint32_t y, uint32_t &color) const
    {
        if (m_gpu == nullptr)
        {
            return false;
        }

        struct mygpu_framebuffer *framebuffer = mygpu_get_framebuffer(m_gpu);

        if (framebuffer == nullptr)
        {
            return false;
        }

        return mygpu_framebuffer_get_pixel(framebuffer, x, y, &color) == 0;
    }

    std::unique_ptr<Buffer> Renderer::CreateBuffer(size_t size)
    {
        if (m_gpu == nullptr)
        {
            return nullptr;
        }

        struct mygpu_buffer *buffer = mygpu_buffer_create(m_gpu, size);

        if (buffer == nullptr)
        {
            return nullptr;
        }

        return std::unique_ptr<Buffer>(new Buffer(buffer));
    }

    std::unique_ptr<Buffer> Renderer::CreateVertexBuffer(const Vertex *vertices, size_t vertexCount)
    {
        if (!Valid() || vertices == nullptr || vertexCount == 0)
        {
            return nullptr;
        }

        if (vertexCount > SIZE_MAX / sizeof(Vertex))
        {
            return nullptr;
        }

        const size_t size = vertexCount * sizeof(Vertex);

        auto buffer = CreateBuffer(size);

        if (buffer == nullptr || !buffer->WriteVertices(vertices, vertexCount))
        {
            return nullptr;
        }

        return buffer;
    }

    std::unique_ptr<CommandBuffer> Renderer::CreateCommandBuffer(size_t size)
    {
        if (m_gpu == nullptr)
        {
            return nullptr;
        }

        struct mygpu_command_buffer *commandBuffer = mygpu_command_buffer_create(static_cast<uint32_t>(size));

        if (commandBuffer == nullptr)
        {
            return nullptr;
        }

        return std::unique_ptr<CommandBuffer>(new CommandBuffer(commandBuffer));
    }

    bool Renderer::Submit(const CommandBuffer &commandBuffer)
    {
        if (!Valid() || !commandBuffer.Valid())
        {
            return false;
        }

        mygpu_fence_reset(m_fence);

        if (mygpu_queue_submit(m_queue, commandBuffer.m_commandBuffer, m_fence) != 0)
        {
            return false;
        }

        if (mygpu_queue_process(m_gpu, m_queue) != 0)
        {
            return false;
        }

        return mygpu_fence_is_signaled(m_fence) != 0;
    }

} // namespace myrenderer