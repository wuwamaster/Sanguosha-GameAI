CC = gcc
CFLAGS = -Wall -g -I./include   # 这里改成 include 目录
SRC_DIR = src
BUILD_DIR = build
TARGET = sanguosha.exe

SOURCES = $(wildcard $(SRC_DIR)/*.c)
TEST_SOURCES = $(wildcard tests/test_*.c)
TEST_SRC = $(filter-out $(SRC_DIR)/main.c,$(SOURCES))
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TEST_TARGETS = $(patsubst tests/%.c,tests/%.exe,$(TEST_SOURCES))

all: $(TARGET)

test: $(TEST_TARGETS)

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

tests/%.exe: tests/%.c
	$(CC) $(CFLAGS) $< $(TEST_SRC) -o $@

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(TARGET) del $(TARGET)
	if exist tests\*.exe del tests\*.exe

.PHONY: all clean test
