#include "data_buffer.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

TEST(data_buffer_test, DataReadTest)
{
    const std::string file_name("../hex_mode.png");
    EXPECT_TRUE(fs::exists(file_name));

    std::fstream in(file_name, std::ios::in | std::ios::binary);
    if (in.is_open())
    {
        std::unique_ptr<std::uint8_t[]> expectation(new std::uint8_t[fs::file_size(file_name)]());
        in.read(reinterpret_cast<char*>(expectation.get()), fs::file_size(file_name));

        if (in)
        {
            in.close();
            DataBuffer buffer;
            buffer.open_file(file_name);
            EXPECT_EQ(buffer.size(), fs::file_size(file_name));
            // start from the first byte
            buffer.load_chunk(0);
            bool match = true;
            for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
                match = buffer[i] == expectation[i];

            EXPECT_TRUE(match);
        }
    }
}
