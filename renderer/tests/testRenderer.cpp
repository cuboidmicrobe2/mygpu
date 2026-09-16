#include <cassert>
#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

static void TestRendererBasics()
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
}

static void TestBuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

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
}

static void TestCommandBufferClearAndReset()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);
    assert(commandBuffer->Valid());
    assert(commandBuffer->IsEmpty());

    constexpr uint32_t commandClearColor = 0x12345678u;

    assert(commandBuffer->Clear(commandClearColor));
    assert(!commandBuffer->IsEmpty());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == commandClearColor);

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->Clear(0x11223344u));
    assert(!commandBuffer->IsEmpty());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commandBuffer));

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x11223344u);
}

static void TestDrawRect()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);

    constexpr uint32_t rectColor = 0x55667788u;

    assert(commandBuffer->DrawRect(10, 10, 20, 20, rectColor));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    assert(renderer.GetPixel(10, 10, color));
    assert(color == rectColor);

    assert(renderer.GetPixel(20, 20, color));
    assert(color == rectColor);

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(!commandBuffer->DrawRect(10, 10, 0, 20, rectColor));

    assert(!commandBuffer->DrawRect(10, 10, 20, 0, rectColor));
}

static void TestMultipleCommands()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);

    constexpr uint32_t firstRectColor = 0x11112222u;
    constexpr uint32_t secondRectColor = 0x33334444u;

    assert(commandBuffer->Clear(0x00000000u));

    assert(commandBuffer->DrawRect(5, 5, 10, 10, firstRectColor));

    assert(commandBuffer->DrawRect(30, 5, 10, 10, secondRectColor));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    assert(renderer.GetPixel(5, 5, color));
    assert(color == firstRectColor);

    assert(renderer.GetPixel(30, 5, color));
    assert(color == secondRectColor);

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x00000000u);
}

static void TestMultipleTriangles()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu},
        {70.0f, 10.0f, 0xFFFFFFFFu},
        {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    assert(vertexBuffer != nullptr);
    assert(vertexBuffer->Valid());
    assert(vertexBuffer->Size() == sizeof(vertices));

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->DrawTriangles(6));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(20, 15, color));
    assert(color == 0xFFFFFFFFu);

    assert(renderer.GetPixel(60, 15, color));
    assert(color == 0xFFFFFFFFu);
}

static void TestBufferLifetime()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);

    assert(vertexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);

    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->DrawTriangles(3));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));
}

static void TestTriangleCommandBuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu},
        {70.0f, 10.0f, 0xFFFFFFFFu},
        {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    assert(vertexBuffer != nullptr);

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);
    assert(commandBuffer->Valid());
    assert(commandBuffer->IsEmpty());

    assert(commandBuffer->Clear(0x00000000u));
    assert(!commandBuffer->IsEmpty());

    assert(commandBuffer->BindVertexBuffer(*vertexBuffer));
    assert(commandBuffer->DrawTriangles(6));

    assert(!commandBuffer->IsEmpty());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    assert(renderer.GetPixel(20, 15, color));
    assert(color == 0xFFFFFFFFu);

    assert(renderer.GetPixel(60, 15, color));
    assert(color == 0xFFFFFFFFu);

    auto tooSmallVertexBuffer = renderer.CreateBuffer(sizeof(myrenderer::Vertex) * 2);

    assert(tooSmallVertexBuffer != nullptr);

    assert(commandBuffer->BindVertexBuffer(*tooSmallVertexBuffer));
    assert(!commandBuffer->DrawTriangles(3));
}

static void TestFrameLifecycle()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto frameCommands = renderer.CreateCommandBuffer(256);

    assert(frameCommands != nullptr);
    assert(frameCommands->Valid());
    assert(frameCommands->IsEmpty());

    assert(renderer.BeginFrame());

    assert(frameCommands->Clear(0xCAFEBABEu));
    assert(!frameCommands->IsEmpty());

    assert(renderer.EndFrame(*frameCommands));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0xCAFEBABEu);

    assert(!renderer.EndFrame(*frameCommands));

    assert(renderer.BeginFrame());
    assert(!renderer.BeginFrame());

    assert(frameCommands->Reset());
    assert(frameCommands->Clear(0xDEADBEEFu));

    assert(renderer.EndFrame(*frameCommands));
    assert(!renderer.EndFrame(*frameCommands));
}

