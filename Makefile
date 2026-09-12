CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude

BUILD_DIR = build
TEST_BUILD_DIR = $(BUILD_DIR)/tests
LIBRARY = $(BUILD_DIR)/libmygpu.a

# --------------------------------------------------
# GPU sources
# --------------------------------------------------

GPU_CORE_SOURCES = \
	gpu/memory.c \
	gpu/registers.c \
	gpu/framebuffer.c

GPU_SOURCES = \
	$(GPU_CORE_SOURCES) \
	gpu/gpu.c \
	gpu/fence.c \
	gpu/queue.c \
	gpu/commands.c \
	gpu/buffer.c \
	gpu/vertex.c \
	gpu/rasterizer.c

GPU_OBJECTS = \
	$(GPU_SOURCES:gpu/%.c=$(BUILD_DIR)/gpu/%.o)

# --------------------------------------------------
# Library
# --------------------------------------------------

$(BUILD_DIR)/gpu/%.o: gpu/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(GPU_OBJECTS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

library: $(LIBRARY)

# --------------------------------------------------
# Test executables
# --------------------------------------------------

GPU_TEST = $(TEST_BUILD_DIR)/test_gpu
MEMORY_TEST = $(TEST_BUILD_DIR)/test_memory
REGISTERS_TEST = $(TEST_BUILD_DIR)/test_registers
FRAMEBUFFER_TEST = $(TEST_BUILD_DIR)/test_framebuffer
BUFFER_TEST = $(TEST_BUILD_DIR)/test_buffer
VERTEX_BUFFER_TEST = $(TEST_BUILD_DIR)/test_vertex_buffer
COMMANDS_TEST = $(TEST_BUILD_DIR)/test_commands
QUEUE_TEST = $(TEST_BUILD_DIR)/test_queue
DRAW_TRIANGLES_TEST = $(TEST_BUILD_DIR)/test_draw_triangles

# --------------------------------------------------
# Test source files
# --------------------------------------------------

GPU_TEST_SOURCE = tests/test_gpu.c
MEMORY_TEST_SOURCE = tests/test_memory.c
REGISTERS_TEST_SOURCE = tests/test_registers.c
FRAMEBUFFER_TEST_SOURCE = tests/test_framebuffer.c
BUFFER_TEST_SOURCE = tests/test_buffer.c
VERTEX_BUFFER_TEST_SOURCE = tests/test_vertex_buffer.c
COMMANDS_TEST_SOURCE = tests/test_commands.c
QUEUE_TEST_SOURCE = tests/test_queue.c
DRAW_TRIANGLES_TEST_SOURCE = tests/test_draw_triangles.c

# --------------------------------------------------
# Phony targets
# --------------------------------------------------

.PHONY: all library test \
	test-gpu \
	test-memory \
	test-registers \
	test-framebuffer \
	test-buffer \
	test-vertex-buffer \
	test-commands \
	test-queue \
	test-draw-triangles \
	clean

# --------------------------------------------------
# Default
# --------------------------------------------------

all: $(LIBRARY) \
	$(GPU_TEST) \
	$(MEMORY_TEST) \
	$(REGISTERS_TEST) \
	$(FRAMEBUFFER_TEST) \
	$(BUFFER_TEST) \
	$(VERTEX_BUFFER_TEST) \
	$(COMMANDS_TEST) \
	$(QUEUE_TEST) \
	$(DRAW_TRIANGLES_TEST)

# --------------------------------------------------
# Build tests
# --------------------------------------------------

$(GPU_TEST): $(LIBRARY) $(GPU_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GPU_TEST_SOURCE) $(LIBRARY) -o $@

$(MEMORY_TEST): gpu/memory.c $(MEMORY_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/memory.c $(MEMORY_TEST_SOURCE) -o $@

$(REGISTERS_TEST): gpu/registers.c $(REGISTERS_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/registers.c $(REGISTERS_TEST_SOURCE) -o $@

$(FRAMEBUFFER_TEST): gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE) -o $@

$(BUFFER_TEST): $(LIBRARY) $(BUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(BUFFER_TEST_SOURCE) $(LIBRARY) -o $@

$(VERTEX_BUFFER_TEST): $(LIBRARY) $(VERTEX_BUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(VERTEX_BUFFER_TEST_SOURCE) $(LIBRARY) -o $@

$(COMMANDS_TEST): $(LIBRARY) $(COMMANDS_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(COMMANDS_TEST_SOURCE) $(LIBRARY) -o $@

$(QUEUE_TEST): $(LIBRARY) $(QUEUE_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(QUEUE_TEST_SOURCE) $(LIBRARY) -o $@

$(DRAW_TRIANGLES_TEST): $(LIBRARY) $(DRAW_TRIANGLES_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DRAW_TRIANGLES_TEST_SOURCE) $(LIBRARY) -o $@

# --------------------------------------------------
# Run tests
# --------------------------------------------------

test: test-gpu \
	test-memory \
	test-registers \
	test-framebuffer \
	test-buffer \
	test-vertex-buffer \
	test-commands \
	test-queue \
	test-draw-triangles

test-gpu: $(GPU_TEST)
	./$(GPU_TEST)

test-memory: $(MEMORY_TEST)
	./$(MEMORY_TEST)

test-registers: $(REGISTERS_TEST)
	./$(REGISTERS_TEST)

test-framebuffer: $(FRAMEBUFFER_TEST)
	./$(FRAMEBUFFER_TEST)

test-buffer: $(BUFFER_TEST)
	./$(BUFFER_TEST)

test-vertex-buffer: $(VERTEX_BUFFER_TEST)
	./$(VERTEX_BUFFER_TEST)

test-commands: $(COMMANDS_TEST)
	./$(COMMANDS_TEST)

test-queue: $(QUEUE_TEST)
	./$(QUEUE_TEST)

test-draw-triangles: $(DRAW_TRIANGLES_TEST)
	./$(DRAW_TRIANGLES_TEST)

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)