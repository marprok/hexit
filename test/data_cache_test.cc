#include "ChunkCache.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

TEST(ChunkCacheTest, LoadChunkTest)
{
    const std::string file_name("../hex_mode.png");
    EXPECT_TRUE(fs::exists(file_name));

    std::fstream in(file_name, std::ios::in | std::ios::binary);
    if (in.is_open())
    {
        if (in)
        {
            in.close();
            ChunkCache cache;
            cache.open_file(file_name);
            EXPECT_EQ(cache.size(), fs::file_size(file_name));
            EXPECT_TRUE(cache.total_chunks() > 0);

            cache.load_chunk(0);
            auto previous_chunk_id = cache.recent_chunk().m_id;
            bool match             = true;
            for (std::uint32_t i = 1; i < cache.total_chunks() && match; ++i)
            {
                cache.load_chunk(i);
                match             = previous_chunk_id == cache.fallback_chunk().m_id;
                previous_chunk_id = cache.recent_chunk().m_id;
            }

            EXPECT_TRUE(match);
        }
    }
}
