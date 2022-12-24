#include "SignatureReader.h"
#include <algorithm>
#include <cstring>

SignatureReader::SignatureReader()
{
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01 }, "JPG" });
    m_signatures.push_back({ { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }, "PNG" });
    m_signatures.push_back({ { 0x25, 0x50, 0x44, 0x46, 0x2D }, "PDF" });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xDB }, "JPG" });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xEE }, "JPG" });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0 }, "JPG" });
    m_signatures.push_back({ { 0x50, 0x4B, 0x03, 0x04 }, "ZIP" });
    m_signatures.push_back({ { 0x50, 0x4B, 0x05, 0x06 }, "ZIP" });
    m_signatures.push_back({ { 0x50, 0x4B, 0x07, 0x08 }, "ZIP" });
    m_signatures.push_back({ { 0x1F, 0x8B }, "GZIP" });

    // handle this?
    // m_signatures.push_back({ { 0xFF, 0xD8, FF, E1, ?? ??, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 }, "JPG" });
}

std::string SignatureReader::get_type(const SignatureQuery& query)
{
    for (const auto& signature : m_signatures)
    {
        if (query.m_size < signature.first.size())
            continue;

        if (std::memcmp(query.m_buffer.data(), signature.first.data(), signature.first.size()) == 0)
            return signature.second;
    }
    return "UNK";
}
