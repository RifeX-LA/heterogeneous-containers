#pragma once

#include <type_traits>
#include <iterator>

namespace flow::detail {

    // Получить базовый итератор
    template <std::input_or_output_iterator It>
    [[nodiscard]] auto get_based_iterator(It it) noexcept {
        if constexpr (requires { it.base(); }) {
            return get_based_iterator(it.base());
        }
        else {
            return it;
        }
    }
}