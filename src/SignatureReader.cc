#include "SignatureReader.h"
#include <algorithm>
#include <cstring>

SignatureReader::SignatureReader()
{
    m_signatures.push_back({ { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C }, "ASF", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01 }, "JPG", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE1, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 }, "JPG", { 4, 5 } });
    m_signatures.push_back({ { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }, "PNG", {} });
    m_signatures.push_back({ { 0x52, 0x49, 0x46, 0x46, 0x57, 0x41, 0x56, 0x45 }, "WAV", { 4, 5, 6, 7 } });
    m_signatures.push_back({ { 0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E, 0x0A }, "DEB", {} });
    m_signatures.push_back({ { 0x42, 0x4C, 0x45, 0x4E, 0x44, 0x45, 0x52 }, "BLEND", {} });
    m_signatures.push_back({ { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 }, "GIF", {} });
    m_signatures.push_back({ { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 }, "GIF", {} });
    m_signatures.push_back({ { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C }, "7z", {} });
    m_signatures.push_back({ { 0x25, 0x50, 0x44, 0x46, 0x2D }, "PDF", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xDB }, "JPG", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xEE }, "JPG", {} });
    m_signatures.push_back({ { 0xFF, 0xD8, 0xFF, 0xE0 }, "JPG", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x03, 0x04 }, "ZIP", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x05, 0x06 }, "ZIP", {} });
    m_signatures.push_back({ { 0x50, 0x4B, 0x07, 0x08 }, "ZIP", {} });
    m_signatures.push_back({ { 0x4F, 0x67, 0x67, 0x53 }, "OGG", {} });
    m_signatures.push_back({ { 0x66, 0x4C, 0x61, 0x43 }, "FLAC", {} });
    m_signatures.push_back({ { 0x1A, 0x45, 0xDF, 0xA3 }, "MKV", {} });
    m_signatures.push_back({ { 0x00, 0x61, 0x73, 0x6D }, "WASM", {} });
    m_signatures.push_back({ { 0x00, 0x00, 0x01, 0xBA }, "MPEG", {} });
    m_signatures.push_back({ { 0x00, 0x00, 0x01, 0xB3 }, "MPEG", {} });
    m_signatures.push_back({ { 0x4E, 0x45, 0x53, 0x1A }, "NES", {} });
    m_signatures.push_back({ { 0x49, 0x44, 0x33 }, "MP3", {} });
    m_signatures.push_back({ { 0x4E, 0x45, 0x53 }, "NES", {} });
    m_signatures.push_back({ { 0x1F, 0x8B }, "GZ", {} });
    m_signatures.push_back({ { 0xFF, 0xFB }, "MP3", {} });
    m_signatures.push_back({ { 0xFF, 0xF3 }, "MP3", {} });
    m_signatures.push_back({ { 0xFF, 0xF2 }, "MP3", {} });
    m_signatures.push_back({ { 0x42, 0x4D }, "BMP", {} });
}

std::string SignatureReader::get_type(const SignatureQuery& query)
{
    for (const auto& signature : m_signatures)
    {
        std::size_t signature_size = signature.m_buffer.size() + signature.m_skip.size();
        if (query.m_size < signature_size)
            continue;

        bool match = false;
        for (std::size_t i = 0, j = 0; i < signature_size && j < signature.m_buffer.size();)
        {
            if (signature.m_skip.contains(i))
            {
                i++;
                continue;
            }

            match = (query.m_buffer[i++] == signature.m_buffer[j++]);
            if (!match)
                break;
        }

        if (match)
            return signature.m_type;
    }

    return "UNK";
}
