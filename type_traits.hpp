/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TYPE_TRAITS_HPP_
#define TYPE_TRAITS_HPP_

#include <type_traits>

namespace fp {
    template<typename...> struct type_seq;
    template<int...> struct int_seq;
    
    template<typename T>
    using Invoke = typename T::type;
    
    template<typename T>
    struct identity { using type = T; };
    
    template<typename T>
    using Identity = Invoke<identity<T>>;
    
    template<bool B, typename...>
    struct dependent_bool_type : std::integral_constant<bool, B> { };
    
    template<bool B, typename... T>
    using Bool = Invoke<dependent_bool_type<B, T...>>;
    
    template<typename T, T V>
    using Const = std::integral_constant<T, V>;
    
    template<int I>
    using Int = Const<decltype(I), I>;
    
    template<typename T>
    using Alias = T;
    
    template<typename T>
    using NotDeducible = Identity<T>;

    template<typename T>
    using RemoveCv = Invoke<std::remove_cv<T>>;

    template<typename T>
    using RemoveReference = Invoke<std::remove_reference<T>>;

    template<typename T>
    using Unqualified = RemoveCv<RemoveReference<T>>;
    
    template<typename If, typename Then, typename Else>
    using Conditional = Invoke<std::conditional<If::value, Then, Else>>;
    
    template<typename T>
    using Not = Bool<!T::value>;
    
    namespace impl {
        
        template<typename... T>
        struct any : Bool<false> { };
        template<typename H, typename... T>
        struct any<H, T...> : Conditional<H, Bool<true>, any<T...>> { };
        
        template<typename... T>
        struct all : Bool<true> { };
        template<typename H, typename... T>
        struct all<H, T...> : Conditional<H, all<T...>, Bool<false>> { };
    }
    
    template<typename... T> 
    struct Any : impl::any<T...> { };
    
    template<typename... T>
    struct All : impl::all<T...> { };
    
    namespace impl {
        
        template<typename...> struct first_type_of;
        template<typename...> struct last_type_of;
        template<int, typename...> struct nth_type_of;
        
        template<typename H, typename... T>
        struct first_type_of<H, T...> { using type = H; };

        template<typename H, typename... T>
        struct last_type_of<H, T...> : last_type_of<T...> { };
        template<typename T>
        struct last_type_of<T> { using type = T; };
        
        template<int I, typename H, typename... T>
        struct nth_type_of<I, H, T...> : nth_type_of<(I - 1), T...> { };
        template<typename H, typename... T>
        struct nth_type_of<0, H, T...> { using type = H; };
    }
    
    template<typename... T>
    using FirstTypeOf = Invoke<impl::first_type_of<T...>>;
    
    template<typename... T>
    using LastTypeOf = Invoke<impl::last_type_of<T...>>;
    
    template<int I, typename... T>
    using NthTypeOf = Invoke<impl::nth_type_of<I, T...>>;
    
    namespace impl {
        
        template<int...> struct first_value_of;
        template<int...> struct last_value_of;
        template<int, int...> struct nth_value_of;
        
        template<int H, int... T>
        struct first_value_of<H, T...> { enum { value = H }; };

        template<int H, int... T>
        struct last_value_of<H, T...> : last_value_of<T...> { };
        template<int T>
        struct last_value_of<T> { enum { value = T }; };
        
        template<int I, int H, int... T>
        struct nth_value_of<I, H, T...> : nth_value_of<(I - 1), T...> { };
        template<int H, int... T>
        struct nth_value_of<0, H, T...> { enum { value = H }; };
    }
    
    template<int... T>
    using FirstValueOf = Invoke<impl::first_value_of<T...>>;
    
    template<int... T>
    using LastValueOf = Invoke<impl::last_value_of<T...>>;
    
    template<int I, int... T>
    using NthValueOf = Invoke<impl::nth_value_of<I, T...>>;
    
    namespace impl {
        template<int, typename...>
        struct skip_n_types;
        
        template<int N, typename H, typename... T>
        struct skip_n_types<N, H, T...> : skip_n_types<(N - 1), T...> { };
        
        template<typename... T>
        struct skip_n_types<0, T...> { using type = type_seq<T...>; };
        
        template<int, int...>
        struct skip_n_values;
        
        template<int N, int H, int... T>
        struct skip_n_values<N, H, T...> : skip_n_values<(N - 1), T...> { };
        
        template<int... T>
        struct skip_n_values<0, T...> { using type = int_seq<T...>; };
    }
    
    template<int N, typename... T>
    using SkipTypes = Invoke<impl::skip_n_types<N, T...>>;
    
    template<int N, int... T>
    using SkipValues = Invoke<impl::skip_n_values<N, T...>>;
    
    namespace impl {
        
        template<typename T>
        struct result_of {
            using type = Invoke<T>;
        };
        
        template<typename TRet, typename... TArg>
        struct result_of<TRet(*)(TArg...) > {
            using type = TRet;
        };

        template<typename TClass, typename TRet, typename... TArg>
        struct result_of<TRet(TClass::*)(TArg...)> {
            using type = TRet;
        };

        template<typename Fn, typename... Arg>
        struct result_of<Fn(Arg...)> {
            using type = Invoke<std::result_of<Fn(Arg...)>>;
        };
    }
    
    template<typename T>
    using ResultOf = Invoke<impl::result_of<T>>;
    
    template<typename, template<typename...> class>
    struct is_specialization_of;
    template<typename TType, template<typename...> class TTemplate>
    struct is_specialization_of : Bool<true> { };
    template<template<typename...> class TTemplate, typename... TArgs>
    struct is_specialization_of<TTemplate<TArgs...>, TTemplate> : Bool<false> { };
    
    namespace impl {
        
        template<typename Head, typename... Tail>
        struct min : Head { };
        template<typename T, typename U, typename... Tail>
        struct min<T, U, Tail...> : min<Conditional<Bool<(T::value < U::value)>, T, U>, Tail...> { };

        template<typename Head, typename... Tail>
        struct max : Head { };
        template<typename T, typename U, typename... Tail>
        struct max<T, U, Tail...> : max<Conditional<Bool<(T::value > U::value)>, T, U>, Tail...> { };

        template<typename... T>
        struct count : Int<0> { };
        template<typename Head, typename... Tail>
        struct count<Head, Tail...> : Int<(!!Head::value) + count<Tail...>::value> { };
    }
    
    template<typename... T>
    struct min : impl::min<T...> { };
    
    template<typename... T>
    struct max : impl::max<T...> { };
    
    template<typename... T>
    struct count : impl::count<T...> { };
    
    namespace detail {
        enum class enabler { _ };
    }
    
    constexpr auto _ = detail::enabler::_;
    
    template<typename... TCond>
    using EnableIf = Invoke<std::enable_if<All<TCond...>::value, decltype(_)>>;
    
    template<typename... TCond>
    using DisableIf = Invoke<std::enable_if<Not<All<TCond...>>::value, decltype(_)>>;
    
    namespace impl {
        
        template<typename...>
        struct is_same : Bool<false> { };
        
        template<typename T, typename U, typename... Tail>
        struct is_same<T, U, Tail...> : Conditional<std::is_same<T, U>, is_same<U, Tail...>, Bool<false>> { };
    }
    
    template<typename... T>
    struct is_same : impl::is_same<T...> { };
    
    template<int Idx>
    struct indexed {
        using value_type = int;
        constexpr static int index = Idx;

        constexpr operator value_type() const {
            return index;
        }
    };
}

#endif