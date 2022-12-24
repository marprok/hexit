#ifndef SIGNATURE_READER_H
#define SIGNATURE_READER_H

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

class SignatureReader
{
public:
    struct SignatureQuery
    {
        std::array<std::uint8_t, 32> m_buffer = {};
        std::size_t                  m_size;
    };

    SignatureReader();
    ~SignatureReader() = default;

    std::string get_type(const SignatureQuery& query);

private:
    std::vector<std::pair<std::vector<std::uint8_t>, const std::string>> m_signatures;
};

#endif // SIGNATURE_READER_H
