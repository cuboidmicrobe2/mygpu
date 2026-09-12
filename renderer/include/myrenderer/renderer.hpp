#pragma once

#include <cstdint>

struct mygpu;

namespace myrenderer
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        bool Valid() const;

        uint32_t Width() const;
        uint32_t Height() const;

        bool Clear(uint32_t color);

        bool GetPixel(uint32_t x, uint32_t y, uint32_t &color) const;

    private:
        struct mygpu *m_gpu;
    };
} // namespace myrenderer