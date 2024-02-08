#include <gtest/gtest.h>

#include "crc32.h"

TEST(CRC32, HelloWorld) {
    const uint8_t data[] = "Hello, world!";
    const uint32_t expected = 0xEBE6C6E6;
    const uint32_t actual = crc32(data, sizeof(data) - 1, INITIAL_CRC);
    EXPECT_EQ(expected, actual);
}