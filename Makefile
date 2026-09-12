CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude

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

BUFFER_SOURCES = \
	$(GPU_SOURCES)

VERTEX_SOURCES = \
	$(GPU_SOURCES)

COMMANDS_SOURCES = \
	$(GPU_SOURCES)

QUEUE_SOURCES = \
	$(GPU_SOURCES)

# --------------------------------------------------
# Test executables
# --------------------------------------------------

GPU_TEST = test_gpu
MEMORY_TEST = test_memory
REGISTERS_TEST = test_registers
FRAMEBUFFER_TEST = test_framebuffer
BUFFER_TEST = test_buffer
VERTEX_BUFFER_TEST = test_vertex_buffer
COMMANDS_TEST = test_commands
QUEUE_TEST = test_queue
DRAW_TRIANGLES_TEST = test_draw_triangles

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

.PHONY: all test \
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

all: $(GPU_TEST) \
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

$(GPU_TEST): $(GPU_SOURCES) $(GPU_TEST_SOURCE)
	$(CC) $(CFLAGS) $(GPU_SOURCES) $(GPU_TEST_SOURCE) -o $@

$(MEMORY_TEST): gpu/memory.c $(MEMORY_TEST_SOURCE)
	$(CC) $(CFLAGS) gpu/memory.c $(MEMORY_TEST_SOURCE) -o $@

$(REGISTERS_TEST): gpu/registers.c $(REGISTERS_TEST_SOURCE)
	$(CC) $(CFLAGS) gpu/registers.c $(REGISTERS_TEST_SOURCE) -o $@

$(FRAMEBUFFER_TEST): gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE)
	$(CC) $(CFLAGS) gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE) -o $@

$(BUFFER_TEST): $(BUFFER_SOURCES) $(BUFFER_TEST_SOURCE)
	$(CC) $(CFLAGS) $(BUFFER_SOURCES) $(BUFFER_TEST_SOURCE) -o $@

$(VERTEX_BUFFER_TEST): $(VERTEX_SOURCES) $(VERTEX_BUFFER_TEST_SOURCE)
	$(CC) $(CFLAGS) $(VERTEX_SOURCES) $(VERTEX_BUFFER_TEST_SOURCE) -o $@

$(COMMANDS_TEST): $(COMMANDS_SOURCES) $(COMMANDS_TEST_SOURCE)
	$(CC) $(CFLAGS) $(COMMANDS_SOURCES) $(COMMANDS_TEST_SOURCE) -o $@

$(QUEUE_TEST): $(QUEUE_SOURCES) $(QUEUE_TEST_SOURCE)
	$(CC) $(CFLAGS) $(QUEUE_SOURCES) $(QUEUE_TEST_SOURCE) -o $@

$(DRAW_TRIANGLES_TEST): $(GPU_SOURCES) $(DRAW_TRIANGLES_TEST_SOURCE)
	$(CC) $(CFLAGS) $(GPU_SOURCES) $(DRAW_TRIANGLES_TEST_SOURCE) -o $@

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
	test_draw_triangles

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
	rm -f \
	$(GPU_TEST) \
	$(MEMORY_TEST) \
	$(REGISTERS_TEST) \
	$(FRAMEBUFFER_TEST) \
	$(BUFFER_TEST) \
	$(VERTEX_BUFFER_TEST) \
	$(COMMANDS_TEST) \
	$(QUEUE_TEST) \
	$(DRAW_TRIANGLES_TEST)