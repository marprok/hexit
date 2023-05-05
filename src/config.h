#ifndef HEXIT_CONFIG_H
#define HEXIT_CONFIG_H

namespace Hexit
{
inline constexpr bool          SUPPORT_BIG_FILES = false;
inline constexpr std::uint32_t BYTES_PER_LINE    = 16;
// The distance between hex and ASCII digits.
inline constexpr std::uint32_t ASCII_PADDING = 2;
// The distatnce between the line byte offset and the ASCII digits.
inline constexpr std::uint32_t HEX_PADDING = 2;
// The number of hex digits used for the line byte offset.
inline constexpr std::uint32_t LINE_OFFSET_LEN = SUPPORT_BIG_FILES ? sizeof(std::uint64_t) * 2 : sizeof(std::uint32_t) * 2;
inline constexpr std::uint32_t FIRST_HEX       = LINE_OFFSET_LEN + 1 + HEX_PADDING;
inline constexpr std::uint32_t FIRST_ASCII     = FIRST_HEX + BYTES_PER_LINE * 3 - 1 + ASCII_PADDING;

// Special key sequences.
inline constexpr int CTRL_Q = 'q' & 0x1F; // Quit
inline constexpr int CTRL_S = 's' & 0x1F; // Save
inline constexpr int CTRL_X = 'x' & 0x1F; // HEX mode
inline constexpr int CTRL_A = 'a' & 0x1F; // ASCII mode
inline constexpr int CTRL_Z = 'z' & 0x1F; // Suspend
inline constexpr int CTRL_G = 'g' & 0x1F; // Go to byte
} // namespace Hexit

#endif // HEXIT_CONFIG_H