static void TestFrameCommandValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Clear(0x01020304u));

    assert(!renderer.EndFrame(*commands));

    assert(renderer.BeginFrame());

    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x01020304u);
}

static void TestCommandBufferResetState()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);

    assert(vertexBuffer != nullptr);

    auto commandBuffer = renderer.CreateCommandBuffer(256);

    assert(commandBuffer != nullptr);

    assert(commandBuffer->BindVertexBuffer(*vertexBuffer));
    assert(commandBuffer->DrawTriangles(3));

    assert(commandBuffer->Reset());
    assert(commandBuffer->IsEmpty());

    assert(!commandBuffer->DrawTriangles(3));
}

static void TestIndexedTriangle()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu},
        {70.0f, 10.0f, 0xFFFFFFFFu},
        {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    assert(vertexBuffer != nullptr);

    const uint32_t indices[3] = {0, 1, 2};

    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    assert(indexBuffer != nullptr);
    assert(indexBuffer->Valid());
    assert(indexBuffer->Size() == sizeof(indices));

    assert(indexBuffer->Write(0, indices, sizeof(indices)));

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->BindIndexBuffer(*indexBuffer));
    assert(commands->DrawIndexed(3));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(20, 15, color));
    assert(color == 0xFFFFFFFFu);
}

static void TestIndexedDrawValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu},
        {70.0f, 10.0f, 0xFFFFFFFFu},
        {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    assert(vertexBuffer != nullptr);

    const uint32_t indices[3] = {0, 1, 2};

    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    assert(indexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);

    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->BindIndexBuffer(*indexBuffer));

    assert(!commands->DrawIndexed(0));
    assert(!commands->DrawIndexed(4));
    assert(commands->DrawIndexed(3));

    auto tooSmallIndexBuffer = renderer.CreateBuffer(sizeof(uint32_t) * 2);

    assert(tooSmallIndexBuffer != nullptr);

    assert(commands->BindIndexBuffer(*tooSmallIndexBuffer));
    assert(!commands->DrawIndexed(3));

    assert(commands->Reset());

    assert(!commands->DrawIndexed(3));

    assert(commands->BindVertexBuffer(*vertexBuffer));

    assert(!commands->DrawIndexed(3));

    assert(commands->BindIndexBuffer(*indexBuffer));

    assert(commands->DrawIndexed(3));

    assert(commands->Reset());
    assert(commands->IsEmpty());

    assert(!commands->DrawIndexed(3));
}

static void TestCopyBuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    assert(source != nullptr);
    assert(destination != nullptr);

    const uint32_t sourceValues[3] = {
        0x11111111u,
        0x22222222u,
        0x33333333u};

    uint32_t destinationValues[3] = {};

    assert(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->CopyBuffer(*source, 0, *destination, 0, sizeof(sourceValues)));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(destination->Read(0, destinationValues, sizeof(destinationValues)));

    assert(destinationValues[0] == sourceValues[0]);
    assert(destinationValues[1] == sourceValues[1]);
    assert(destinationValues[2] == sourceValues[2]);
}

static void TestCopyBufferWithOffsets()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 4);
    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 4);

    assert(source != nullptr);
    assert(destination != nullptr);

    const uint32_t sourceValues[4] = {
        0x11111111u,
        0x22222222u,
        0x33333333u,
        0x44444444u};

    uint32_t destinationValues[4] = {};

    assert(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->CopyBuffer(*source, sizeof(uint32_t), *destination, sizeof(uint32_t), sizeof(uint32_t) * 2));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(destination->Read(0, destinationValues, sizeof(destinationValues)));

    assert(destinationValues[0] == 0);
    assert(destinationValues[1] == sourceValues[1]);
    assert(destinationValues[2] == sourceValues[2]);
    assert(destinationValues[3] == 0);
}

static void TestCopyBufferValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t));
    auto destination = renderer.CreateBuffer(sizeof(uint32_t));

    assert(source != nullptr);
    assert(destination != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(!commands->CopyBuffer(*source, 0, *destination, 0, 0));
    assert(!commands->CopyBuffer(*source, sizeof(uint32_t), *destination, 0, sizeof(uint32_t)));
    assert(!commands->CopyBuffer(*source, 0, *destination, sizeof(uint32_t), sizeof(uint32_t)));
    assert(!commands->CopyBuffer(*source, 0, *destination, 0, sizeof(uint32_t) * 2));
}

