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
        : m_id(0u)
        , m_load_count(0u)
    {
        randomize();
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
        m_load_count++;
        m_id++;
        return true;
    }

    bool write(const std::uint8_t* i_buffer, std::size_t buffer_size) override
    {
        if (!i_buffer
            || buffer_size == 0
            || buffer_size > ChunkCache::capacity
            || m_id >= chunk_count)
            return false;

        std::memcpy(m_data[m_id], i_buffer, buffer_size);
        return true;
    }

    bool seek(std::uint32_t offset) override
    {
        m_id = offset / ChunkCache::capacity;
        return true;
    }

    const fs::path& name() const override
    {
        return m_name;
    }

    std::uint32_t size() const override
    {
        return chunk_count * ChunkCache::capacity;
    }

    std::uint8_t* data() { return reinterpret_cast<std::uint8_t*>(m_data); }

    std::uint32_t load_count() const { return m_load_count; }

private:
    inline void randomize()
    {
        std::srand(std::time(nullptr));
        std::uint8_t* bytes = data();
        for (std::size_t i = 0; i < chunk_count * ChunkCache::capacity; ++i)
            bytes[i] = rand() % 256;
    }

    fs::path      m_name;
    std::uint8_t  m_data[chunk_count][ChunkCache::capacity];
    std::uint32_t m_id;
    std::uint32_t m_load_count;
};

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

// General information regarding the IOHandler interface.
TEST(DataBufferTest, IOHandlerInformation)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    ASSERT_EQ(buffer.name(), file_name);
    ASSERT_EQ(buffer.size(), expected_size_bytes);
    ASSERT_EQ(expected_chunks(), buffer.total_chunks());
}

// Accessing bytes from a chunk that already is in memory
// should not cause any more loading to happen in the underlying IOHandler.
TEST(DataBufferTest, ChunkCaching)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    ASSERT_EQ(buffer.total_chunks(), expected_chunks());
    constexpr std::uint32_t first_chunk_id = 2, last_chunkc_id = 3;
    for (std::uint32_t i = DataBuffer::capacity * first_chunk_id;
         (i < DataBuffer::capacity * (last_chunkc_id + 1)) && (i < buffer.size());
         ++i)
    {
        // Access each byte to trigger the caching mechanism.
        buffer[i];
    }
    // Accessing again the bytes from the first chunk, should not cause any loading.
    for (std::uint32_t i = DataBuffer::capacity * first_chunk_id;
         (i < DataBuffer::capacity * last_chunkc_id) && (i < buffer.size());
         ++i)
    {
        buffer[i];
    }
    EXPECT_EQ(handler.load_count(), 2);
    EXPECT_EQ(buffer.recent().m_id, last_chunkc_id);
    EXPECT_EQ(buffer.fallback().m_id, first_chunk_id);
}

// When load_chunk(chunk_id) gets called, the chunk returned by recent()
// should contain the data of chunk_id. The chunk returned by fallback()
// should contain the data that recent() had before the call to
// load_chunk(chunk_id).
TEST(DataBufferTest, LoadChunk)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    ASSERT_EQ(buffer.total_chunks(), expected_chunks());
    ASSERT_TRUE(buffer.load_chunk(0));
    EXPECT_EQ(buffer.recent().m_id, 0);
    for (std::uint32_t i = 1; i < buffer.total_chunks(); ++i)
    {
        ASSERT_TRUE(buffer.load_chunk(i));
        EXPECT_EQ(buffer.recent().m_id, buffer.fallback().m_id + 1);
    }
}

// This is the same test as LoadChunk but this time the chunks get loaded
// in reverse order.
TEST(DataBufferTest, LoadChunkReverse)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    ASSERT_EQ(buffer.total_chunks(), expected_chunks());
    ASSERT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    EXPECT_EQ(buffer.recent().m_id, buffer.total_chunks() - 1);
    for (std::uint32_t i = buffer.total_chunks() - 2; i > 0; --i)
    {
        ASSERT_TRUE(buffer.load_chunk(i));
        EXPECT_EQ(buffer.recent().m_id, buffer.fallback().m_id - 1);
    }
}

// When a byte gets modified, the buffer should keep the new value
// internally as well as mark the buffer as "dirty". The actual contents
// of the underlying IOHandler will not get updated until save gets called.
TEST(DataBufferTest, DataModification)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    const auto    byte_id     = buffer.size() - 1;
    std::uint8_t* expectation = handler.data();
    expectation[byte_id]      = 0x0F;
    ASSERT_TRUE(buffer.open(file_name));
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

