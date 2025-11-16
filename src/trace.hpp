#ifndef TRACE_HPP
#define TRACE_HPP

#include <iostream>
#include <string_view>

namespace detail
{
template <typename T, typename... Args>
void print_trace_args(std::ostream &os, const T &first, const Args &...rest)
{
    os << first;
    if constexpr (sizeof...(rest) > 0) {
        os << " ";
        print_trace_args(os, rest...);
    }
}

template <typename... Args>
void trace_impl(std::string_view file, int line, std::string_view func, const Args &...args)
{
    if (IsDebug()) {
        std::cerr << "[" << file << ":" << line << " (" << func << ")] ";
        print_trace_args(std::cerr, args...);
        std::cerr << std::endl;
    }
}

}  // namespace detail

#define TRACE(...) ::detail::trace_impl(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif  // TRACE_HPP
