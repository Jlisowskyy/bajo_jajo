#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <cstdint>

#define NODISCARD   [[nodiscard]]
#define FUNC_INLINE inline __attribute__((always_inline))

// static constexpr std::uint64_t kSeed = 0xAB98142De;
// static constexpr std::uint64_t kSeed = 0xEEEE;
// static constexpr std::uint64_t kSeed = 0xEEEE;
// static constexpr std::uint64_t kSeed = 0xABCD;
static constexpr std::uint64_t kSeed = 0x1;

#endif  // DEFINES_HPP
