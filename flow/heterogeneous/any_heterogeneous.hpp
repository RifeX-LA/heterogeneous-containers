#pragma once

#include <any>
#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>

#include <flow/heterogeneous/visitor.hpp>
#include <flow/heterogeneous/common.hpp>

namespace flow {

    namespace detail {

        // Применить для std::any функцию-посетитель.
        // Для этого перебираем все типы из type_list и,
        // если std::any хранит указанный тип, то применяем функцию,
        // иначе переходим к следующему типу
        template <typename Visitor, typename AnyT, decayed Head, decayed... Tail>
        void visit_any(Visitor visitor, AnyT && any, type_list<Head, Tail...>) {
            if (any.type() == typeid(Head)) {
                std::invoke(visitor, std::any_cast<Head &>(std::forward<AnyT>(any)));
            }
            else if constexpr (sizeof...(Tail) > 0) {
                detail::visit_any(std::move(visitor), std::forward<AnyT>(any), type_list<Tail...>{});
            }
        }

        // Применить функцию-посетитель ко всем std::any в диапазоне
        template <std::ranges::input_range Rng, container_visitor Visitor>
        void visit_any_range(Rng && rng, Visitor visitor) {
            std::ranges::for_each(std::forward<Rng>(rng), [&]<typename AnyT>(AnyT && any){
                visit_any(visitor, std::forward<AnyT>(any), typename unwrap_ref_t<Visitor>::types{});
            });
        }

        // Содержит ли std::any тип T
        template <decayed T>
        struct any_holds_type_t {
            [[nodiscard]] bool operator()(std::any const & any) const noexcept {
                return any.type() == typeid(T);
            }
        };

        template <decayed T>
        inline constexpr any_holds_type_t<T> any_holds_type;


        // Получить хранимый в std::any объект типа T
        template <decayed T>
        struct any_get_t {
            template <typename AnyT>
            [[nodiscard]] decltype(auto) operator()(AnyT && any) const noexcept {
                return std::any_cast<T&>(std::forward<AnyT>(any));
            }
        };

        template <decayed T>
        inline constexpr any_get_t<T> any_get;

    }

    class any_heterogeneous_container {
    public:
        template <typename T>
        requires (std::copy_constructible<std::decay_t<T>>)
        void push_back(T && value) {
            m_data.emplace_back(std::forward<T>(value));
        }

        template <decayed T, typename... Args>
        requires (std::constructible_from<T, Args...> && std::copy_constructible<T>)
        void emplace_back(Args &&... args) {
            m_data.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
        }

        template <decayed T>
        std::size_t erase() {
            return std::erase_if(m_data, detail::any_holds_type<T>);
        }

        void erase(std::input_or_output_iterator auto it) {
            m_data.erase(detail::get_based_iterator(it));
        }

        template <decayed T>
        [[nodiscard]] auto common_elements() noexcept {
            return m_data | std::views::filter(detail::any_holds_type<T>) | std::views::transform(detail::any_get<T>);
        }

        template <decayed T>
        [[nodiscard]] auto common_elements() const noexcept {
            return m_data | std::views::filter(detail::any_holds_type<T>) | std::views::transform(detail::any_get<T>);
        }

        void visit(container_visitor auto visitor) & {
            detail::visit_any_range(m_data, std::move(visitor));
        }

        void visit(container_visitor auto visitor) const & {
            detail::visit_any_range(m_data, std::move(visitor));
        }

        void visit(container_visitor auto visitor) && {
            detail::visit_any_range(m_data | std::views::as_rvalue, std::move(visitor));
        }

        void visit(container_visitor auto visitor) const && {
            detail::visit_any_range(m_data | std::views::as_rvalue, std::move(visitor));
        }

    private:
        std::vector<std::any> m_data;
    };

}
