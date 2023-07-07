#pragma once

#include <cstdint>
#include <string>

namespace tapsdk {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

class WebPath : public std::string {
public:
    WebPath(const std::string& str) : std::string{str} {}

    WebPath(const char* str) : std::string{str} {}

    WebPath operator/(const WebPath& r) const { return *this + "/" + r; }
};

}  // namespace tapsdk