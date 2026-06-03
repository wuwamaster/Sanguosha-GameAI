#pragma once

#include <stdio.h>
#include <stdlib.h>

#define ASSERT_EQ(actual, expected, msg) do { \
    if ((actual) != (expected)) { \
        fprintf(stderr, "ASSERT_EQ failed: %s\n", msg); \
        fprintf(stderr, "  actual: %d\n", (int)(actual)); \
        fprintf(stderr, "  expected: %d\n", (int)(expected)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define ASSERT_NE(actual, expected, msg) do { \
    if ((actual) == (expected)) { \
        fprintf(stderr, "ASSERT_NE failed: %s\n", msg); \
        fprintf(stderr, "  actual: %d\n", (int)(actual)); \
        fprintf(stderr, "  expected: not %d\n", (int)(expected)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define ASSERT_TRUE(expr, msg) do { \
    if (!(expr)) { \
        fprintf(stderr, "ASSERT_TRUE failed: %s\n", msg); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define ASSERT_NOT_NULL(ptr, msg) do { \
    if ((ptr) == NULL) { \
        fprintf(stderr, "ASSERT_NOT_NULL failed: %s\n", msg); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define TEST_START(name) printf("[TEST] %s\n", name)
#define TEST_PASS(name) printf("[PASS] %s\n", name)
