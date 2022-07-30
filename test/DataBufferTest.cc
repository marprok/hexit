#include "DataBuffer.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

namespace
{
const std::string file_name("../hex_mode.png");
}

TEST(DataBufferTest, FileData)
{
    EXPECT_TRUE(fs::exists(file_name));
    DataBuffer buffer;
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_EQ(buffer.name(), fs::canonical(file_name));
    EXPECT_EQ(buffer.size(), fs::file_size(file_name));
    std::uint32_t expected_chunks = fs::file_size(file_name) / DataBuffer::ChunkCache::capacity;
    if (fs::file_size(file_name) / DataBuffer::ChunkCache::capacity)
        expected_chunks++;
    EXPECT_EQ(expected_chunks, buffer.total_chunks());
}

TEST(DataBufferTest, LoadChunk)
{
    EXPECT_TRUE(fs::exists(file_name));
    DataBuffer buffer;
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_EQ(buffer.size(), fs::file_size(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 0);

    EXPECT_TRUE(buffer.load_chunk(0));
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = 1; i < buffer.total_chunks(); ++i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, LoadChunkReverse)
{
    EXPECT_TRUE(fs::exists(file_name));
    DataBuffer buffer;
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_EQ(buffer.size(), fs::file_size(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 2);

    EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = buffer.total_chunks() - 2; i > 0; --i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, DataModification)
{
    EXPECT_TRUE(fs::exists(file_name));
    DataBuffer buffer;
    EXPECT_TRUE(buffer.open_file(file_name));
    auto byte_id = buffer.size() - 1;
    EXPECT_NE(buffer[byte_id], 0xFF);
    EXPECT_FALSE(buffer.is_dirty(byte_id));
    EXPECT_FALSE(buffer.has_dirty());
    buffer.set_byte(byte_id, 0xFF);
    EXPECT_TRUE(buffer.is_dirty(byte_id));
    EXPECT_TRUE(buffer.has_dirty());
    EXPECT_EQ(buffer[byte_id], 0xFF);
}

TEST(DataBufferTest, DataRead)
{
    EXPECT_TRUE(fs::exists(file_name));
    std::fstream in(file_name, std::ios::in | std::ios::binary);
    if (in.is_open())
    {
        auto expectation = std::make_unique<std::uint8_t[]>(fs::file_size(file_name));
        in.read(reinterpret_cast<char*>(expectation.get()), fs::file_size(file_name));
        if (in)
        {
            in.close();
            DataBuffer buffer;
            EXPECT_TRUE(buffer.open_file(file_name));
            EXPECT_EQ(buffer.size(), fs::file_size(file_name));
            // start from the first byte
            EXPECT_TRUE(buffer.load_chunk(0));
            bool match = true;
            for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
                match = buffer[i] == expectation[i];

            EXPECT_TRUE(match);
        }
    }
}

TEST(DataBufferTest, DataReadReverse)
{
    EXPECT_TRUE(fs::exists(file_name));
    std::fstream in(file_name, std::ios::in | std::ios::binary);
    if (in.is_open())
    {
        auto expectation = std::make_unique<std::uint8_t[]>(fs::file_size(file_name));
        in.read(reinterpret_cast<char*>(expectation.get()), fs::file_size(file_name));
        if (in)
        {
            in.close();
            DataBuffer buffer;
            EXPECT_TRUE(buffer.open_file(file_name));
            EXPECT_EQ(buffer.size(), fs::file_size(file_name));
            // start from the first byte
            EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
            bool match = true;
            for (std::uint32_t i = buffer.size() - 1; i > 0 && match; --i)
                match = buffer[i] == expectation[i];

            EXPECT_TRUE(match);
        }
    }
}
