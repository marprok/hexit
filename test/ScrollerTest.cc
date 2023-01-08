#include "Scroller.h"
#include <gtest/gtest.h>

TEST(ScrollerTest, InvalidParameters)
{
    Scroller scroller(0u, 0u);
    EXPECT_EQ(scroller.first(), 0u);
    EXPECT_EQ(scroller.last(), 0u);
    EXPECT_EQ(scroller.total(), 0u);
    EXPECT_EQ(scroller.visible(), 0u);
    EXPECT_EQ(scroller.active(), 0u);
}

// The scroller should automatically scroll up/down when needed.
TEST(ScrollerTest, AutoScrolling)
{
    constexpr std::uint32_t TOTAL_BYTES = 4321u, BYTES_PER_LINE = 5u, EXPECTED_LINES = 865u;
    Scroller                scroller(TOTAL_BYTES, BYTES_PER_LINE);
    ASSERT_EQ(scroller.total(), EXPECTED_LINES);

    std::uint32_t current_byte = 0u, visible_lines = 31u;
    {
        scroller.adjust_lines(visible_lines, current_byte / BYTES_PER_LINE);
        EXPECT_EQ(scroller.first(), 0u);
        EXPECT_EQ(scroller.last(), visible_lines - 1);
        EXPECT_EQ(scroller.active(), 0u);
        EXPECT_EQ(scroller.visible(), visible_lines);

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
        {
            if (i < (visible_lines - 1))
                EXPECT_FALSE(scroller.move_down());
            else if (i < (EXPECTED_LINES - 1))
                EXPECT_TRUE(scroller.move_down());
            else
                EXPECT_FALSE(scroller.move_down());
        }
    }

    current_byte = TOTAL_BYTES - 1;
    {
        scroller.adjust_lines(visible_lines, (TOTAL_BYTES - 1) / BYTES_PER_LINE);
        EXPECT_EQ(scroller.first(), EXPECTED_LINES - visible_lines);
        EXPECT_EQ(scroller.last(), EXPECTED_LINES - 1);
        EXPECT_EQ(scroller.active(), visible_lines - 1);
        EXPECT_EQ(scroller.visible(), visible_lines);

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
        {
            if (i < (visible_lines - 1))
                EXPECT_FALSE(scroller.move_up());
            else if (i < (EXPECTED_LINES - 1))
                EXPECT_TRUE(scroller.move_up());
            else
                EXPECT_FALSE(scroller.move_up());
        }
    }

    // When the visible lines are more than the total number of lines
    // no scrolling should be necessary.
    current_byte  = 0;
    visible_lines = EXPECTED_LINES;
    {
        scroller.adjust_lines(visible_lines, current_byte / BYTES_PER_LINE);
        EXPECT_EQ(scroller.first(), 0u);
        EXPECT_EQ(scroller.last(), visible_lines - 1);
        EXPECT_EQ(scroller.active(), 0u);
        EXPECT_EQ(scroller.visible(), EXPECTED_LINES);

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
            EXPECT_FALSE(scroller.move_down());

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
            EXPECT_FALSE(scroller.move_up());
    }

    current_byte  = 0u;
    visible_lines = 2 * EXPECTED_LINES;
    {
        scroller.adjust_lines(visible_lines, current_byte / BYTES_PER_LINE);
        EXPECT_EQ(scroller.first(), 0u);
        // The last line should never be greater than the total number of lines.
        EXPECT_EQ(scroller.last(), EXPECTED_LINES - 1);
        EXPECT_EQ(scroller.active(), 0u);
        EXPECT_EQ(scroller.visible(), EXPECTED_LINES);

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
            EXPECT_FALSE(scroller.move_down());

        for (std::uint32_t i = 0u; i < (2 * EXPECTED_LINES); ++i)
            EXPECT_FALSE(scroller.move_up());
    }
}
