#pragma once

#include <string_view>
#include <type_traits>

namespace tapsdk {

class DeleteCopyAndMove {
public:
    DeleteCopyAndMove(const DeleteCopyAndMove&) = delete;
    DeleteCopyAndMove(DeleteCopyAndMove&&) = delete;
    virtual ~DeleteCopyAndMove() = default;
    DeleteCopyAndMove& operator=(const DeleteCopyAndMove&) = delete;
    DeleteCopyAndMove& operator=(DeleteCopyAndMove&&) = delete;

protected:
    DeleteCopyAndMove() = default;
};

#define DECLARE_ENUM_FLAG_OPERATORS(type)                                                          \
    [[nodiscard]] constexpr type operator|(type a, type b) noexcept {                              \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(static_cast<T>(a) | static_cast<T>(b));                           \
    }                                                                                              \
    [[nodiscard]] constexpr type operator&(type a, type b) noexcept {                              \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(static_cast<T>(a) & static_cast<T>(b));                           \
    }                                                                                              \
    [[nodiscard]] constexpr type operator^(type a, type b) noexcept {                              \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(static_cast<T>(a) ^ static_cast<T>(b));                           \
    }                                                                                              \
    [[nodiscard]] constexpr type operator<<(type a, type b) noexcept {                             \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(static_cast<T>(a) << static_cast<T>(b));                          \
    }                                                                                              \
    [[nodiscard]] constexpr type operator>>(type a, type b) noexcept {                             \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(static_cast<T>(a) >> static_cast<T>(b));                          \
    }                                                                                              \
    constexpr type& operator|=(type& a, type b) noexcept {                                         \
        a = a | b;                                                                                 \
        return a;                                                                                  \
    }                                                                                              \
    constexpr type& operator&=(type& a, type b) noexcept {                                         \
        a = a & b;                                                                                 \
        return a;                                                                                  \
    }                                                                                              \
    constexpr type& operator^=(type& a, type b) noexcept {                                         \
        a = a ^ b;                                                                                 \
        return a;                                                                                  \
    }                                                                                              \
    constexpr type& operator<<=(type& a, type b) noexcept {                                        \
        a = a << b;                                                                                \
        return a;                                                                                  \
    }                                                                                              \
    constexpr type& operator>>=(type& a, type b) noexcept {                                        \
        a = a >> b;                                                                                \
        return a;                                                                                  \
    }                                                                                              \
    [[nodiscard]] constexpr type operator~(type key) noexcept {                                    \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<type>(~static_cast<T>(key));                                            \
    }                                                                                              \
    [[nodiscard]] constexpr bool True(type key) noexcept {                                         \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<T>(key) != 0;                                                           \
    }                                                                                              \
    [[nodiscard]] constexpr bool False(type key) noexcept {                                        \
        using T = std::underlying_type_t<type>;                                                    \
        return static_cast<T>(key) == 0;                                                           \
    }

constexpr bool EndWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

}  // namespace tapsdk