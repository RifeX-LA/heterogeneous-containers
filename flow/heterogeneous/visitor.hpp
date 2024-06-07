#pragma once

#include <concepts>
#include <type_traits>

namespace flow {

    template <typename T>
    concept decayed = std::same_as<T, std::decay_t<T>>;

    template <typename...>
    struct type_list {};

    template <decayed... Types>
    struct visitor {
        using types = type_list<Types...>;
    };

    namespace detail {

        template<typename>
        struct is_type_list : std::false_type {};

        template <typename... Types>
        struct is_type_list<type_list<Types...>> : std::true_type {};


        template <typename T>
        using unwrap_ref_t = std::remove_reference_t<std::unwrap_ref_decay_t<T>>;


        template <typename T>
        concept container_visitor = requires {
            typename T::types;
            is_type_list<typename T::types>::value;
        };
    }


    template <typename T>
    concept container_visitor = detail::container_visitor<detail::unwrap_ref_t<T>>;
}