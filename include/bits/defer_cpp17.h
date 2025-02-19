#ifndef DP_CPP17_DEFER
#define DP_CPP17_DEFER

#if defined(DP_CPP98_DEFER) || defined(DP_DEFER)
#error "Multiple different defer types detected. Use one or the other"
#endif

#include <type_traits>
#include <utility>
#include <functional>
#include <tuple>
#include <exception>

#include "bits/macros.h"

//Full documentation here: https://github.com/DryPerspective/C_Builder_Extras/wiki/Defer

namespace dp {

    template<typename Callable, typename... Args>
    class defer {

        using cleanup_type = std::decay_t<Callable>; //Function types (not function pointers) can't be stored but can still be "passed" to the ctor

        cleanup_type cleanup;
        std::tuple<Args...> call_args;

    public:
        //Template for forwarding references
        template<typename T, typename... TArgs,
            std::enable_if_t<std::is_constructible_v<std::tuple<Args...>, TArgs...>&&
            std::is_convertible_v<T, Callable>&&
            std::is_invocable_v<Callable, Args...>, bool> = true>
        defer(T&& t, TArgs&&... args) : cleanup{ std::forward<T>(t) }, call_args{ std::make_tuple(std::forward<TArgs...>(args...)) } {}

        //By definition, this is a scope-local construct. So moving/copying it makes no sense.
        defer(const defer&) = delete;
        defer(defer&&) = delete;
        defer& operator=(const defer&) = delete;
        defer& operator=(defer&&) = delete;

        ~defer() noexcept {
            static_assert(noexcept(std::apply(cleanup, call_args)), "Attempt to defer a non-noexcept task");
            std::apply(std::move(cleanup), std::move(call_args));
        }

    };
    template<typename T, typename... Args>
    defer(T, Args...) -> defer<T, Args...>;


    template<typename Callable>
    class defer<Callable> {

        using cleanup_type = std::decay_t<Callable>;

        cleanup_type cleanup;

    public:
        template<typename F,
            std::enable_if_t<std::is_convertible_v<F, Callable>&&
            std::is_invocable_v<F>, bool> = true>
        defer(F&& inFunc) : cleanup{ std::forward<F>(inFunc) } {}

        //By definition, this is a scope-local construct. So moving/copying it makes no sense.
        defer(const defer&) = delete;
        defer(defer&&) = delete;
        defer& operator=(const defer&) = delete;
        defer& operator=(defer&&) = delete;

        ~defer() noexcept {
            static_assert(noexcept(std::invoke(cleanup)), "Attempt to defer a non-noexcept task");
            std::invoke(cleanup);
        }
    };
    template<typename T>
    defer(T) -> defer<T>;
}



#define DEFER(ARGS) [[maybe_unused]] auto DP_UNIQUE_NAME(Defer_Struct) = dp::defer([&]() mutable noexcept {ARGS ;});






#endif