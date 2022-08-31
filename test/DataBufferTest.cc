#include "DataBuffer.h"
#include "IOHandler.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

namespace
{
class IOHandlerMock : public IOHandler
{
public:
    static constexpr std::uint32_t chunk_count = 255;
    IOHandlerMock()
        : m_id(0)
    {
    }

    ~IOHandlerMock() = default;

    bool open(const fs::path& path) override
    {
        m_name = path;
        return true;
    }

    void close() override
    {
    }

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override
    {
        if (!o_buffer
            || buffer_size == 0
            || buffer_size > ChunkCache::capacity
            || m_id >= chunk_count)
            return false;

        std::memcpy(o_buffer, m_data[m_id], buffer_size);
        m_id++;
        return true;
    }

    void write(const std::uint8_t* i_buffer, std::size_t buffer_size) override
    {
        if (!i_buffer
            || buffer_size == 0
            || buffer_size > ChunkCache::capacity
            || m_id >= chunk_count)
            return;

        std::memcpy(m_data[m_id], i_buffer, buffer_size);
    }

    void seek(std::uint32_t offset) override
    {
        m_id = offset / ChunkCache::capacity;
    }

    const fs::path& name() const
    {
        return m_name;
    }

    std::uint32_t size() const
    {
        return chunk_count * ChunkCache::capacity;
    }

    std::uint8_t* data() { return reinterpret_cast<std::uint8_t*>(m_data); }

    void randomize()
    {
        std::srand(std::time(nullptr));
        std::uint8_t* bytes = data();
        for (std::size_t i = 0; i < chunk_count * ChunkCache::capacity; ++i)
            bytes[i] = rand() % 256;
    }

private:
    fs::path m_name;
    // Let the data uninitialized to simulate "real" data
    std::uint8_t  m_data[chunk_count][ChunkCache::capacity];
    std::uint32_t m_id;
};

const std::string file_name("test/path/to/somewhere");
}

TEST(DataBufferTest, IOData)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    EXPECT_EQ(buffer.name(), handler.name());
    EXPECT_EQ(buffer.size(), handler.size());
    std::uint32_t expected_chunks = handler.size() / ChunkCache::capacity;
    if (handler.size() % ChunkCache::capacity)
        expected_chunks++;
    EXPECT_EQ(expected_chunks, buffer.total_chunks());
}

TEST(DataBufferTest, LoadChunk)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 0);

    EXPECT_TRUE(buffer.load_chunk(0));
    EXPECT_EQ(buffer.recent_chunk().m_id, 0);
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = 1; i < buffer.total_chunks(); ++i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_EQ(buffer.recent_chunk().m_id, buffer.fallback_chunk().m_id + 1);
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, LoadChunkReverse)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 2);
    EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    EXPECT_EQ(buffer.recent_chunk().m_id, buffer.total_chunks() - 1);
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = buffer.total_chunks() - 2; i > 0; --i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_EQ(buffer.recent_chunk().m_id, buffer.fallback_chunk().m_id - 1);
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, DataModification)
{
    IOHandlerMock handler;
    std::uint8_t* expectation = handler.data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    const auto byte_id   = buffer.size() - 1;
    expectation[byte_id] = 0x0F;
    EXPECT_EQ(buffer[byte_id], 0x0F);
    EXPECT_FALSE(buffer.is_dirty(byte_id));
    EXPECT_FALSE(buffer.has_dirty());
    buffer.set_byte(byte_id, 0xFF);
    EXPECT_TRUE(buffer.is_dirty(byte_id));
    EXPECT_TRUE(buffer.has_dirty());
    EXPECT_EQ(buffer[byte_id], 0xFF);
}

TEST(DataBufferTest, DataRead)
{
    IOHandlerMock handler;
    handler.randomize();
    std::uint8_t* expectation = handler.data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    // start from the first chunk and the first byte
    EXPECT_TRUE(buffer.load_chunk(0));
    EXPECT_EQ(buffer.recent_chunk().m_id, 0);
    bool match = true;
    for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

TEST(DataBufferTest, DataReadReverse)
{
    IOHandlerMock handler;
    handler.randomize();
    std::uint8_t* expectation = handler.data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    // start from the last chunk and the last byte
    EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    EXPECT_EQ(buffer.recent_chunk().m_id, buffer.total_chunks() - 1);
    bool match = true;
    for (std::uint32_t i = buffer.size() - 1; i > 0 && match; --i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

TEST(DataBufferTest, SaveChunk)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    DataBuffer buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    auto chunk_id = buffer.total_chunks() / 2;
    EXPECT_TRUE(buffer.load_chunk(chunk_id));
    EXPECT_EQ(buffer.recent_chunk().m_id, chunk_id);
    auto& data_chunk = buffer.recent_chunk();
    EXPECT_EQ(chunk_id, data_chunk.m_id);
    std::memset(data_chunk.m_data, 0xEF, data_chunk.m_count);
    auto expectation = raw_data + (chunk_id * ChunkCache::capacity);
    EXPECT_NE(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    buffer.save_chunk(data_chunk);
    EXPECT_EQ(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_EQ(expectation[0], 0xEF);
}

TEST(DataBufferTest, SaveAllChunks)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    DataBuffer buffer(handler);
    EXPECT_TRUE(buffer.open(file_name));
    std::uint32_t dirty_ids[] = { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        EXPECT_TRUE(buffer.load_chunk(id));
        auto data_chunk = buffer.recent_chunk();
        EXPECT_EQ(id, data_chunk.m_id);
        auto expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            buffer.set_byte(id * ChunkCache::capacity + i, id + 1);
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
        }
    }
    // save all the dirty bytes
    buffer.save();
    for (auto id : dirty_ids)
    {
        EXPECT_TRUE(buffer.load_chunk(id));
        auto data_chunk = buffer.recent_chunk();
        EXPECT_EQ(id, data_chunk.m_id);
        auto expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            EXPECT_EQ(expectation[i], buffer[id * ChunkCache::capacity + i]);
            EXPECT_EQ(expectation[i], id + 1);
        }
    }
}
