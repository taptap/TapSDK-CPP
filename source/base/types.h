#pragma once

#include <cstdint>
#include <string>
#include "fmt/format.h"

namespace tapsdk {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = std::array<u64, 2>;
static_assert(sizeof(u128) == 16, "u128 must be 128 bits wide");

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

constexpr inline u64 operator ""_KB(unsigned long long n) {
    return static_cast<u64>(n) * UINT64_C(1024);
}

constexpr inline u64 operator ""_MB(unsigned long long n) {
    return operator ""_KB(n) * UINT64_C(1024);
}

constexpr inline u64 operator ""_GB(unsigned long long n) {
    return operator ""_MB(n) * UINT64_C(1024);
}

class WebPath : public std::string {
public:
    WebPath(const std::string& str) : std::string{str} {}

    WebPath(const char* str) : std::string{str} {}

    WebPath operator/(const WebPath& r) const { return *this + "/" + r; }

    WebPath operator/(const std::string& r) const { return *this + "/" + r; }

    WebPath operator/(int r) const { return *this + "/" + fmt::format("{}", r); }
};

}  // namespace tapsdk