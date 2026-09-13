#include <cassert>
#include <cstdint>
#include <memory>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

int main()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    assert(renderer.Width() > 0);
    assert(renderer.Height() > 0);

    constexpr uint32_t clearColor = 0xAABBCCDDu;

    assert(renderer.Clear(clearColor));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == clearColor);

    auto buffer = renderer.CreateBuffer(3 * sizeof(uint32_t));

    assert(buffer != nullptr);
    assert(buffer->Valid());
    assert(buffer->Size() == 3 * sizeof(uint32_t));

    const uint32_t values[3] = {
        0x11111111u,
        0x22222222u,
        0x33333333u};

    assert(buffer->Write(0, values, sizeof(values)));

    uint32_t result[3] = {};

    assert(buffer->Read(0, result, sizeof(result)));

    assert(result[0] == values[0]);
    assert(result[1] == values[1]);
    assert(result[2] == values[2]);

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);
    assert(commandBuffer->Valid());
    assert(commandBuffer->IsEmpty());

    constexpr uint32_t commandClearColor = 0x12345678u;

    assert(commandBuffer->Clear(commandClearColor));
    assert(!commandBuffer->IsEmpty());

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(0, 0, color));
    assert(color == commandClearColor);

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->Clear(0x11223344u));
    assert(!commandBuffer->IsEmpty());

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x11223344u);

    // Test DrawRect.
    constexpr uint32_t rectColor = 0x55667788u;

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->DrawRect(10, 10, 20, 20, rectColor));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(10, 10, color));
    assert(color == rectColor);

    assert(renderer.GetPixel(20, 20, color));
    assert(color == rectColor);

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(!commandBuffer->DrawRect(10, 10, 0, 20, rectColor));

    assert(!commandBuffer->DrawRect(10, 10, 20, 0, rectColor));

    // Test multiple commands in one command buffer.
    constexpr uint32_t firstRectColor = 0x11112222u;
    constexpr uint32_t secondRectColor = 0x33334444u;

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->Clear(0x00000000u));

    assert(commandBuffer->DrawRect(5, 5, 10, 10, firstRectColor));

    assert(commandBuffer->DrawRect(30, 5, 10, 10, secondRectColor));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(5, 5, color));
    assert(color == firstRectColor);

    assert(renderer.GetPixel(30, 5, color));
    assert(color == secondRectColor);

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x00000000u);

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu},
        {70.0f, 10.0f, 0xFFFFFFFFu},
        {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateBuffer(sizeof(vertices));

    assert(vertexBuffer != nullptr);
    assert(vertexBuffer->Valid());

    assert(vertexBuffer->WriteVertices(vertices, 6));

    // Test mixed commands in one command buffer.
    constexpr uint32_t mixedRectColor = 0xABCDEF01u;

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->Clear(0x00000000u));

    assert(commandBuffer->DrawRect(5, 5, 10, 10, mixedRectColor));

    assert(commandBuffer->DrawTriangles(*vertexBuffer, 6));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(5, 5, color));
    assert(color == mixedRectColor);

    assert(renderer.GetPixel(20, 15, color));
    assert(color == 0xFFFFFFFFu);

    assert(renderer.GetPixel(60, 15, color));
    assert(color == 0xFFFFFFFFu);

    // Test triangle-only command buffer.
    auto triangleCommandBuffer = renderer.CreateCommandBuffer(1024);

    assert(triangleCommandBuffer != nullptr);
    assert(triangleCommandBuffer->Valid());
    assert(triangleCommandBuffer->IsEmpty());

    assert(triangleCommandBuffer->Clear(0x00000000u));
    assert(!triangleCommandBuffer->IsEmpty());

    assert(triangleCommandBuffer->DrawTriangles(*vertexBuffer, 6));

    assert(!triangleCommandBuffer->IsEmpty());

    assert(renderer.Submit(*triangleCommandBuffer));

    assert(renderer.GetPixel(20, 15, color));
    assert(color == 0xFFFFFFFFu);

    assert(renderer.GetPixel(60, 15, color));
    assert(color == 0xFFFFFFFFu);

    auto tooSmallVertexBuffer = renderer.CreateBuffer(sizeof(myrenderer::Vertex) * 2);

    assert(tooSmallVertexBuffer != nullptr);

    assert(!triangleCommandBuffer->DrawTriangles(*tooSmallVertexBuffer, 3));

    return 0;
}
