#include "ByteBuffer.h"
#include "IOHandlerMock.h"
#include <array>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

namespace
{
using namespace Hexit;

const std::string file_name("test/path/to/somewhere");

// In case of an io error, is_ok should return true and error_msg() should contain
// an error message.
TEST(ByteBufferTest, RandomAccessError)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    ByteBuffer buffer(handler);
    EXPECT_EQ(buffer.size, handler.size());
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
    const auto               size = handler.size();
    ByteBuffer               buffer(handler);
    constexpr std::uintmax_t first_chunk_id = 2, last_chunkc_id = 3;
    for (std::uintmax_t i = ChunkCache::capacity * first_chunk_id;
         (i < ChunkCache::capacity * (last_chunkc_id + 1)) && (i < size);
         ++i)
    {
        // Access each byte to trigger the caching mechanism.
        buffer[i];
    }
    // Accessing again the bytes from the first chunk, should not cause any loading.
    for (std::uintmax_t i = ChunkCache::capacity * first_chunk_id;
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
    const auto                    size = handler.size();
    ByteBuffer                    buffer(handler);
    std::uint8_t*                 expectation = handler.data().data();
    std::array<std::uintmax_t, 3> byte_ids { 0, size - 1, 1 };
    std::array<std::uint8_t, 3>   original_values { 0xBE, 0xAB, 0xAC };
    std::array<std::uint8_t, 3>   new_values { 0xEF, 0xBA, 0xDC };
    ASSERT_EQ(size, handler.size());
    // set the expectation
    for (std::uintmax_t i = 0; i < byte_ids.size(); ++i)
        expectation[byte_ids[i]] = original_values[i];

    for (std::uintmax_t i = 0; i < byte_ids.size(); ++i)
    {
        auto byte_id         = byte_ids[i];
        expectation[byte_id] = original_values[i];
        // access the byte to load the chunk
        const auto byte_old = buffer[byte_id];
        EXPECT_EQ(byte_old, original_values[i]);
        EXPECT_FALSE(buffer.is_dirty(byte_id));
        EXPECT_FALSE(buffer.has_dirty());
        buffer.set_byte(byte_id, new_values[i]);
        EXPECT_TRUE(buffer.is_dirty(byte_id));
        EXPECT_TRUE(buffer.has_dirty());
        const auto byte_new = buffer[byte_id];
        EXPECT_EQ(byte_new, new_values[i]);
        EXPECT_NE(expectation[byte_id], new_values[i]);
        EXPECT_EQ(expectation[byte_id], byte_old);
        buffer.save();
        EXPECT_EQ(expectation[byte_id], new_values[i]);
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
    std::uint8_t* expectation = handler.data().data();
    ByteBuffer    buffer(handler);
    for (std::uintmax_t i = 0; i < size; ++i)
        ASSERT_EQ(buffer[i], expectation[i]);
}

// This is the same test as DataRead but this time the bytes get accessed
// in reverse order.
TEST(ByteBufferTest, ByteReadReverse)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto    size        = handler.size();
    std::uint8_t* expectation = handler.data().data();
    ByteBuffer    buffer(handler);
    for (std::uintmax_t i = size - 1; i > 0; --i)
        ASSERT_EQ(buffer[i], expectation[i]);
}

// Setting bytes using ByteBuffer will not update the actual data
// until save() gets called.
TEST(ByteBufferTest, SaveBytes)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    const auto    size     = handler.size();
    std::uint8_t* raw_data = handler.data().data();
    ByteBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, size);
    std::array<std::uintmax_t, 9> dirty_ids { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        std::uintmax_t from = id * ChunkCache::capacity;
        std::uintmax_t to   = from + ChunkCache::capacity;
        if (to >= size)
            to = from + size % ChunkCache::capacity;

        const auto old_value = raw_data[from];
        for (; from < to; ++from)
        {
            buffer.set_byte(from, old_value + 1);
            EXPECT_EQ(buffer[from], old_value + 1);
            EXPECT_NE(raw_data[from], old_value + 1);
        }
    }

    EXPECT_EQ(handler.load_count(), dirty_ids.size());
    buffer.save();
    EXPECT_EQ(handler.load_count(), 2 * dirty_ids.size());

    for (auto id : dirty_ids)
    {
        std::uintmax_t from = id * ChunkCache::capacity;
        std::uintmax_t to   = from + ChunkCache::capacity;
        if (to >= size)
            to = from + size % ChunkCache::capacity;

        const std::uint8_t old_value = raw_data[from] - 1;
        // save all the dirty bytes
        for (from = id * ChunkCache::capacity; from < to; ++from)
        {
            EXPECT_EQ(raw_data[from], buffer[from]);
            EXPECT_EQ(raw_data[from], old_value + 1);
        }
    }
    EXPECT_EQ(handler.load_count(), 3 * dirty_ids.size());
}

