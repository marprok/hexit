#ifndef SIGNATURE_READER_H
#define SIGNATURE_READER_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace Hexit
{
class SignatureReader
{
public:
    SignatureReader();
    ~SignatureReader() = default;

    std::string get_type(const std::vector<std::uint8_t>& query);

private:
    struct Signature
    {
        std::vector<std::uint8_t> m_buffer; // Byte values to compare.
        std::string               m_type;   // Name of the file type.
        std::set<std::size_t>     m_skip;   // A set of byte indexes in the query whose value should not be taken into account.
                                            // For example, the signature for a WAV file is: 0x52 0x49 0x46 0x46 ?? ?? ?? ?? 0x57 0x41 0x56 0x45
                                            // The fields of the corresponding Signature object should be:
                                            // m_buffer: 0x52 0x49 0x46 0x46 0x57 0x41 0x56 0x45
                                            // m_type: "wav"
                                            // m_skip: 4,5,6,7
    };
    std::vector<Signature> m_signatures;
};
} // namespace Hexit
#endif // SIGNATURE_READER_H
