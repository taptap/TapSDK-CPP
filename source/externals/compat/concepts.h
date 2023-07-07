#pragma once

#include <concepts>

namespace std {
template<class _From, class _To>
concept convertible_to =
        std::is_convertible_v<_From, _To> &&
        requires {
            static_cast<_To>(declval<_From>());
        };
}