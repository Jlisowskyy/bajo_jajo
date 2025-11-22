#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <cstdint>

#define NODISCARD   [[nodiscard]]
#define FUNC_INLINE inline __attribute__((always_inline))

// static constexpr std::uint64_t kSeed = 0xAB98142De;
// static constexpr std::uint64_t kSeed = 0x1;
static constexpr std::uint64_t kSeed = 0xDEADC0DE;
// static constexpr std::uint64_t kSeed = 0xBEEFBEEF;
// static constexpr std::uint64_t kSeed = 0xDEADBEEF;
// static constexpr std::uint64_t kSeed = 0xAEAEAE;
// static constexpr std::uint64_t kSeed = 0xFEFEFE;
// static constexpr std::uint64_t kSeed = 0xFFFFFF;

#endif  // DEFINES_HPP
