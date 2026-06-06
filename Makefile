CC = gcc
CFLAGS = -Wall -g -I./include -I./vendor/raylib/include   # 添加 raylib 头文件路径
LDFLAGS = -L./vendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm  # raylib 链接参数
SRC_DIR = src
BUILD_DIR = build
TARGET = sanguosha.exe

SOURCES = $(wildcard $(SRC_DIR)/*.c)
MAIN_SOURCES = $(filter-out $(SRC_DIR)/ui_console.c,$(SOURCES))  # 主程序用 ui_raylib
TEST_SOURCES = $(wildcard tests/test_*.c)
TEST_SRC = $(filter-out $(SRC_DIR)/main.c,$(filter-out $(SRC_DIR)/ui_raylib.c,$(SOURCES)))  # 测试用 ui_console
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(MAIN_SOURCES))
TEST_TARGETS = $(patsubst tests/%.c,tests/%.exe,$(TEST_SOURCES))

all: $(TARGET)

test: $(TEST_TARGETS)

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

tests/%.exe: tests/%.c
	$(CC) $(CFLAGS) $< $(TEST_SRC) -o $@

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(TARGET) del $(TARGET)
	if exist tests\*.exe del tests\*.exe

.PHONY: all clean test
