CC = gcc
CFLAGS = -Wall -g -I./include -I./vendor/raylib/include
LDFLAGS = -L./vendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
SRC_DIR = src
BUILD_DIR = build
TARGET = sanguosha.exe

SOURCES = $(wildcard $(SRC_DIR)/*.c)
MAIN_SOURCES = $(filter-out $(SRC_DIR)/ui_console.c,$(SOURCES))
TEST_SOURCES = $(wildcard tests/test_*.c)
TEST_SRC = $(filter-out $(SRC_DIR)/main.c,$(filter-out $(SRC_DIR)/ui_raylib.c,$(SOURCES)))
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(MAIN_SOURCES))
TEST_TARGETS = $(patsubst tests/%.c,tests/%.exe,$(TEST_SOURCES))

all: $(TARGET)

test: $(TEST_TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

tests/%.exe: tests/%.c
	$(CC) $(CFLAGS) $< $(TEST_SRC) -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) tests/*.exe

.PHONY: all clean test
