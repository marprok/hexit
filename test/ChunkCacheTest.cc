#include "ChunkCache.h"
#include "IOHandlerMock.h"
#include <gtest/gtest.h>

namespace
{
using namespace Hexit;

inline std::uintmax_t expected_chunks(const IOHandler& handler)
{
    std::uintmax_t chunks = handler.size() / ChunkCache::capacity;
    if (handler.size() % ChunkCache::capacity)
        chunks++;
    return chunks;
}

// General information regarding the IOHandler interface.
TEST(ChunkCacheTest, IOHandlerInformation)
{
    IOHandlerMock handler;
    ChunkCache    cache(handler);

    ASSERT_EQ(cache.recent().m_id, UINT64_MAX);
    ASSERT_EQ(cache.recent().m_count, 0);
    ASSERT_EQ(sizeof(cache.recent().m_data), ChunkCache::capacity);

    ASSERT_EQ(cache.fallback().m_id, UINT64_MAX);
    ASSERT_EQ(cache.fallback().m_count, 0);
    ASSERT_EQ(sizeof(cache.fallback().m_data), ChunkCache::capacity);

    ASSERT_EQ(expected_chunks(handler), cache.total_chunks());
}

// When load_chunk(chunk_id) gets called, the chunk returned by recent()
// should contain the data of chunk_id. The chunk returned by fallback()
// should contain the data that recent() had before the call to
// load_chunk(chunk_id).
TEST(ChunkCacheTest, LoadChunk)
{
    IOHandlerMock handler;
    ChunkCache    cache(handler);
    ASSERT_EQ(cache.total_chunks(), expected_chunks(handler));
    ASSERT_TRUE(cache.load_chunk(0));
    EXPECT_EQ(cache.recent().m_id, 0);
    for (std::uintmax_t i = 1; i < cache.total_chunks(); ++i)
    {
        ASSERT_TRUE(cache.load_chunk(i));
        EXPECT_EQ(cache.recent().m_id, cache.fallback().m_id + 1);
    }
}

// This is the same test as LoadChunk but this time the chunks get loaded
// in reverse order.
TEST(ChunkCacheTest, LoadChunkReverse)
{
    IOHandlerMock handler;
    ChunkCache    cache(handler);
    ASSERT_EQ(cache.total_chunks(), expected_chunks(handler));
    ASSERT_TRUE(cache.load_chunk(cache.total_chunks() - 1));
    EXPECT_EQ(cache.recent().m_id, cache.total_chunks() - 1);
    for (std::uintmax_t i = cache.total_chunks() - 2; i > 0; --i)
    {
        ASSERT_TRUE(cache.load_chunk(i));
        EXPECT_EQ(cache.recent().m_id, cache.fallback().m_id - 1);
    }
}

// When a chunk gets saved, the underlying IOHandler should get updated.
TEST(ChunkCacheTest, SaveChunk)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    ChunkCache    cache(handler);
    ASSERT_EQ(cache.total_chunks(), expected_chunks(handler));
    const auto chunk_id = cache.total_chunks() / 2;
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(cache.load_chunk(chunk_id));
    auto& data_chunk = cache.recent();
    EXPECT_EQ(chunk_id, data_chunk.m_id);
    std::memset(data_chunk.m_data, 0xEF, data_chunk.m_count);
    std::uint8_t* expectation = raw_data + (chunk_id * ChunkCache::capacity);
    EXPECT_NE(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_TRUE(cache.save_chunk(data_chunk));
    EXPECT_EQ(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_EQ(expectation[0], 0xEF);
}

// When a chunk is read only, the underlying IOHandler should not get updated.
TEST(ChunkCacheTest, SaveChunkReadOnly)
{
    IOHandlerMock handler(true);
    std::uint8_t* raw_data = handler.data();
    ChunkCache    cache(handler);
    ASSERT_EQ(cache.total_chunks(), expected_chunks(handler));
    const auto chunk_id = cache.total_chunks() / 2;
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(cache.load_chunk(chunk_id));
    auto& data_chunk = cache.recent();
    EXPECT_EQ(chunk_id, data_chunk.m_id);
    std::memset(data_chunk.m_data, 0xEF, data_chunk.m_count);
    std::uint8_t* expectation = raw_data + (chunk_id * ChunkCache::capacity);
    EXPECT_NE(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_FALSE(cache.save_chunk(data_chunk));
    EXPECT_NE(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_NE(expectation[0], 0xEF);
}
} // namespace