static void TestCopyBufferDifferentSizes()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 4);
    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 2);

    assert(source != nullptr);
    assert(destination != nullptr);

    const uint32_t sourceValues[4] = {
        0x11111111u,
        0x22222222u,
        0x33333333u,
        0x44444444u};

    uint32_t destinationValues[2] = {};

    assert(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->CopyBuffer(*source, sizeof(uint32_t), *destination, 0, sizeof(uint32_t) * 2));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(destination->Read(0, destinationValues, sizeof(destinationValues)));

    assert(destinationValues[0] == sourceValues[1]);
    assert(destinationValues[1] == sourceValues[2]);
}

static void TestPresented()
{
    myrenderer::Renderer renderer;
    myrenderer::CommandBuffer *invalidCommands = nullptr;

    assert(!invalidCommands);

    assert(renderer.Valid());
    assert(!renderer.Presented());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());
    assert(commands->Clear(0x12345678u));
    assert(commands->Present());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(renderer.Presented());
}

static void TestNotPresented()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());
    assert(!renderer.Presented());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());
    assert(commands->Clear(0x12345678u));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(!renderer.Presented());
}

static void TestCopyValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(!commands->Copy(0, 0, 0, 0, 0, 10));
    assert(!commands->Copy(0, 0, 0, 0, 10, 0));
}

static void TestCopy()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Clear(0x00000000u));
    assert(commands->DrawRect(1, 1, 2, 2, 0xFF0000FFu));
    assert(commands->Copy(1, 1, 5, 5, 2, 2));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(5, 5, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(6, 5, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(5, 6, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(6, 6, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x00000000u);
}

static void TestCopyOverlap()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Clear(0x00000000u));
    assert(commands->DrawRect(1, 1, 3, 1, 0xFF0000FFu));
    assert(commands->Copy(1, 1, 2, 1, 3, 1));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(1, 1, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(2, 1, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(3, 1, color));
    assert(color == 0xFF0000FFu);

    assert(renderer.GetPixel(4, 1, color));
    assert(color == 0xFF0000FFu);
}

static void TestCommandBufferReuse()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());
    assert(commands->Clear(0x12345678u));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));
}

static void TestPresentedAcrossFrames()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());
    assert(commands->Present());

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));
    assert(renderer.Presented());

    assert(commands->Reset());
    assert(commands->Clear(0x12345678u));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    assert(!renderer.Presented());
}

static void TestBeginFramePreservesFramebuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());
    assert(commands->Clear(0x12345678u));

    assert(renderer.BeginFrame());
    assert(renderer.EndFrame(*commands));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x12345678u);

    assert(renderer.BeginFrame());

    assert(renderer.GetPixel(0, 0, color));
    assert(color == 0x12345678u);

    assert(!renderer.Presented());
}

static void TestWriteIndices()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto buffer = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    assert(buffer != nullptr);
    assert(buffer->Valid());

    const uint32_t indices[3] = {0, 1, 2};
    uint32_t readBack[3] = {};

    assert(buffer->WriteIndices(indices, 3));
    assert(buffer->Read(0, readBack, sizeof(readBack)));

    assert(readBack[0] == 0);
    assert(readBack[1] == 1);
    assert(readBack[2] == 2);
}

static void TestWriteIndicesValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto buffer = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    assert(buffer != nullptr);
    assert(buffer->Valid());

    const uint32_t indices[3] = {0, 1, 2};

    assert(!buffer->WriteIndices(nullptr, 3));
    assert(!buffer->WriteIndices(indices, 0));
}

static void TestCreateIndexBuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const uint32_t indices[3] = {0, 1, 2};

    auto buffer = renderer.CreateIndexBuffer(indices, 3);

    assert(buffer != nullptr);
    assert(buffer->Valid());
    assert(buffer->Size() == sizeof(indices));

    uint32_t readBack[3] = {};

    assert(buffer->Read(0, readBack, sizeof(readBack)));

    assert(readBack[0] == 0);
    assert(readBack[1] == 1);
    assert(readBack[2] == 2);
}

