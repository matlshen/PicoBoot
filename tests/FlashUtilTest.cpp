#include <gtest/gtest.h>
#include "flash_util.h"

TEST(FlashUtil, ToU32) {
    uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
    EXPECT_EQ(ToU32(data), 0x04030201);
}

TEST(FlashUtil, ToU16) {
    uint8_t data[2] = {0x11, 0x22};
    EXPECT_EQ(ToU16(data), 0x2211);
}

TEST(FlashUtil, IsRangeValid) {
    // Lower bound
    EXPECT_EQ(FlashUtil_IsRangeValid(0x8004000, 0x800), true);

    // Maximum size
    EXPECT_EQ(FlashUtil_IsRangeValid(0x8004000, 0x1C000), true);

    // Upper bound
    EXPECT_EQ(FlashUtil_IsRangeValid(0x801F800, 0x800), true);

    // Below lower bound
    EXPECT_EQ(FlashUtil_IsRangeValid(0x1, 0x1), false);

    // Above upper bound
    EXPECT_EQ(FlashUtil_IsRangeValid(0x8020000, 0x800), false);
    EXPECT_EQ(FlashUtil_IsRangeValid(0x801F800, 0x1000), false);
}

TEST(FlashUtil, IsPageAligned) {
    // Page aligned
    EXPECT_EQ(FlashUtil_IsPageAligned(0x8004000), true);
    EXPECT_EQ(FlashUtil_IsPageAligned(0x1800), true);

    // Not page aligned
    EXPECT_EQ(FlashUtil_IsPageAligned(0x8003FFF), false);
    EXPECT_EQ(FlashUtil_IsPageAligned(0x8000020), false);
}

TEST(FlashUtil, GetRangeAlignment) {
    EXPECT_EQ(FlashUtil_GetRangeAlignment(0x8004000, 0x800), 0x800);
    EXPECT_EQ(FlashUtil_GetRangeAlignment(0x2, 0x1), 0x1);
    EXPECT_EQ(FlashUtil_GetRangeAlignment(0x801F800, 0x800), 0x800);
    EXPECT_EQ(FlashUtil_GetRangeAlignment(0x2, 0x4), 0x2);
}

