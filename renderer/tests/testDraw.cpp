#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

#include "test.hpp"

static void TestDrawRect() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(commandBuffer != nullptr);

    constexpr uint32_t rectColor = 0x55667788u;

    REQUIRE(commandBuffer->DrawRect(10, 10, 20, 20, rectColor));

    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(10, 10, color));
    REQUIRE(color == rectColor);

    REQUIRE(renderer.GetPixel(20, 20, color));
    REQUIRE(color == rectColor);

    REQUIRE(commandBuffer->Reset());
    REQUIRE(commandBuffer->IsEmpty());

    REQUIRE(!commandBuffer->DrawRect(10, 10, 0, 20, rectColor));

    REQUIRE(!commandBuffer->DrawRect(10, 10, 20, 0, rectColor));
}

static void TestMultipleTriangles() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu}, {30.0f, 10.0f, 0xFFFFFFFFu}, {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu}, {70.0f, 10.0f, 0xFFFFFFFFu}, {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    REQUIRE(vertexBuffer != nullptr);
    REQUIRE(vertexBuffer->Valid());
    REQUIRE(vertexBuffer->Size() == sizeof(vertices));

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->DrawTriangles(6, 0));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color == 0xFFFFFFFFu);

    REQUIRE(renderer.GetPixel(60, 15, color));
    REQUIRE(color == 0xFFFFFFFFu);
}

static void TestFirstVertex() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFF0000FFu}, {30.0f, 10.0f, 0xFF0000FFu}, {20.0f, 30.0f, 0xFF0000FFu},

        {50.0f, 10.0f, 0x00FF00FFu}, {70.0f, 10.0f, 0x00FF00FFu}, {60.0f, 30.0f, 0x00FF00FFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    REQUIRE(vertexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->DrawTriangles(3, 3));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(60, 15, color));
    REQUIRE(color == 0x00FF00FFu);

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color != 0xFF0000FFu);
}

static void TestVertexBufferOffset() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());
    REQUIRE(renderer.BeginFrame());

    const myrenderer::Vertex vertices[] = {
        {3.0f, 3.0f, 0xFFFF0000},   {13.0f, 3.0f, 0xFFFF0000},  {8.0f, 13.0f, 0xFFFF0000},

        {21.0f, 21.0f, 0xFF00FF00}, {31.0f, 21.0f, 0xFF00FF00}, {26.0f, 31.0f, 0xFF00FF00}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);
    REQUIRE(vertexBuffer);
    REQUIRE(vertexBuffer->Valid());

    auto commands = renderer.CreateCommandBuffer(1024);
    REQUIRE(commands);
    REQUIRE(commands->Valid());

    const size_t offset = 3 * sizeof(myrenderer::Vertex);

    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, offset));
    REQUIRE(commands->DrawTriangles(3, 0));
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(26, 24, color));
    REQUIRE(color == 0xFF00FF00);

    REQUIRE(renderer.GetPixel(8, 6, color));
    REQUIRE(color != 0xFFFF0000);
}

static void TestVertexBufferOffsetValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertex = {0.0f, 0.0f, 0xFFFFFFFF};

    auto vertexBuffer = renderer.CreateVertexBuffer(&vertex, 1);
    REQUIRE(vertexBuffer);
    REQUIRE(vertexBuffer->Valid());

    auto commands = renderer.CreateCommandBuffer(1024);
    REQUIRE(commands);
    REQUIRE(commands->Valid());

    REQUIRE(!commands->BindVertexBuffer(*vertexBuffer, vertexBuffer->Size() + 1));
}

static void TestTriangleCommandBuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu}, {30.0f, 10.0f, 0xFFFFFFFFu}, {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu}, {70.0f, 10.0f, 0xFFFFFFFFu}, {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    REQUIRE(vertexBuffer != nullptr);

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(commandBuffer != nullptr);
    REQUIRE(commandBuffer->Valid());
    REQUIRE(commandBuffer->IsEmpty());

    REQUIRE(commandBuffer->Clear(0x00000000u));
    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(commandBuffer->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commandBuffer->DrawTriangles(6, 0));

    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color == 0xFFFFFFFFu);

    REQUIRE(renderer.GetPixel(60, 15, color));
    REQUIRE(color == 0xFFFFFFFFu);

    auto tooSmallVertexBuffer = renderer.CreateBuffer(sizeof(myrenderer::Vertex) * 2);

    REQUIRE(tooSmallVertexBuffer != nullptr);

    REQUIRE(commandBuffer->BindVertexBuffer(*tooSmallVertexBuffer, 0));
    REQUIRE(!commandBuffer->DrawTriangles(3, 0));
}

static void TestIndexedTriangle() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu}, {30.0f, 10.0f, 0xFFFFFFFFu}, {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu}, {70.0f, 10.0f, 0xFFFFFFFFu}, {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    REQUIRE(vertexBuffer != nullptr);

    const uint32_t indices[3] = {0, 1, 2};

    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    REQUIRE(indexBuffer != nullptr);
    REQUIRE(indexBuffer->Valid());
    REQUIRE(indexBuffer->Size() == sizeof(indices));

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->BindIndexBuffer(*indexBuffer));
    REQUIRE(commands->DrawIndexed(3, 0));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color == 0xFFFFFFFFu);
}

