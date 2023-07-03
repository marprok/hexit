#include "ByteBuffer.h"
#include "IOHandlerMock.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace
{
namespace fs = std::filesystem;
using namespace Hexit;

const std::string       file_name("test/path/to/somewhere");
constexpr std::uint64_t expected_size_bytes = IOHandlerMock::chunk_count * ChunkCache::capacity;

// In case of an io error, is_ok should return true and error_msg() should contain
// an error message.
TEST(ByteBufferTest, RandomAccessError)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    ByteBuffer buffer(handler);
    EXPECT_EQ(buffer.size, expected_size_bytes);
    handler.mock_io_fail(true);
    EXPECT_TRUE(buffer.error_msg().empty());
    EXPECT_TRUE(buffer.is_ok());
    buffer[0];
    EXPECT_FALSE(buffer.is_ok());
    EXPECT_FALSE(buffer.error_msg().empty());
}

// Accessing bytes from a chunk that already is in memory
// should not cause any more loading to happen in the underlying IOHandler.
TEST(ByteBufferTest, DataCaching)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto              size = handler.size();
    ByteBuffer              buffer(handler);
    constexpr std::uint64_t first_chunk_id = 2, last_chunkc_id = 3;
    for (std::uint64_t i = ChunkCache::capacity * first_chunk_id;
         (i < ChunkCache::capacity * (last_chunkc_id + 1)) && (i < size);
         ++i)
    {
        // Access each byte to trigger the caching mechanism.
        buffer[i];
    }
    // Accessing again the bytes from the first chunk, should not cause any loading.
    for (std::uint64_t i = ChunkCache::capacity * first_chunk_id;
         (i < ChunkCache::capacity * last_chunkc_id) && (i < size);
         ++i)
    {
        buffer[i];
    }
    EXPECT_EQ(handler.load_count(), 2);
}

// When a byte gets modified, the buffer should keep the new value
// internally as well as mark the buffer as "dirty". The actual contents
// of the underlying IOHandler will not get updated until save gets called.
TEST(ByteBufferTest, DataModification)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto                   size = handler.size();
    ByteBuffer                   buffer(handler);
    std::uint8_t*                expectation = handler.data();
    std::array<std::uint64_t, 3> byte_ids { 0, size - 1, 1 };
    std::array<std::uint8_t, 3>  original_values { 0xBE, 0xAB, 0xAC };
    std::array<std::uint8_t, 3>  new_values { 0xEF, 0xBA, 0xDC };
    for (auto byte_id : byte_ids)
        expectation[byte_id] = original_values[byte_id];
    ASSERT_EQ(size, expected_size_bytes);

    for (auto byte_id : byte_ids)
    {
        // access the byte to load the chunk
        const auto byte_old = buffer[byte_id];
        EXPECT_EQ(byte_old, original_values[byte_id]);
        EXPECT_FALSE(buffer.is_dirty(byte_id));
        EXPECT_FALSE(buffer.has_dirty());
        buffer.set_byte(byte_id, new_values[byte_id]);
        EXPECT_TRUE(buffer.is_dirty(byte_id));
        EXPECT_TRUE(buffer.has_dirty());
        const auto byte_new = buffer[byte_id];
        EXPECT_EQ(byte_new, new_values[byte_id]);
        EXPECT_NE(expectation[byte_id], new_values[byte_id]);
        EXPECT_EQ(expectation[byte_id], byte_old);
        buffer.save();
        EXPECT_EQ(expectation[byte_id], new_values[byte_id]);
        EXPECT_NE(expectation[byte_id], byte_old);
    }
    // Only two I/O reads should be performed.
    // - The first one should be due to cold start when we read the first byte.
    // - The second one when we try to access the last byte.
    EXPECT_EQ(handler.load_count(), 2);
}

// ByteBuffer should have access to all the bytes that the underlying
// IOHandler does.
TEST(ByteBufferTest, ByteRead)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto    size        = handler.size();
    std::uint8_t* expectation = handler.data();
    ByteBuffer    buffer(handler);
    for (std::uint64_t i = 0; i < size; ++i)
        ASSERT_EQ(buffer[i], expectation[i]);
}

// This is the same test as DataRead but this time the bytes get accessed
// in reverse order.
TEST(ByteBufferTest, ByteReadReverse)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto    size        = handler.size();
    std::uint8_t* expectation = handler.data();
    ByteBuffer    buffer(handler);
    for (std::uint64_t i = size - 1; i > 0; --i)
        ASSERT_EQ(buffer[i], expectation[i]);
}

// Setting bytes using ByteBuffer will not update the actual data
// until save() gets called.
TEST(ByteBufferTest, SaveBytes)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto    size     = handler.size();
    std::uint8_t* raw_data = handler.data();
    ByteBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, size);
    std::array<std::uint64_t, 9> dirty_ids { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        std::uint64_t from = id * ChunkCache::capacity;
        std::uint64_t to   = from + ChunkCache::capacity;
        if (to >= size)
            to = from + size % ChunkCache::capacity;

        const auto old_value = buffer[from];
        for (; from < to; ++from)
        {
            buffer.set_byte(from, old_value + 1);
            EXPECT_EQ(buffer[from], old_value + 1);
            EXPECT_NE(raw_data[from], old_value + 1);
        }
        // save all the dirty bytes
        buffer.save();
        for (from = id * ChunkCache::capacity; from < to; ++from)
        {
            EXPECT_EQ(raw_data[from], buffer[from]);
            EXPECT_EQ(raw_data[from], old_value + 1);
        }
    }
    EXPECT_EQ(handler.load_count(), dirty_ids.size());
}

// Same as SaveBytes but this time the read only flag is set to true
// and as a result it should not be possible to modify the actual data.
TEST(ByteBufferTest, SaveAllBytesReadOnly)
{
    IOHandlerMock handler(true);
    ASSERT_TRUE(handler.open(file_name));
    const auto    size     = handler.size();
    std::uint8_t* raw_data = handler.data();
    ByteBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, size);
    std::array<std::uint64_t, 9> dirty_ids { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        std::uint64_t from = id * ChunkCache::capacity;
        std::uint64_t to   = from + ChunkCache::capacity;
        if (to >= size)
            to = from + size % ChunkCache::capacity;

        const auto old_value = buffer[from];
        for (; from < to; ++from)
        {
            buffer.set_byte(from, old_value + 1);
            EXPECT_EQ(buffer[from], old_value + 1);
            EXPECT_NE(raw_data[from], old_value + 1);
        }
        // save all the dirty bytes
        buffer.save();
        for (from = id * ChunkCache::capacity; from < to; ++from)
        {
            EXPECT_NE(raw_data[from], buffer[from]);
            EXPECT_NE(raw_data[from], old_value + 1);
        }
    }
    EXPECT_EQ(handler.load_count(), dirty_ids.size());
}

TEST(ByteBufferTest, ErrorDuringSave)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto size = handler.size();
    ByteBuffer buffer(handler);

    ASSERT_EQ(size, expected_size_bytes);
    EXPECT_TRUE(buffer.error_msg().empty());
    EXPECT_TRUE(buffer.is_ok());
    EXPECT_FALSE(buffer.is_dirty(0));
    EXPECT_FALSE(buffer.has_dirty());
    buffer.set_byte(0, 0xBE);
    EXPECT_TRUE(buffer.is_dirty(0));
    EXPECT_TRUE(buffer.has_dirty());
    handler.mock_io_fail(true);
    buffer.save();
    EXPECT_FALSE(buffer.error_msg().empty());
    EXPECT_FALSE(buffer.is_ok());
}
} // namespace
