#include "SignatureReader.h"
#include <algorithm>
#include <gtest/gtest.h>

namespace
{
SignatureReader::SignatureQuery make_query(const std::vector<std::uint8_t>& bytes)
{
    if (bytes.empty())
        return {};

    SignatureReader::SignatureQuery query;
    std::size_t                     bytes_to_copy = std::min(bytes.size(), query.m_buffer.max_size());

    for (std::size_t i = 0u; i < bytes_to_copy; ++i)
        query.m_buffer[i] = bytes[i];

    query.m_size = bytes_to_copy;

    return query;
}
}

TEST(SignatureReaderTest, SignatureMapping)
{
    SignatureReader reader;

    // Empty query
    {
        auto query = make_query({});
        EXPECT_EQ(reader.get_type(query), std::string(""));
    }

    // The query bytes match exactly with a signature
    {
        auto query = make_query({ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A });
        EXPECT_EQ(reader.get_type(query), std::string("PNG"));
    }
    // There are some extra query bytes at the end of the valid signature sequence
    {
        auto query = make_query({ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0xFF, 0x09, 0x43 });
        EXPECT_EQ(reader.get_type(query), std::string("PNG"));
    }
    // There are extra query bytes at the beginning of a valid signature sequence
    {
        auto query = make_query({ 0x43, 0xDD, 0x76, 0x00, 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A });
        EXPECT_EQ(reader.get_type(query), std::string(""));
    }
}
