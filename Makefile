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

all: dirs $(TARGET)

test: dirs $(TEST_TARGETS)

dirs:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

tests/%.exe: tests/%.c | dirs
	$(CC) $(CFLAGS) $< $(TEST_SRC) -o $@

clean:
	del /F /Q $(TARGET) tests\*.exe 2>nul
	rd /S /Q $(BUILD_DIR) 2>nul

.PHONY: all clean test dirs