// DataBuffer should have access to all the bytes that the underlying
// IOHandler does.
TEST(DataBufferTest, DataRead)
{
    IOHandlerMock handler;
    std::uint8_t* expectation = handler.data();
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    // start from the first chunk and the first byte
    ASSERT_TRUE(buffer.load_chunk(0));
    ASSERT_EQ(buffer.recent().m_id, 0);
    bool match = true;
    for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

// This is the same test as DataRead but this time the bytes get accessed
// in reverse order.
TEST(DataBufferTest, DataReadReverse)
{
    IOHandlerMock handler;
    std::uint8_t* expectation = handler.data();
    DataBuffer    buffer(handler);
    ASSERT_TRUE(buffer.open(file_name));
    // start from the last chunk and the last byte
    ASSERT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    EXPECT_EQ(buffer.recent().m_id, buffer.total_chunks() - 1);
    bool match = true;
    for (std::uint32_t i = buffer.size() - 1; i > 0 && match; --i)
        match = buffer[i] == expectation[i];
    EXPECT_TRUE(match);
}

// When a chunk gets saved, the underlying IOHandler should get updated.
TEST(DataBufferTest, SaveChunk)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    DataBuffer    buffer(handler);
    const auto    chunk_id = buffer.total_chunks() / 2;
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(buffer.open(file_name));
    ASSERT_TRUE(buffer.load_chunk(chunk_id));
    auto& data_chunk = buffer.recent();
    EXPECT_EQ(chunk_id, data_chunk.m_id);
    std::memset(data_chunk.m_data, 0xEF, data_chunk.m_count);
    std::uint8_t* expectation = raw_data + (chunk_id * ChunkCache::capacity);
    EXPECT_NE(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_TRUE(buffer.save_chunk(data_chunk));
    EXPECT_EQ(std::memcmp(expectation, data_chunk.m_data, data_chunk.m_count), 0);
    EXPECT_EQ(expectation[0], 0xEF);
}

// Setting bytes using DataBuffer will not update the actual data
// until save() gets called.
TEST(DataBufferTest, SaveAllChunks)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    DataBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(buffer.open(file_name));
    std::uint32_t dirty_ids[] = { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(buffer.load_chunk(id));
        ChunkCache::DataChunk data_chunk = buffer.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            buffer.set_byte(id * ChunkCache::capacity + i, id + 1);
            EXPECT_EQ(buffer[id * ChunkCache::capacity + i], id + 1);
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            // The recent chunk should not change
            ASSERT_EQ(id, buffer.recent().m_id);
        }
    }
    // save all the dirty bytes
    buffer.save();
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(buffer.load_chunk(id));
        ChunkCache::DataChunk data_chunk = buffer.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            EXPECT_EQ(expectation[i], buffer[id * ChunkCache::capacity + i]);
            EXPECT_EQ(expectation[i], id + 1);
            // The recent chunk should not change
            ASSERT_EQ(id, buffer.recent().m_id);
        }
    }
}

// Same as SaveAllChunks but this time the read only flag is set to true
// and as a result it should not be possible to modify the actual data.
TEST(DataBufferTest, SaveAllChunksReadOnly)
{
    IOHandlerMock handler;
    std::uint8_t* raw_data = handler.data();
    DataBuffer    buffer(handler);
    // initialize the data to zero
    std::memset(raw_data, 0, handler.size());
    ASSERT_TRUE(buffer.open(file_name, true));
    std::uint32_t dirty_ids[] = { 0, 10, 50, 80, 100, 140, 150, 200, 249 };
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(buffer.load_chunk(id));
        ChunkCache::DataChunk data_chunk = buffer.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            buffer.set_byte(id * ChunkCache::capacity + i, id + 1);
            EXPECT_EQ(buffer[id * ChunkCache::capacity + i], id + 1);
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            // The recent chunk should not change
            ASSERT_EQ(id, buffer.recent().m_id);
        }
    }
    // save all the dirty bytes
    buffer.save();
    for (auto id : dirty_ids)
    {
        ASSERT_TRUE(buffer.load_chunk(id));
        ChunkCache::DataChunk data_chunk = buffer.recent();
        EXPECT_EQ(id, data_chunk.m_id);
        std::uint8_t* expectation = raw_data + (id * ChunkCache::capacity);
        for (std::size_t i = 0; i < data_chunk.m_count; ++i)
        {
            EXPECT_NE(expectation[i], buffer[id * ChunkCache::capacity + i]);
            EXPECT_NE(expectation[i], id + 1);
            // The recent chunk should not change
            ASSERT_EQ(id, buffer.recent().m_id);
        }
    }
}
