#include "ByteBuffer.h"
#include "IOHandlerMock.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

namespace
{
const std::string       file_name("test/path/to/somewhere");
constexpr std::uint32_t expected_size_bytes = IOHandlerMock::chunk_count * ChunkCache::capacity;
inline std::uint32_t    expected_chunks()
{
    std::uint32_t chunks = expected_size_bytes / ChunkCache::capacity;
    if (expected_size_bytes % ChunkCache::capacity)
        chunks++;
    return chunks;
}
}

// Accessing bytes from a chunk that already is in memory
// should not cause any more loading to happen in the underlying IOHandler.
TEST(ByteBufferTest, ChunkCaching)
{
    IOHandlerMock handler;
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    ASSERT_TRUE(cache.open(file_name));
    ASSERT_EQ(cache.total_chunks(), expected_chunks());
    constexpr std::uint32_t first_chunk_id = 2, last_chunkc_id = 3;
    for (std::uint32_t i = ChunkCache::capacity * first_chunk_id;
         (i < ChunkCache::capacity * (last_chunkc_id + 1)) && (i < buffer.size());
         ++i)
    {
        // Access each byte to trigger the caching mechanism.
        buffer[i];
    }
    // Accessing again the bytes from the first chunk, should not cause any loading.
    for (std::uint32_t i = ChunkCache::capacity * first_chunk_id;
         (i < ChunkCache::capacity * last_chunkc_id) && (i < buffer.size());
         ++i)
    {
        buffer[i];
    }
    EXPECT_EQ(handler.load_count(), 2);
    EXPECT_EQ(cache.recent().m_id, last_chunkc_id);
    EXPECT_EQ(cache.fallback().m_id, first_chunk_id);
}

// When a byte gets modified, the buffer should keep the new value
// internally as well as mark the buffer as "dirty". The actual contents
// of the underlying IOHandler will not get updated until save gets called.
TEST(ByteBufferTest, DataModification)
{
    IOHandlerMock handler;
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    const auto    byte_id     = buffer.size() - 1;
    std::uint8_t* expectation = handler.data();
    expectation[byte_id]      = 0x0F;
    ASSERT_TRUE(cache.open(file_name));
    ASSERT_EQ(buffer.size(), expected_size_bytes);
    // access the byte to load the chunk
    auto byte_old = buffer[byte_id];
    EXPECT_EQ(byte_old, 0x0F);
    EXPECT_FALSE(buffer.is_dirty(byte_id));
    EXPECT_FALSE(buffer.has_dirty());
    buffer.set_byte(byte_id, 0xFF);
    EXPECT_TRUE(buffer.is_dirty(byte_id));
    EXPECT_TRUE(buffer.has_dirty());
    EXPECT_EQ(buffer[byte_id], 0xFF);
    EXPECT_NE(expectation[byte_id], 0xFF);
    EXPECT_EQ(expectation[byte_id], byte_old);
}

// ByteBuffer should have access to all the bytes that the underlying
// IOHandler does.
TEST(ByteBufferTest, ByteRead)
{
    IOHandlerMock handler;
    std::uint8_t* expectation = handler.data();
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    ASSERT_TRUE(cache.open(file_name));
    // start from the first chunk and the first byte
    ASSERT_TRUE(cache.load_chunk(0));
    ASSERT_EQ(cache.recent().m_id, 0);
    bool match = true;
    for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

// This is the same test as DataRead but this time the bytes get accessed
// in reverse order.
TEST(ByteBufferTest, ByteReadReverse)
{
    IOHandlerMock handler;
    std::uint8_t* expectation = handler.data();
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    ASSERT_TRUE(cache.open(file_name));
    // start from the last chunk and the last byte
    ASSERT_TRUE(cache.load_chunk(cache.total_chunks() - 1));
    EXPECT_EQ(cache.recent().m_id, cache.total_chunks() - 1);
    bool match = true;
    for (std::uint32_t i = buffer.size() - 1; i > 0 && match; --i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

// Setting bytes using ByteBuffer will not update the actual data
// until save() gets called.
TEST(ByteBufferTest, SaveBytes)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(cache.open(file_name));
    std::uint32_t dirty_ids[] = { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(cache.load_chunk(id));
        auto data_chunk = cache.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            buffer.set_byte(id * ChunkCache::capacity + i, id + 1);
            EXPECT_EQ(buffer[id * ChunkCache::capacity + i], id + 1);
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            // The recent chunk should not change
            ASSERT_EQ(id, cache.recent().m_id);
        }
    }
    // save all the dirty bytes
    buffer.save();
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(cache.load_chunk(id));
        auto data_chunk = cache.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            EXPECT_EQ(expectation[i], buffer[id * ChunkCache::capacity + i]);
            EXPECT_EQ(expectation[i], id + 1);
            // The recent chunk should not change
            ASSERT_EQ(id, cache.recent().m_id);
        }
    }
}

// Same as SaveBytes but this time the read only flag is set to true
// and as a result it should not be possible to modify the actual data.
TEST(ByteBufferTest, SaveAllBytesReadOnly)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    ChunkCache    cache(handler);
    ByteBuffer    buffer(cache);
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(cache.open(file_name, true));
    std::uint32_t dirty_ids[] = { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(cache.load_chunk(id));
        auto data_chunk = cache.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            buffer.set_byte(id * ChunkCache::capacity + i, id + 1);
            EXPECT_EQ(buffer[id * ChunkCache::capacity + i], id + 1);
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            // The recent chunk should not change
            ASSERT_EQ(id, cache.recent().m_id);
        }
    }
    // save all the dirty bytes
    buffer.save();
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(cache.load_chunk(id));
        auto data_chunk = cache.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            EXPECT_NE(expectation[i], id + 1);
            // The recent chunk should not change
            ASSERT_EQ(id, cache.recent().m_id);
        }
    }
}