// Same as SaveBytes but this time the read only flag is set to true
// and as a result it should not be possible to modify the actual data.
TEST(ByteBufferTest, SaveAllBytesReadOnly)
{
    IOHandlerMock handler(true);
    ASSERT_TRUE(handler.open(file_name));
    const auto    size     = handler.size();
    std::uint8_t* raw_data = handler.data().data();
    ByteBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, size);
    std::array<std::uintmax_t, 9> dirty_ids { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        std::uintmax_t from = id * ChunkCache::capacity;
        std::uintmax_t to   = from + ChunkCache::capacity;
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

    ASSERT_EQ(size, handler.size());
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

TEST(ByteBufferTest, FindCornerCases)
{
    IOHandlerMock handler;
    ASSERT_TRUE(handler.open(file_name));
    ByteBuffer buffer(handler);

    // empty needle
    for (std::size_t i = 0; i < buffer.size; ++i)
        ASSERT_FALSE(buffer.find({}, i).has_value());

    // full match
    for (std::size_t i = 0; i < buffer.size; ++i)
        ASSERT_EQ(buffer.find(handler.data(), i).has_value(), !i);

    // out of range
    ASSERT_FALSE(buffer.find(handler.data(), buffer.size + 1).has_value());
}

TEST(ByteBufferTest, FindMatches)
{
    std::vector<std::uint8_t> needle;
    needle.resize(100);
    for (auto& byte : needle)
        byte = static_cast<std::uint8_t>(rand() % 256);
    // match at start
    {
        IOHandlerMock handler;
        ASSERT_TRUE(handler.open(file_name));
        ByteBuffer buffer(handler);
        for (std::size_t i = 0; i < needle.size(); ++i)
            handler.data()[i] = needle[i];

        auto res = buffer.find(needle);
        ASSERT_TRUE(res.has_value());
        ASSERT_EQ(res.value(), 0);
    }
    // match at midle
    {
        IOHandlerMock handler;
        ASSERT_TRUE(handler.open(file_name));
        ByteBuffer buffer(handler);
        for (std::size_t i = 0; i < needle.size(); ++i)
            handler.data()[buffer.size / 2 + i] = needle[i];

        auto res = buffer.find(needle);
        ASSERT_TRUE(res.has_value());
        ASSERT_EQ(res.value(), buffer.size / 2);
    }
    // match at end
    {
        IOHandlerMock handler;
        ASSERT_TRUE(handler.open(file_name));
        ByteBuffer buffer(handler);
        for (std::size_t i = 0; i < needle.size(); ++i)
            handler.data()[buffer.size - needle.size() - 1 + i] = needle[i];

        auto res = buffer.find(needle);
        ASSERT_TRUE(res.has_value());
        ASSERT_EQ(res.value(), buffer.size - needle.size() - 1);
    }
    // multiple matches
    {
        IOHandlerMock handler;
        ASSERT_TRUE(handler.open(file_name));
        ByteBuffer buffer(handler);
        for (std::size_t i = 0; i < needle.size(); ++i)
        {
            handler.data()[i]                                   = needle[i];
            handler.data()[buffer.size - needle.size() - 1 + i] = needle[i];
        }
        auto res = buffer.find(needle);
        ASSERT_TRUE(res.has_value());
        ASSERT_EQ(res.value(), 0);
        res = buffer.find(needle, needle.size());
        ASSERT_TRUE(res.has_value());
        ASSERT_EQ(res.value(), buffer.size - needle.size() - 1);
    }
    // no match
    {
        IOHandlerMock handler;
        ASSERT_TRUE(handler.open(file_name));
        ByteBuffer buffer(handler);
        ASSERT_FALSE(buffer.find(needle).has_value());
    }
}
} // namespace
