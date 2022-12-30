#include "Scroller.h"
#include <gtest/gtest.h>

// Scroller should automatically calculate the total amount of lines
TEST(ScrollerTest, TotalLines)
{
    constexpr std::uint32_t total_bytes = 4321, bytes_per_line = 5;
    Scroller                scroller(total_bytes, bytes_per_line);
    EXPECT_EQ(scroller.total(), 865);
}

TEST(ScrollerTest, AdjustLines)
{
    constexpr std::uint32_t total_bytes = 3214, bytes_per_line = 5;
    Scroller                scroller(total_bytes, bytes_per_line);
    //    EXPECT_EQ(scroller.total(), 64);
}
