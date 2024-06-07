#pragma once

#include <variant>
#include <vector>
#include <ranges>
#include <algorithm>

#include <flow/heterogeneous/visitor.hpp>
#include <flow/heterogeneous/common.hpp>

namespace flow {

    namespace detail {

        // Можно ли создать std::variant<VariantTypes...> с помощью типа T
        template <typename T, typename... VariantTypes>
        concept can_create_type = requires { std::variant<VariantTypes...>(std::declval<T>()); };

        // Существует ли тип T в списке типов Types...
        template <typename T, typename... Types>
        concept exists_in = std::disjunction_v<std::is_same<T, Types>...>;


        // Применить функцию-посетитель ко всем std::variant в диапазоне
        template <std::ranges::input_range Rng>
        void visit_variant_range(Rng && rng, auto visitor) {
            std::ranges::for_each(std::forward<Rng>(rng), [&]<typename Variant>(Variant && variant) {
                std::visit(visitor, std::forward<Variant>(variant));
            });
        }

        // Содержит ли std::variant тип T
        template <typename T>
        struct variant_holds_type_t {
            template <typename... Types>
            [[nodiscard]] constexpr bool operator()(std::variant<Types...> const & variant) const noexcept {
                return std::holds_alternative<T>(variant);
            }
        };

        template <typename T>
        inline constexpr variant_holds_type_t<T> variant_holds_type;


        // Получить хранимый в std::variant объект типа T
        template <typename T>
        struct variant_get_t {
            template <typename Variant>
            [[nodiscard]] constexpr decltype(auto) operator()(Variant && variant) const noexcept {
                return std::get<T>(std::forward<Variant>(variant));
            }
        };

        template <typename T>
        inline constexpr variant_get_t<T> variant_get;

    }

    template <decayed... Types>
    class variant_heterogeneous_container {
    public:
        template <detail::can_create_type<Types...> T>
        void push_back(T && value) {
            m_data.emplace_back(std::forward<T>(value));
        }

        template <detail::exists_in<Types...> T, typename... Args>
        requires (std::constructible_from<T, Args...>)
        void emplace_back(Args &&... args) {
            m_data.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
        }

        template <detail::exists_in<Types...> T>
        std::size_t erase() {
            return std::erase_if(m_data, detail::variant_holds_type<T>);
        }

        void erase(auto it) {
            m_data.erase(detail::get_based_iterator(it));
        }

        void clear() {
            m_data.clear();
        }

        template <detail::exists_in<Types...> T>
        [[nodiscard]] auto common_elements() noexcept {
            return m_data | std::views::filter(detail::variant_holds_type<T>) | std::views::transform(detail::variant_get<T>);
        }

        template <detail::exists_in<Types...> T>
        [[nodiscard]] auto common_elements() const noexcept {
            return m_data | std::views::filter(detail::variant_holds_type<T>) | std::views::transform(detail::variant_get<T>);
        }

        void visit(auto visitor) & {
            detail::visit_variant_range(m_data, std::move(visitor));
        }

        void visit(auto visitor) const & {
            detail::visit_variant_range(m_data, std::move(visitor));
        }

        void visit(auto visitor) && {
            detail::visit_variant_range(m_data | std::views::as_rvalue, std::move(visitor));
        }

        void visit(auto visitor) const && {
            detail::visit_variant_range(m_data | std::views::as_rvalue, std::move(visitor));
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return m_data.size();
        }

    private:
        std::vector<std::variant<Types...>> m_data;
    };

}
