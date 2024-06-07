#pragma once

#include <vector>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <functional>

#include <flow/heterogeneous/visitor.hpp>

namespace flow {

    class heterogeneous_container {
    public:
        heterogeneous_container() = default;

        heterogeneous_container(heterogeneous_container const & other) {
            copy(other);
        }

        heterogeneous_container & operator=(heterogeneous_container const & rhs) {
            clear();
            copy(rhs);
            return *this;
        }

        ~heterogeneous_container() noexcept {
            clear();
        }

        template <typename T>
        void push_back(T && value) {
            emplace_back<std::decay_t<T>>(std::forward<T>(value));
        }

        template <decayed T, typename... Args>
        requires (std::constructible_from<T, Args...>)
        void emplace_back(Args &&... args) {
            if (!s_data<T>.contains(this)) {
                m_clear_functions.emplace_back([this](heterogeneous_container const * c) {
                    s_data<T>.erase(c);
                });
                m_copy_functions.emplace_back([this](heterogeneous_container const & from,
                                                     heterogeneous_container const & to) {
                    s_data<T>[std::addressof(to)] = s_data<T>[std::addressof(from)];
                });
            }
            s_data<T>[this].emplace_back(std::forward<Args>(args)...);
            ++m_size;
        }

        template <decayed T>
        std::size_t erase() {
            std::size_t erased_count = 0;
            if (auto const it = s_data<T>.find(this); it != s_data<T>.end()) {
                erased_count = it->second.size();
                m_size -= erased_count;
                s_data<T>.erase(it);
            }
            return erased_count;
        }

        template <std::input_or_output_iterator It>
        void erase(It it) {
            s_data<std::iter_value_t<It>>[this].erase(it);
            --m_size;
        }

        template <container_visitor Visitor>
        void visit(Visitor visitor) & {
            visit_impl(std::move(visitor), typename detail::unwrap_ref_t<Visitor>::types{});
        }

        template <container_visitor Visitor>
        void visit(Visitor visitor) const & {
            visit_impl(std::move(visitor), typename detail::unwrap_ref_t<Visitor>::types{});
        }

        template <container_visitor Visitor>
        void visit(Visitor visitor) && {
            visit_impl(std::move(visitor), typename detail::unwrap_ref_t<Visitor>::types{});
        }

        template <container_visitor Visitor>
        void visit(Visitor visitor) const && {
            visit_impl(std::move(visitor), typename detail::unwrap_ref_t<Visitor>::types{});
        }

        template <decayed T>
        [[nodiscard]] auto common_elements() noexcept {
           return std::views::all(s_data<T>[this]);
        }

        template <decayed T>
        [[nodiscard]] auto common_elements() const noexcept {
            return std::views::all(s_data<T>[this] | std::views::as_const);
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return m_size;
        }

        void clear() noexcept {
            std::ranges::for_each(m_clear_functions, [this](auto && clear_fn) {
                clear_fn(this);
            });
            m_clear_functions.clear();
            m_copy_functions.clear();
            m_size = 0;
        }

    private:
        void copy(heterogeneous_container const & other) {
            m_clear_functions = other.m_clear_functions;
            m_copy_functions = other.m_copy_functions;
            std::ranges::for_each(m_copy_functions, [&](auto && copy_fn) {
                copy_fn(other, *this);
            });
            m_size = other.m_size;
        }

        template <typename... Types>
        void visit_impl(auto visitor, type_list<Types...>) & {
            (..., std::ranges::for_each(s_data<Types>[this], visitor));
        }

        template <typename... Types>
        void visit_impl(auto visitor, type_list<Types...>) const & {
            (..., std::ranges::for_each(s_data<Types>[this], visitor));
        }

        template <typename... Types>
        void visit_impl(auto visitor, type_list<Types...>) && {
            (..., std::ranges::for_each(s_data<Types>[this] | std::views::as_rvalue, visitor));
        }

        template <typename... Types>
        void visit_impl(auto visitor, type_list<Types...>) const && {
            (..., std::ranges::for_each(s_data<Types>[this] | std::views::as_rvalue, visitor));
        }

    private:
        std::size_t m_size = 0;

        std::vector<std::function<void(heterogeneous_container const *)>> m_clear_functions;
        std::vector<std::function<void(heterogeneous_container const &,
                                       heterogeneous_container const &)>> m_copy_functions;

        template <typename T>
        static std::unordered_map<heterogeneous_container const *, std::vector<T>> s_data;
    };

    template <typename T>
    std::unordered_map<heterogeneous_container const *, std::vector<T>> heterogeneous_container::s_data;

}
