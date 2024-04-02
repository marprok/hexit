#ifndef HEXIT_CONFIG_H
#define HEXIT_CONFIG_H

#include <cstdint>

namespace Hexit
{
inline constexpr std::uint32_t BYTES_PER_LINE = 16;
// The distance between hex and ASCII digits.
inline constexpr std::uint32_t ASCII_PADDING = 2;
// The distatnce between the line byte offset and the ASCII digits.
inline constexpr std::uint32_t HEX_PADDING = 2;
// The number of hex digits used for the line byte offset.
inline constexpr std::uint32_t LINE_OFFSET_LEN = sizeof(std::uintmax_t) * 2;
inline constexpr std::uint32_t FIRST_HEX       = LINE_OFFSET_LEN + 1 + HEX_PADDING;
inline constexpr std::uint32_t FIRST_ASCII     = FIRST_HEX + BYTES_PER_LINE * 3 - 1 + ASCII_PADDING;

// Special key sequences.
inline constexpr int CTRL_Q = 'q' & 0x1F;
inline constexpr int CTRL_S = 's' & 0x1F;
inline constexpr int CTRL_X = 'x' & 0x1F;
inline constexpr int CTRL_A = 'a' & 0x1F;
inline constexpr int CTRL_Z = 'z' & 0x1F;
inline constexpr int CTRL_G = 'g' & 0x1F;

// Feel free to map the controls to the keys of your choice :)
inline constexpr int K_QUIT  = CTRL_Q; // Quit
inline constexpr int K_SAVE  = CTRL_S; // Save
inline constexpr int K_HEX   = CTRL_X; // HEX mode
inline constexpr int K_ASCII = CTRL_A; // ASCII mode
inline constexpr int K_SUSP  = CTRL_Z; // Suspend
inline constexpr int K_GO_TO = CTRL_G; // Go to byte
} // namespace Hexit

#endif // HEXIT_CONFIG_H
