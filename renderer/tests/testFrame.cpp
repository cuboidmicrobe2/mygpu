#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

#include "test.hpp"

static void TestRendererBasics() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    REQUIRE(renderer.Width() > 0);
    REQUIRE(renderer.Height() > 0);

    constexpr uint32_t clearColor = 0xAABBCCDDu;

    REQUIRE(renderer.Clear(clearColor));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == clearColor);
}

static void TestFrameLifecycle() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto frameCommands = renderer.CreateCommandBuffer(256);

    REQUIRE(frameCommands != nullptr);
    REQUIRE(frameCommands->Valid());
    REQUIRE(frameCommands->IsEmpty());

    REQUIRE(renderer.BeginFrame());

    REQUIRE(frameCommands->Clear(0xCAFEBABEu));
    REQUIRE(!frameCommands->IsEmpty());

    REQUIRE(renderer.EndFrame(*frameCommands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0xCAFEBABEu);

    REQUIRE(!renderer.EndFrame(*frameCommands));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(!renderer.BeginFrame());

    REQUIRE(frameCommands->Reset());
    REQUIRE(frameCommands->Clear(0xDEADBEEFu));

    REQUIRE(renderer.EndFrame(*frameCommands));
    REQUIRE(!renderer.EndFrame(*frameCommands));
}

static void TestFrameCommandValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Clear(0x01020304u));

    REQUIRE(!renderer.EndFrame(*commands));

    REQUIRE(renderer.BeginFrame());

    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x01020304u);
}

static void TestPresented() {
    myrenderer::Renderer renderer;
    myrenderer::CommandBuffer *invalidCommands = nullptr;

    REQUIRE(!invalidCommands);

    REQUIRE(renderer.Valid());
    REQUIRE(!renderer.Presented());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());
    REQUIRE(commands->Clear(0x12345678u));
    REQUIRE(commands->Present());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(renderer.Presented());
}

static void TestNotPresented() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());
    REQUIRE(!renderer.Presented());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());
    REQUIRE(commands->Clear(0x12345678u));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(!renderer.Presented());
}

static void TestPresentedAcrossFrames() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());
    REQUIRE(commands->Present());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));
    REQUIRE(renderer.Presented());

    REQUIRE(commands->Reset());
    REQUIRE(commands->Clear(0x12345678u));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(!renderer.Presented());
}

static void TestBeginFramePreservesFramebuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());
    REQUIRE(commands->Clear(0x12345678u));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x12345678u);

    REQUIRE(renderer.BeginFrame());

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x12345678u);

    REQUIRE(!renderer.Presented());
}

static void TestClearValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Clear(0x12345678u));

    REQUIRE(commands->Reset());
    REQUIRE(commands->IsEmpty());
}

int main() {
    RUN_TEST(TestRendererBasics);
    RUN_TEST(TestFrameLifecycle);
    RUN_TEST(TestFrameCommandValidation);
    RUN_TEST(TestPresented);
    RUN_TEST(TestNotPresented);
    RUN_TEST(TestPresentedAcrossFrames);
    RUN_TEST(TestBeginFramePreservesFramebuffer);
    RUN_TEST(TestClearValidation);

    return TEST_FINISH();
}