static void TestCreateIndexBufferValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const uint32_t indices[3] = {0, 1, 2};

    assert(renderer.CreateIndexBuffer(nullptr, 3) == nullptr);
    assert(renderer.CreateIndexBuffer(indices, 0) == nullptr);
}

static void TestCreateVertexBuffer()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto buffer = renderer.CreateVertexBuffer(vertices, 3);

    assert(buffer != nullptr);
    assert(buffer->Valid());
    assert(buffer->Size() == sizeof(vertices));

    myrenderer::Vertex readBack[3] = {};

    assert(buffer->Read(0, readBack, sizeof(readBack)));

    assert(readBack[0].x == 10.0f);
    assert(readBack[0].y == 10.0f);
    assert(readBack[1].x == 30.0f);
    assert(readBack[2].y == 30.0f);
}

static void TestCreateVertexBufferValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    assert(renderer.CreateVertexBuffer(nullptr, 3) == nullptr);
    assert(renderer.CreateVertexBuffer(vertices, 0) == nullptr);
}

static void TestClearValidation()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Clear(0x12345678u));

    assert(commands->Reset());
    assert(commands->IsEmpty());
}

static void TestCommandBufferCapacity()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(sizeof(uint32_t));

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Present());
    assert(!commands->Present());
}

static void TestCommandBufferResetCapacity()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(sizeof(uint32_t));

    assert(commands != nullptr);
    assert(commands->Valid());

    assert(commands->Present());
    assert(!commands->Present());

    assert(commands->Reset());
    assert(commands->IsEmpty());

    assert(commands->Present());
}

static void TestResetClearsBindings()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);

    assert(vertexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->DrawTriangles(3));

    assert(commands->Reset());

    assert(!commands->DrawTriangles(3));
}

static void TestResetClearsIndexBinding()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    const uint32_t indices[3] = {0, 1, 2};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);
    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    assert(vertexBuffer != nullptr);
    assert(indexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->BindIndexBuffer(*indexBuffer));
    assert(commands->DrawIndexed(3));

    assert(commands->Reset());

    assert(!commands->DrawIndexed(3));
}

static void TestResetClearsAllBindings()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {
        {10.0f, 10.0f, 0xFFFFFFFFu},
        {30.0f, 10.0f, 0xFFFFFFFFu},
        {20.0f, 30.0f, 0xFFFFFFFFu}};

    const uint32_t indices[3] = {0, 1, 2};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);
    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    assert(vertexBuffer != nullptr);
    assert(indexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    assert(commands != nullptr);
    assert(commands->BindVertexBuffer(*vertexBuffer));
    assert(commands->BindIndexBuffer(*indexBuffer));
    assert(commands->DrawIndexed(3));

    assert(commands->Reset());

    assert(!commands->DrawTriangles(3));
    assert(!commands->DrawIndexed(3));
}

int main()
{
    TestRendererBasics();
    TestBuffer();
    TestCommandBufferClearAndReset();

    TestDrawRect();
    TestMultipleCommands();

    TestMultipleTriangles();
    TestBufferLifetime();
    TestTriangleCommandBuffer();

    TestFrameLifecycle();
    TestFrameCommandValidation();

    TestCommandBufferResetState();

    TestIndexedTriangle();
    TestIndexedDrawValidation();

    TestCopyBuffer();
    TestCopyBufferWithOffsets();
    TestCopyBufferValidation();
    TestCopyBufferDifferentSizes();

    TestPresented();
    TestNotPresented();

    TestCopyValidation();
    TestCopy();
    TestCopyOverlap();

    TestCommandBufferReuse();
    TestPresentedAcrossFrames();

    TestBeginFramePreservesFramebuffer();

    TestWriteIndices();
    TestWriteIndicesValidation();

    TestCreateIndexBuffer();
    TestCreateIndexBufferValidation();

    TestCreateVertexBuffer();
    TestCreateVertexBufferValidation();

    TestClearValidation();

    TestCommandBufferCapacity();

    TestCommandBufferResetCapacity();

    TestResetClearsBindings();
    TestResetClearsIndexBinding();
    TestResetClearsAllBindings();

    return 0;
}