static void TestIndexedDrawValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[6] = {
        {10.0f, 10.0f, 0xFFFFFFFFu}, {30.0f, 10.0f, 0xFFFFFFFFu}, {20.0f, 30.0f, 0xFFFFFFFFu},

        {50.0f, 10.0f, 0xFFFFFFFFu}, {70.0f, 10.0f, 0xFFFFFFFFu}, {60.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);

    REQUIRE(vertexBuffer != nullptr);

    const uint32_t indices[3] = {0, 1, 2};

    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    REQUIRE(indexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);

    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->BindIndexBuffer(*indexBuffer));

    REQUIRE(!commands->DrawIndexed(0, 0));
    REQUIRE(!commands->DrawIndexed(4, 0));
    REQUIRE(commands->DrawIndexed(3, 0));

    auto tooSmallIndexBuffer = renderer.CreateBuffer(sizeof(uint32_t) * 2);

    REQUIRE(tooSmallIndexBuffer != nullptr);

    REQUIRE(commands->BindIndexBuffer(*tooSmallIndexBuffer));
    REQUIRE(!commands->DrawIndexed(3, 0));

    REQUIRE(commands->Reset());

    REQUIRE(!commands->DrawIndexed(3, 0));

    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));

    REQUIRE(!commands->DrawIndexed(3, 0));

    REQUIRE(commands->BindIndexBuffer(*indexBuffer));

    REQUIRE(commands->DrawIndexed(3, 0));

    REQUIRE(commands->Reset());
    REQUIRE(commands->IsEmpty());

    REQUIRE(!commands->DrawIndexed(3, 0));
}

static void TestIndexedFirstIndex() {
    myrenderer::Renderer renderer;
    REQUIRE(renderer.Valid());

    myrenderer::Vertex vertices[] = {
        {10.0f, 10.0f, 0xFF0000FF}, {30.0f, 10.0f, 0xFF0000FF}, {20.0f, 30.0f, 0xFF0000FF},

        {50.0f, 10.0f, 0x00FF00FF}, {70.0f, 10.0f, 0x00FF00FF}, {60.0f, 30.0f, 0x00FF00FF}};

    uint32_t indices[] = {0, 1, 2, 3, 4, 5};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);
    auto indexBuffer = renderer.CreateIndexBuffer(indices, 6);
    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(vertexBuffer != nullptr);
    REQUIRE(indexBuffer != nullptr);
    REQUIRE(commandBuffer != nullptr);

    REQUIRE(commandBuffer->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commandBuffer->BindIndexBuffer(*indexBuffer));
    REQUIRE(commandBuffer->DrawIndexed(3, 3));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(60, 15, color));
    REQUIRE(color == 0x00FF00FF);

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color != 0xFF0000FF);
}

static void TestIndexedVertexBufferOffset() {
    myrenderer::Renderer renderer;
    REQUIRE(renderer.Valid());

    myrenderer::Vertex vertices[] = {
        {10.0f, 10.0f, 0xFF0000FF}, {30.0f, 10.0f, 0xFF0000FF}, {20.0f, 30.0f, 0xFF0000FF},

        {50.0f, 10.0f, 0x00FF00FF}, {70.0f, 10.0f, 0x00FF00FF}, {60.0f, 30.0f, 0x00FF00FF}};

    uint32_t indices[] = {0, 1, 2};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 6);
    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);
    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(vertexBuffer != nullptr);
    REQUIRE(indexBuffer != nullptr);
    REQUIRE(commandBuffer != nullptr);

    const size_t offset = 3 * sizeof(myrenderer::Vertex);

    REQUIRE(commandBuffer->BindVertexBuffer(*vertexBuffer, offset));
    REQUIRE(commandBuffer->BindIndexBuffer(*indexBuffer));
    REQUIRE(commandBuffer->DrawIndexed(3, 0));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(60, 15, color));
    REQUIRE(color == 0x00FF00FF);

    REQUIRE(renderer.GetPixel(20, 15, color));
    REQUIRE(color != 0xFF0000FF);
}

static void TestDrawTrianglesRangeValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);
    auto commands = renderer.CreateCommandBuffer(1024);

    REQUIRE(vertexBuffer != nullptr);
    REQUIRE(commands != nullptr);

    // All three vertices fit
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->DrawTriangles(3, 0));

    // Starting at vertex 1
    REQUIRE(!commands->DrawTriangles(3, 1));

    // Huge first vertex
    REQUIRE(!commands->DrawTriangles(3, UINT32_MAX - 1));

    // Skipping one vertex
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, sizeof(myrenderer::Vertex)));
    REQUIRE(!commands->DrawTriangles(3, 0));
}

int main() {
    RUN_TEST(TestDrawRect);
    RUN_TEST(TestMultipleTriangles);
    RUN_TEST(TestFirstVertex);
    RUN_TEST(TestVertexBufferOffset);
    RUN_TEST(TestVertexBufferOffsetValidation);
    RUN_TEST(TestTriangleCommandBuffer);
    RUN_TEST(TestIndexedTriangle);
    RUN_TEST(TestIndexedDrawValidation);
    RUN_TEST(TestIndexedFirstIndex);
    RUN_TEST(TestIndexedVertexBufferOffset);
    RUN_TEST(TestDrawTrianglesRangeValidation);

    return TEST_FINISH();
}
