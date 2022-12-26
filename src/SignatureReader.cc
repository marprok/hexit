#include "SignatureReader.h"
#include <algorithm>
#include <cstring>

SignatureReader::SignatureReader()
{
    m_signatures.push_back({ { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C }, "asf", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01 }, "jpg", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE1, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 }, "jpg", { 4, 5 } });
    m_signatures.push_back({ { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }, "png", {} });
    m_signatures.push_back({ { 0x52, 0x49, 0x46, 0x46, 0x57, 0x41, 0x56, 0x45 }, "wav", { 4, 5, 6, 7 } });
    m_signatures.push_back({ { 0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E, 0x0A }, "deb", {} });
    m_signatures.push_back({ { 0x42, 0x4C, 0x45, 0x4E, 0x44, 0x45, 0x52 }, "blend", {} });
    m_signatures.push_back({ { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 }, "gif", {} });
    m_signatures.push_back({ { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 }, "gif", {} });
    m_signatures.push_back({ { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C }, "7z", {} });
    m_signatures.push_back({ { 0x25, 0x50, 0x44, 0x46, 0x2D }, "pdf", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xDB }, "jpg", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xEE }, "jpg", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0 }, "jpg", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x03, 0x04 }, "zip", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x05, 0x06 }, "zip", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x07, 0x08 }, "zip", {} });
    m_signatures.push_back({ { 0x4F, 0x67, 0x67, 0x53 }, "ogg", {} });
    m_signatures.push_back({ { 0x66, 0x4C, 0x61, 0x43 }, "flac", {} });
    m_signatures.push_back({ { 0x1A, 0x45, 0xDF, 0xA3 }, "mkv", {} });
    m_signatures.push_back({ { 0x00, 0x61, 0x73, 0x6D }, "wasm", {} });
    m_signatures.push_back({ { 0x00, 0x00, 0x01, 0xBA }, "mpeg", {} });
    m_signatures.push_back({ { 0x00, 0x00, 0x01, 0xB3 }, "mpeg", {} });
    m_signatures.push_back({ { 0x4E, 0x45, 0x53, 0x1A }, "nes", {} });
    m_signatures.push_back({ { 0x49, 0x44, 0x33 }, "mp3", {} });
    m_signatures.push_back({ { 0x4E, 0x45, 0x53 }, "nes", {} });
    m_signatures.push_back({ { 0x1F, 0x8B }, "gz", {} });
    m_signatures.push_back({ { 0xFF, 0xFB }, "mp3", {} });
    m_signatures.push_back({ { 0xFF, 0xF3 }, "mp3", {} });
    m_signatures.push_back({ { 0xFF, 0xF2 }, "mp3", {} });
    m_signatures.push_back({ { 0x42, 0x4D }, "bmp", {} });
}

std::string SignatureReader::get_type(const std::vector<std::uint8_t>& query)
{
    for (const auto& signature : m_signatures)
    {
        std::size_t signature_size = signature.m_buffer.size() + signature.m_skip.size();
        if ((query.size() < signature_size)
            || (signature.m_skip.size() >= signature.m_buffer.size()))
            continue;

        std::size_t i = 0, j = 0;
        bool        match = false;
        for (; (i < signature_size) && (j < signature.m_buffer.size());)
        {
            if (signature.m_skip.contains(i))
            {
                i++;
                continue;
            }

            match = (query[i++] == signature.m_buffer[j++]);
            if (!match)
                break;
        }

        if ((j == signature.m_buffer.size()) && match)
            return signature.m_type;
    }

    return "unk";
}
