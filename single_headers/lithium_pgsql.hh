// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium_pgsql library.
// https://github.com/matt-42/lithium
//
// This file is generated do not edit it.

#pragma once

#include <any>
#include <arpa/inet.h>
#include <atomic>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <iostream>
#include <libpq-fe.h>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_DATABASE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_DATABASE_HH


#include "libpq-fe.h"

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH

namespace li {

template <typename... T> struct typelist {};

namespace internal {
template <typename T> struct has_parenthesis_operator {
  template <typename C> static char test(decltype(&C::operator()));
  template <typename C> static int test(...);
  static const bool value = sizeof(test<T>(0)) == 1;
};

} // namespace internal

// Traits on callable (function, functors and lambda functions).

// callable_traits<F>::is_callable = true_type if F is callable.
// callable_traits<F>::arity = N if F takes N arguments.
// callable_traits<F>::arguments_tuple_type = tuple<Arg1, ..., ArgN>

template <typename F, typename X = void> struct callable_traits {
  typedef std::false_type is_callable;
  static const int arity = 0;
  typedef std::tuple<> arguments_tuple;
  typedef typelist<> arguments_list;
  typedef void return_type;
};

template <typename F, typename X> struct callable_traits<F&, X> : public callable_traits<F, X> {};
template <typename F, typename X> struct callable_traits<F&&, X> : public callable_traits<F, X> {};
template <typename F, typename X>
struct callable_traits<const F&, X> : public callable_traits<F, X> {};

template <typename F>
struct callable_traits<F, std::enable_if_t<internal::has_parenthesis_operator<F>::value>> {
  typedef callable_traits<decltype(&F::operator())> super;
  typedef std::true_type is_callable;
  static const int arity = super::arity;
  typedef typename super::arguments_tuple arguments_tuple;
  typedef typename super::arguments_list arguments_list;
  typedef typename super::return_type return_type;
};

template <typename C, typename R, typename... ARGS>
struct callable_traits<R (C::*)(ARGS...) const> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename C, typename R, typename... ARGS> struct callable_traits<R (C::*)(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename R, typename... ARGS> struct callable_traits<R(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename R, typename... ARGS> struct callable_traits<R (*)(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename F>
using callable_arguments_tuple_t = typename callable_traits<F>::arguments_tuple;
template <typename F> using callable_arguments_list_t = typename callable_traits<F>::arguments_list;
template <typename F> using callable_return_type_t = typename callable_traits<F>::return_type;

template <typename F> struct is_callable : public callable_traits<F>::is_callable {};

template <typename F, typename... A> struct callable_with {
  template <typename G, typename... B>
  static char test(int x,
                   std::remove_reference_t<decltype(std::declval<G>()(std::declval<B>()...))>* = 0);
  template <typename G, typename... B> static int test(...);
  static const bool value = sizeof(test<F, A...>(0)) == 1;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH


namespace li {

namespace internal {
struct {
  template <typename A, typename... B> constexpr auto operator()(A&& a, B&&... b) {
    auto result = a;
    using expand_variadic_pack = int[];
    (void)expand_variadic_pack{0, ((result += b), 0)...};
    return result;
  }
} reduce_add;

} // namespace internal

template <typename... Ms> struct metamap;

template <typename F, typename... M> decltype(auto) find_first(metamap<M...>&& map, F fun);

template <typename... Ms> struct metamap;

template <typename M1, typename... Ms> struct metamap<M1, Ms...> : public M1, public Ms... {
  typedef metamap<M1, Ms...> self;
  // Constructors.
  inline metamap() = default;
  inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  self& operator=(const self&) = default;
  self& operator=(self&&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  inline metamap(typename M1::_iod_value_type&& m1, typename Ms::_iod_value_type&&... members) : M1{m1}, Ms{std::forward<typename Ms::_iod_value_type>(members)}... {}
  inline metamap(M1&& m1, Ms&&... members) : M1(m1), Ms(std::forward<Ms>(members))... {}
  inline metamap(const M1& m1, const Ms&... members) : M1(m1), Ms((members))... {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <> struct metamap<> {
  typedef metamap<> self;
  // Constructors.
  inline metamap() = default;
  // inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  // self& operator=(const self&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <typename... Ms> constexpr auto size(metamap<Ms...>) { return sizeof...(Ms); }

template <typename M> struct metamap_size_t {};
template <typename... Ms> struct metamap_size_t<metamap<Ms...>> {
  enum { value = sizeof...(Ms) };
};
template <typename M> constexpr int metamap_size() {
  return metamap_size_t<std::decay_t<M>>::value;
}

template <typename... Ks> decltype(auto) metamap_values(const metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
}
template <typename... Ks> decltype(auto) metamap_values(metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
}

template <typename... Ks> decltype(auto) metamap_keys(const metamap<Ks...>& map) {
  return std::make_tuple(typename Ks::_iod_symbol_type()...);
}

template <typename K, typename M> constexpr auto has_key(M&& map, K k) {
  return decltype(has_member(map, k)){};
}

template <typename M, typename K> constexpr auto has_key(K k) {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename M, typename K> constexpr auto has_key() {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename K, typename M, typename O> constexpr auto get_or(M&& map, K k, O default_) {
  if constexpr (has_key<M, decltype(k)>()) {
    return map[k];
  } else
    return default_;
}

template <typename X> struct is_metamap {
  enum { value = false };
};
template <typename... M> struct is_metamap<metamap<M...>> {
  enum { value = true };
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH

namespace li {

template <typename... T, typename... U>
inline decltype(auto) cat(const metamap<T...>& a, const metamap<U...>& b) {
  return metamap<T..., U...>(*static_cast<const T*>(&a)..., *static_cast<const U*>(&b)...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH


namespace li {

template <typename E> struct Exp {};

template <typename E> struct array_subscriptable;

template <typename E> struct callable;

template <typename E> struct assignable;

template <typename E> struct array_subscriptable;

template <typename M, typename... A>
struct function_call_exp : public array_subscriptable<function_call_exp<M, A...>>,
                           public callable<function_call_exp<M, A...>>,
                           public assignable<function_call_exp<M, A...>>,
                           public Exp<function_call_exp<M, A...>> {
  using assignable<function_call_exp<M, A...>>::operator=;

  function_call_exp(const M& m, A&&... a) : method(m), args(std::forward<A>(a)...) {}

  M method;
  std::tuple<A...> args;
};

template <typename O, typename M>
struct array_subscript_exp : public array_subscriptable<array_subscript_exp<O, M>>,
                             public callable<array_subscript_exp<O, M>>,
                             public assignable<array_subscript_exp<O, M>>,
                             public Exp<array_subscript_exp<O, M>> {
  using assignable<array_subscript_exp<O, M>>::operator=;

  array_subscript_exp(const O& o, const M& m) : object(o), member(m) {}

  O object;
  M member;
};

template <typename L, typename R> struct assign_exp : public Exp<assign_exp<L, R>> {
  typedef L left_t;
  typedef R right_t;

  // template <typename V>
  // assign_exp(L l, V&& r) : left(l), right(std::forward<V>(r)) {}
  // template <typename V>
  inline assign_exp(L l, R r) : left(l), right(r) {}
  // template <typename V>
  // inline assign_exp(L l, const V& r) : left(l), right(r) {}

  L left;
  R right;
};

template <typename E> struct array_subscriptable {
public:
  // Member accessor
  template <typename S> constexpr auto operator[](S&& s) const {
    return array_subscript_exp<E, S>(*static_cast<const E*>(this), std::forward<S>(s));
  }
};

template <typename E> struct callable {
public:
  // Direct call.
  template <typename... A> constexpr auto operator()(A&&... args) const {
    return function_call_exp<E, A...>(*static_cast<const E*>(this), std::forward<A>(args)...);
  }
};

template <typename E> struct assignable {
public:
  template <typename L> auto operator=(L&& l) const {
    return assign_exp<E, L>(static_cast<const E&>(*this), std::forward<L>(l));
  }

  template <typename L> auto operator=(L&& l) {
    return assign_exp<E, L>(static_cast<E&>(*this), std::forward<L>(l));
  }

  template <typename T> auto operator=(const std::initializer_list<T>& l) const {
    return assign_exp<E, std::vector<T>>(static_cast<const E&>(*this), std::vector<T>(l));
  }
};

#define iod_query_declare_binary_op(OP, NAME)                                                      \
  template <typename A, typename B>                                                                \
  struct NAME##_exp : public assignable<NAME##_exp<A, B>>, public Exp<NAME##_exp<A, B>> {          \
    using assignable<NAME##_exp<A, B>>::operator=;                                                 \
    NAME##_exp() {}                                                                                \
    NAME##_exp(A&& a, B&& b) : lhs(std::forward<A>(a)), rhs(std::forward<B>(b)) {}                 \
    typedef A lhs_type;                                                                            \
    typedef B rhs_type;                                                                            \
    lhs_type lhs;                                                                                  \
    rhs_type rhs;                                                                                  \
  };                                                                                               \
  template <typename A, typename B>                                                                \
  inline std::enable_if_t<std::is_base_of<Exp<A>, A>::value || std::is_base_of<Exp<B>, B>::value,  \
                          NAME##_exp<A, B>>                                                        \
  operator OP(const A& b, const B& a) {                                                            \
    return NAME##_exp<std::decay_t<A>, std::decay_t<B>>{b, a};                                     \
  }

iod_query_declare_binary_op(+, plus);
iod_query_declare_binary_op(-, minus);
iod_query_declare_binary_op(*, mult);
iod_query_declare_binary_op(/, div);
iod_query_declare_binary_op(<<, shiftl);
iod_query_declare_binary_op(>>, shiftr);
iod_query_declare_binary_op(<, inf);
iod_query_declare_binary_op(<=, inf_eq);
iod_query_declare_binary_op(>, sup);
iod_query_declare_binary_op(>=, sup_eq);
iod_query_declare_binary_op(==, eq);
iod_query_declare_binary_op(!=, neq);
iod_query_declare_binary_op(&, logical_and);
iod_query_declare_binary_op (^, logical_xor);
iod_query_declare_binary_op(|, logical_or);
iod_query_declare_binary_op(&&, and);
iod_query_declare_binary_op(||, or);

#undef iod_query_declare_binary_op

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH


namespace li {

template <typename S>
class symbol : public assignable<S>,
               public array_subscriptable<S>,
               public callable<S>,
               public Exp<S> {};
} // namespace li

#ifdef LI_SYMBOL
#undef LI_SYMBOL
#endif

#define LI_SYMBOL(NAME)                                                                            \
  namespace s {                                                                                    \
  struct NAME##_t : li::symbol<NAME##_t> {                                                         \
                                                                                                   \
    using assignable<NAME##_t>::operator=;                                                         \
                                                                                                   \
    inline constexpr bool operator==(NAME##_t) { return true; }                                    \
    template <typename T> inline constexpr bool operator==(T) { return false; }                    \
                                                                                                   \
    template <typename V> struct variable_t {                                                      \
      typedef NAME##_t _iod_symbol_type;                                                           \
      typedef V _iod_value_type;                                                                   \
      V NAME;                                                                                      \
    };                                                                                             \
                                                                                                   \
    template <typename T, typename... A>                                                           \
    static inline decltype(auto) symbol_method_call(T&& o, A... args) {                            \
      return o.NAME(args...);                                                                      \
    }                                                                                              \
    template <typename T, typename... A> static inline auto& symbol_member_access(T&& o) {         \
      return o.NAME;                                                                               \
    }                                                                                              \
    template <typename T>                                                                          \
    static constexpr auto has_getter(int)                                                          \
        -> decltype(std::declval<T>().NAME(), std::true_type{}) {                                  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_getter(long) { return std::false_type{}; }     \
    template <typename T>                                                                          \
    static constexpr auto has_member(int) -> decltype(std::declval<T>().NAME, std::true_type{}) {  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_member(long) { return std::false_type{}; }     \
                                                                                                   \
    static inline auto symbol_string() { return #NAME; }                                           \
    static inline auto json_key_string() { return "\"" #NAME "\":"; }                              \
  };                                                                                               \
  static constexpr NAME##_t NAME;                                                                  \
  }

namespace li {

template <typename S> inline decltype(auto) make_variable(S s, char const v[]) {
  typedef typename S::template variable_t<const char*> ret;
  return ret{v};
}

template <typename V, typename S> inline decltype(auto) make_variable(S s, V v) {
  typedef typename S::template variable_t<std::remove_const_t<std::remove_reference_t<V>>> ret;
  return ret{v};
}

template <typename K, typename V> inline decltype(auto) make_variable_reference(K s, V&& v) {
  typedef typename K::template variable_t<V> ret;
  return ret{v};
}

template <typename T, typename S, typename... A>
static inline decltype(auto) symbol_method_call(T&& o, S, A... args) {
  return S::symbol_method_call(o, std::forward<A>(args)...);
}

template <typename T, typename S> static inline decltype(auto) symbol_member_access(T&& o, S) {
  return S::symbol_member_access(o);
}

template <typename T, typename S> constexpr auto has_member(T&& o, S) {
  return S::template has_member<T>(0);
}
template <typename T, typename S> constexpr auto has_member() {
  return S::template has_member<T>(0);
}

template <typename T, typename S> constexpr auto has_getter(T&& o, S) {
  return decltype(S::template has_getter<T>(0)){};
}
template <typename T, typename S> constexpr auto has_getter() {
  return decltype(S::template has_getter<T>(0)){};
}

template <typename S, typename T> struct CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE {};

template <typename T, typename S> decltype(auto) symbol_member_or_getter_access(T&& o, S) {
  if constexpr (has_getter<T, S>()) {
    return symbol_method_call(o, S{});
  } else if constexpr (has_member<T, S>()) {
    return symbol_member_access(o, S{});
  } else {
    return CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE<S, T>::error;
  }
}

template <typename S> auto symbol_string(symbol<S> v) { return S::symbol_string(); }

template <typename V> auto symbol_string(V v, typename V::_iod_symbol_type* = 0) {
  return V::_iod_symbol_type::symbol_string();
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH


namespace li {

template <typename... Ms> struct metamap;

namespace internal {

template <typename S, typename V> decltype(auto) exp_to_variable_ref(const assign_exp<S, V>& e) {
  return make_variable_reference(S{}, e.right);
}

template <typename S, typename V> decltype(auto) exp_to_variable(const assign_exp<S, V>& e) {
  typedef std::remove_const_t<std::remove_reference_t<V>> vtype;
  return make_variable(S{}, e.right);
}

template <typename S> decltype(auto) exp_to_variable(const symbol<S>& e) {
  return exp_to_variable(S() = int());
}

template <typename... T> inline decltype(auto) make_metamap_helper(T&&... args) {
  return metamap<T...>(std::forward<T>(args)...);
}

} // namespace internal

// Store copies of values in the map
static struct {
  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return internal::make_metamap_helper(internal::exp_to_variable(std::forward<T>(args))...);
  }
} mmm;

// Store references of values in the map
template <typename... T> inline decltype(auto) make_metamap_reference(T&&... args) {
  // Keep references.
  return internal::make_metamap_helper(internal::exp_to_variable_ref(std::forward<T>(args))...);
}

template <typename... Ks> decltype(auto) metamap_clone(const metamap<Ks...>& map) {
  return mmm((typename Ks::_iod_symbol_type() = map[typename Ks::_iod_symbol_type()])...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH


namespace li {

struct skip {};
static struct {

  template <typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, skip, T&&... args) const {
    return run(map, std::forward<T>(args)...);
  }

  template <typename T1, typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, T1&& a, T&&... args) const {
    return run(
        cat(map, internal::make_metamap_helper(internal::exp_to_variable(std::forward<T1>(a)))),
        std::forward<T>(args)...);
  }

  template <typename... M> inline decltype(auto) run(metamap<M...> map) const { return map; }

  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return run(metamap<>{}, std::forward<T>(args)...);
  }

} make_metamap_skip;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


namespace li {

template <typename... E, typename F> void apply_each(F&& f, E&&... e) {
  (void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
}

template <typename... E, typename F, typename R>
auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) {
  return reduce(f(std::forward<E>(e))...);
}

template <typename T, typename F> void tuple_map(T&& t, F&& f) {
  return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
                    std::forward<T>(t));
}

template <typename T, typename F> auto tuple_reduce(T&& t, F&& f) {
  return std::apply(std::forward<F>(f), std::forward<T>(t));
}

template <typename T, typename F, typename R>
decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
  auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); };
  return std::apply(fun, m);
}

template <typename F> inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

template <typename F, typename... M, typename M1> auto tuple_filter_impl(M1 m1, M... m) {
  if constexpr (std::is_same<M1, F>::value)
    return tuple_filter_impl<F>(m...);
  else
    return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
}

template <typename F, typename... M> auto tuple_filter(const std::tuple<M...>& m) {

  auto fun = [](auto... e) { return tuple_filter_impl<F>(e...); };
  return std::apply(fun, m);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


namespace li {

// Map a function(key, value) on all kv pair
template <typename... M, typename F> void map(const metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

// Map a function(key, value) on all kv pair. Ensure that the calling order
// is kept.
// template <typename O, typename F>
// void map_sequential2(F fun, O& obj)
// {}
// template <typename O, typename M1, typename... M, typename F>
// void map_sequential2(F fun, O& obj, M1 m1, M... ms)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, obj[key]);
//     };

//   apply(m1);
//   map_sequential2(fun, obj, ms...);
// }
// template <typename... M, typename F>
// void map_sequential(const metamap<M...>& m, F fun)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, m[key]);
//     };

//   map_sequential2(fun, m, typename M::_iod_symbol_type{}...);
// }

// Map a function(key, value) on all kv pair (non const).
template <typename... M, typename F> void map(metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

template <typename... E, typename F, typename R> auto apply_each2(F&& f, R&& r, E&&... e) {
  return r(f(std::forward<E>(e))...);
  //(void)std::initializer_list<int>{
  //  ((void)f(std::forward<E>(e)), 0)...};
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename F, typename R>
decltype(auto) map_reduce(const metamap<M...>& m, F map, R reduce) {
  auto apply = [&](auto key) -> decltype(auto) {
    // return map(key, std::forward<decltype(m[key])>(m[key]));
    return map(key, m[key]);
  };

  return apply_each2(apply, reduce, typename M::_iod_symbol_type{}...);
  // return reduce(apply(typename M::_iod_symbol_type{})...);
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename R> decltype(auto) reduce(const metamap<M...>& m, R reduce) {
  return reduce(m[typename M::_iod_symbol_type{}]...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH



namespace li {

template <typename... T, typename... U>
inline decltype(auto) intersection(const metamap<T...>& a, const metamap<U...>& b) {
  return map_reduce(a,
                    [&](auto k, auto&& v) -> decltype(auto) {
                      if constexpr (has_key<metamap<U...>, decltype(k)>()) {
                        return k = std::forward<decltype(v)>(v);
                      } else
                        return skip{};
                    },
                    make_metamap_skip);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH


namespace li {

template <typename... T, typename... U>
inline auto substract(const metamap<T...>& a, const metamap<U...>& b) {
  return map_reduce(a,
                    [&](auto k, auto&& v) {
                      if constexpr (!has_key<metamap<U...>, decltype(k)>()) {
                        return k = std::forward<decltype(v)>(v);
                      } else
                        return skip{};
                    },
                    make_metamap_skip);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH


namespace li {

namespace internal {

template <typename... S, typename... V, typename T, T... I>
auto forward_tuple_as_metamap_impl(std::tuple<S...> keys, std::tuple<V...>& values, std::integer_sequence<T, I...>) {
  return make_metamap_reference((std::get<I>(keys) = std::get<I>(values))...);
}
template <typename... S, typename... V, typename T, T... I>
auto forward_tuple_as_metamap_impl(std::tuple<S...> keys, const std::tuple<V...>& values, std::integer_sequence<T, I...>) {
  return make_metamap_reference((std::get<I>(keys) = std::get<I>(values))...);
}

} // namespace internal

template <typename... S, typename... V>
auto forward_tuple_as_metamap(std::tuple<S...> keys, std::tuple<V...>& values) {
  return internal::forward_tuple_as_metamap_impl(keys, values, std::make_index_sequence<sizeof...(V)>{});
}
template <typename... S, typename... V>
auto forward_tuple_as_metamap(std::tuple<S...> keys, const std::tuple<V...>& values) {
  return internal::forward_tuple_as_metamap_impl(keys, values, std::make_index_sequence<sizeof...(V)>{});
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_HH


#include "libpq-fe.h"

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH


namespace li {
struct sql_blob : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_blob() : std::string() {}
};

struct sql_null_t {};
static sql_null_t null;

template <unsigned SIZE> struct sql_varchar : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_varchar() : std::string() {}
};
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_DATA_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_DATA_HH



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH


namespace li {

template <typename V>
struct type_hashmap {

  template <typename E, typename F> E& get_cache_entry(int& hash, F)
  {
    // Init hash if needed.
    if (hash == -1)
    {
      std::lock_guard lock(mutex_);
      if (hash == -1)
        hash = counter_++;
    }
    // Init cache if miss.
    if (hash >= values_.size() or !values_[hash].has_value())
    {
      if (values_.size() < hash + 1)
        values_.resize(hash+1);
      values_[hash] = E();
    }

    // Return existing cache entry.
    return std::any_cast<E&>(values_[hash]);
  }
  template <typename K, typename F> V& operator()(F f, K key)
  {
    static int hash = -1;
    return this->template get_cache_entry<std::unordered_map<K, V>>(hash, f)[key];
  }

  template <typename F> V& operator()(F f)
  {
    static int hash = -1;
    return this->template get_cache_entry<V>(hash, f);
  }

private:
  static std::mutex mutex_;
  static int counter_;
  std::vector<std::any> values_;
};

template <typename V>
std::mutex type_hashmap<V>::mutex_;
template <typename V>
int type_hashmap<V>::counter_ = 0;

}

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH


namespace li {

inline void println() { 
  std::cout << std::endl; 
}

template <typename A, typename... T> inline void println(A &&a, T &&... args) {
  std::cout << a << " ";
  println(std::forward<T>(args)...);
}

template <typename T> struct is_tuple_after_decay : std::false_type {};
template <typename... T> struct is_tuple_after_decay<std::tuple<T...>> : std::true_type {};

template <typename T> struct is_tuple : is_tuple_after_decay<std::decay_t<T>> {};
template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH


namespace li
{

struct pgsql_statement_data;
struct pgsql_connection_data {

  ~pgsql_connection_data() {
    if (pgconn_) {
      cancel();
      PQfinish(pgconn_);
    }
  }
  void cancel() {
    if (pgconn_) {
      // Cancel any pending request.
      PGcancel* cancel = PQgetCancel(pgconn_);
      char x[256];
      if (cancel) {
        PQcancel(cancel, x, 256);
        PQfreeCancel(cancel);
      }
    }
  }
  template <typename Y>
  void flush(Y& fiber) {
    while(int ret = PQflush(pgconn_))
    {
      if (ret == -1)
      {
        std::cerr << "PQflush error" << std::endl;
      }
      if (ret == 1)
        fiber.yield();
    }
  }

  // template <typename Y>
  void flush_current_query_result() {
    // println("flush_current_query_result ", current_result_id_);
    // ignore_current_query_result_ = true;
    // connection_->end_of_current_result(this->fiber_, false);
    // if (last_batch_end_id_ < current_result_id_)
    //   this->send_end_batch();
    while (true)
    {
      // if (PQconsumeInput(pgconn_) == 0)
      // {
      //   error_ = 1;
      //   break;
      // }
      // if (!PQisBusy(pgconn_))
      {
        // println("PQgetResult try flush");
        PGresult* res = PQgetResult(pgconn_);
        // println("PQgetResult flushed", res);
        // std::cout << 
        if (res) PQclear(res);

        // Break if:
        //   res is null means end of the query result.
        //   status is PGRES_PIPELINE_SYNC becaused it is not followed by a null PGresult.
        if (!res || PQresultStatus(res) == PGRES_PIPELINE_SYNC) 
          break;
      }
      // usleep(10);
    }
    // println("flush_current_query_result ", current_result_id_, " done  ");
    result_processing_in_progress_ = false;
  }

  void ignore_next_query_of_fiber(int fiber_id) {
    for (auto& q : batched_queries)
      if (q.fiber_id == fiber_id)
        q.ignore_result = true;
  }
  void ignore_result(int result_id) {
    for (int i = batched_queries.size() - 1; i >= 0; i--)
    // for (int i = 0; i < batched_queries.size(); i++)
      if (batched_queries[i].result_id == result_id)
      {
        batched_queries[i].ignore_result = true;
        return;
      }

    // println("ignore_result error: result does not exists: ", result_id);
    // assert(0);

  }

  template <typename Y>
  PGresult* wait_for_next_pgresult(Y& fiber, bool nothrow = false, bool async = true) {
    // println("WAIT for next PGresult    ====================== current result id is ", this->current_result_id_);
    while (true) {
      // println("PQconsumeInput");
      if (PQconsumeInput(pgconn_) == 0)
      {
        error_ = 1;          
        if (!nothrow)
          throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
                                  PQerrorMessage(pgconn_));
        else
          std::cerr << "PQconsumeInput() failed: " << PQerrorMessage(pgconn_) << std::endl;
        assert(0);
      }

      if (PQisBusy(pgconn_)) {
        // std::cout << "isbusy" << std::endl;
        try {
          // In async mode, make sure the pq socket wakes up this fiber.
          if (async) {
            // println("         is busy => yield.  ======================");
            fiber.reassign_fd_to_fiber(PQsocket(pgconn_), fiber.continuation_idx);
            fiber.yield();
          }
        } catch (typename Y::exception_type& e) {
          // println("   wait for next PGresult ERROR WITH fiber", fiber.continuation_idx);

          // Free results.
          // Yield thrown a exception (probably because a closed connection).
          // Ignore batched result of this fiber.
          //this->ignore_next_query_of_fiber(fiber.continuation_idx);

          // Flush the remaining result of this query.
          this->flush_current_query_result();
          current_result_id_ = -1;
          // // Go to the next result.
          // end_of_current_result(fiber);

          throw std::move(e);
        }
      } else {
        // std::cout << "notbusy" << std::endl;
        // println("PQgetResult");
        PGresult* res = PQgetResult(pgconn_);
        auto status = PQresultStatus(res);
        if (status == PGRES_FATAL_ERROR and PQerrorMessage(pgconn_)[0] != 0)
        {
          PQclear(res);
          error_ = 1;          
          if (!nothrow)
            throw std::runtime_error(std::string("Postresql fatal error:") +
                                    PQerrorMessage(pgconn_));
          else
            std::cerr << "Postgresql FATAL error: " << PQerrorMessage(pgconn_) << std::endl;

          assert(0);
        }
        else if (status == PGRES_NONFATAL_ERROR)
        {
          PQclear(res);
          std::cerr << "Postgresql non fatal error: " << PQerrorMessage(pgconn_) << std::endl;
        }        
        else if (status == PGRES_PIPELINE_ABORTED) {
          PQclear(res);
          std::cerr << "Postgresql batch aborted: " << PQerrorMessage(pgconn_) << std::endl;        
        }

        // if (res == nullptr)
        //   println("   PQgetResult returned NULL");

        // if (status == PGRES_PIPELINE_SYNC)
        //   println("   PQgetResult returned PGRES_PIPELINE_SYNC");
        // println("    wait for next result OK.");
        return res;
      }
    }
  }
  struct batch_query_info {int fiber_id; int result_id; bool ignore_result; bool is_batch_end=false; };

  void send_end_batch() {
    // if (batched_queries.size() && batched_queries.back().result_id == last_batch_end_id_) return;

    // println("send_end_batch: batch size: ", batched_queries.back().result_id - last_batch_end_id_);
    // println("PQsendEndBatch"); 
    if (0 == PQpipelineSync(this->pgconn_))
      std::cerr << "PQpipelineSync error"  << std::endl; 
    int result_id = this->next_result_id++;// batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
    last_batch_end_id_ = result_id;
    batched_queries.push_back(batch_query_info{0, result_id, true, true});
  }
  // Go to next batched query result
  template <typename Y>
  int batch_query(Y& fiber, bool ignore_result = false) {

    int result_id = next_result_id++;//batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
    // println("batch query ", result_id, " ignore: ", ignore_result, "queue size", batched_queries.size());
    batched_queries.push_back(batch_query_info{fiber.continuation_idx, result_id, ignore_result});

    // if (batched_queries.back().result_id - last_batch_end_id_ > 15000)
    //   this->send_end_batch();
    // if (!end_batch_defered)
    // {
    //   end_batch_defered = true;
    //   // std::cout << "defer endbatch" << std::endl;
    //   fiber.defer([this] { 
    //     end_batch_defered = false;
    //     this->send_end_batch();
    //     });
    // }
    // while (batched_queries.size() > 15000)
    //   fiber.yield();
    return result_id;
  }

  // Get the next query for reading the results.
  // call send_end_batch if the query is in the batch currently
  // being accumulated.
  inline auto pq_get_next_query() {

    auto query = batched_queries.front();
    batched_queries.pop_front();
    current_result_id_ = query.result_id;

    // current_result_id_ = result_id;
    if (current_result_id_ > last_batch_end_id_) this->send_end_batch();
    
    // println("pq_get_next_query");
    // if (0 == PQbatchProcessQueue(pgconn_))
    // {
    //   std::cerr << "PQgetNextQuery error : " <<  PQerrorMessage(pgconn_) << std::endl;
    //   assert(0);
    //   throw std::runtime_error(std::string("PQgetNextQuery error : ") + PQerrorMessage(pgconn_));
    // }
    return query;
  }

  template <typename Y>
  void flush_ignored_results(Y& fiber, bool async = true) {

    // println("flush_ignored_results");

    assert(!result_processing_in_progress_);
    
    result_processing_in_progress_ = true;
    // println(" flush_ignored_results: result_processing_in_progress_ = true");

    // if (batched_queries.size() > 0 && batched_queries.front().ignore_result)
    //   this->send_end_batch();

    while (batched_queries.size() > 0 && batched_queries.front().ignore_result) {


      // std::cout << "PQgetNextQuery" << std::endl; 
      auto query = this->pq_get_next_query();
      // println(" ignore ", query.result_id);
      assert(query.ignore_result);

      // println("ignore result ", query.result_id);
      while (true)
      {
        // println("wait one result");
        PGresult* res = this->wait_for_next_pgresult(fiber, false, async);
        // println("one result ok");
        if (res)
        {
          // if (query.is_batch_end) println("got PGRES_BATCH_END");
          int status = PQresultStatus(res);
          if (query.is_batch_end && status != PGRES_PIPELINE_SYNC)
          {
            std::cerr << " got batch end but PQresultStatus(res) == " << status << ". It should be PGRES_PIPELINE_SYNC: " << PGRES_PIPELINE_SYNC << std::endl;
          }
          assert(!query.is_batch_end || status == PGRES_PIPELINE_SYNC);
          PQclear(res);
          if (query.is_batch_end)
            break; // PGRES_PIPELINE_SYNC is not followed by a null PGresult.
        }
        else break;
      }
      // println(" ignore ", query.result_id, " OK.");

    }
    // println("end flush_ignored_results");
    if (batched_queries.size() == 0)
    {
      // std::cout << "current_result_id_ = -1" << std::endl; 
      current_result_id_ = -1;        
    }

    // println("after flush ignore result, queue size == ", batched_queries.size());

    result_processing_in_progress_ = false;
  }

  // The current result is totally consumed. Go to the next one.
  // If another fiber is waiting for the next one, defer the wake up of this fiber. 
  template <typename Y>
  void end_of_current_result(Y& fiber, bool async = true) {
    // println("end_of_current_result: ", this->current_result_id_);
    // assert(result_processing_in_progress_);
    result_processing_in_progress_ = false;

    // Ignored result.
    flush_ignored_results(fiber, async);

    if (batched_queries.size() > 0)
    {
      auto query = this->pq_get_next_query();
      assert(!query.ignore_result);
      // std::cout << "current_result_id_ = " << current_result_id_ << std::endl; 

      // Wake up the fiber waiting for this query if not the calling one.
      if (query.fiber_id != fiber.continuation_idx)
      {
        // std::cout << " Next fiber to read result is " << query.fiber_id << " with result id " << this->current_result_id_ << std::endl;
        // fiber.reassign_fd_to_fiber(PQsocket(pgconn_), query.fiber_id);
        fiber.defer_fiber_resume(query.fiber_id);
      }
    }
    else
    {
      // println("   end_of_current_result: queue is now empty");
      // std::cout << "current_result_id_ = -1" << std::endl; 
      current_result_id_ = -1;        
    }

    // Ready for processing next result.
    // println(" end of result OK. current_result_id_: ", current_result_id_);

    result_processing_in_progress_ = false;

  }

  template <typename Y>
  void wait_for_result(Y& fiber, int result_id) {
    // println("wait for result", result_id, " current is: ", current_result_id_);
    if (current_result_id_ == result_id) return;

        
    if (current_result_id_ == -1)
    {
      // this->flush_ignored_results(fiber);
      // this->pg_get_next_query();
      this->end_of_current_result(fiber);
    }
    while (current_result_id_ != result_id)
    {
      // try {
      fiber.yield();      
      // } catch 
    }

    // Send endbatch if the requested result is in the current (not ended) batch.
    // if (result_id > last_batch_end_id_) this->send_end_batch();

    // println("wait_for_result ", result_id, " ok: current_result_id_: ", current_result_id_);
    // assert(!result_processing_in_progress_);
    // Flush any ignored result.
    // flush_ignored_results(fiber);

    // println(" PQ GET NEXT QUERY ", current_result_id_);
    // this->pq_get_next_query();
    // println("wait for result done.");

    assert(!result_processing_in_progress_);
    // println(" wait for result: result_processing_in_progress_ = true");
    result_processing_in_progress_ = true;
  }

  PGconn* pgconn_ = nullptr;
  int fd = -1;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
  type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
  std::deque<batch_query_info> batched_queries;
  int current_result_id_ = -1;
  int error_ = 0;
  int next_result_id = 1;
  bool end_batch_defered = false;
  bool result_processing_in_progress_ = false;
  int last_batch_end_id_ = -1;
};

}

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_DATA_HH


namespace li {


struct refcount {

  refcount() : count_(nullptr) {}
  refcount(const refcount& o) : count_(nullptr) {
    assign(o);
  }
  const refcount& operator=(const refcount& o){
    assign(o);
    return *this;
  }

  ~refcount() {
    if (!count_) return;

    if (*count_ == 1) delete count_; 
    else (*count_)--;
    count_ = nullptr;
  }

  void assign(const refcount& o) {
    if (count_) (*count_)--;

    refcount& o2 = *const_cast<refcount*>(&o);
    if (!o2.count_)
    {
      o2.count_ = new int(1);
    }
    count_ = o2.count_;
    (*count_)++;
  }

  int count() { return count_ ? *count_ : 1; }

  int* count_ = nullptr;
};

template <typename Y> struct pgsql_result {

public:
  pgsql_result(std::shared_ptr<pgsql_connection_data> connection_,
  Y& fiber_,
  int result_id_) : connection_(connection_), fiber_(fiber_), result_id_(result_id_) {
      // std::cout << " build result " << result_id_ << std::endl;

  }
  ~pgsql_result() {  
    if (this->refcount_.count() == 1) cleanup();
  }

  void cleanup() { 

      //delete p;
      // std::cout << " destroy result " << std::endl;
      // std::cout << " destroy result " << result_id_ << " this: " << this->result_id_ << std::endl;
      if (this->end_of_result_) {
        // println("destroy totally consumed result. ", this->result_id_);
        return;
      }

      // assert(this->current_result_);
      else if (connection_->current_result_id_ == this->result_id_)
      {
        // println("destroy result being read:", this->result_id_);
        // println(this->current_result_);
        // connection_->ignore_current_query_result();
        connection_->flush_current_query_result();
        connection_->end_of_current_result(this->fiber_, false);
      }
      else {
        // println("will ignore future result because it is destroyed:", this->result_id_);
        connection_->ignore_result(this->result_id_);
      }

      if (this->current_result_) { PQclear(this->current_result_); this->current_result_ = nullptr; } 
    
  }

  // Read metamap and tuples.
  template <typename T> bool read(T&& t1);
  long long int last_insert_id();
  
  // Flush all results.
  void flush_results();

  // Fetch next result, used internally by read.
  template <typename T> bool fetch_next_result(T&& output);

  std::shared_ptr<pgsql_connection_data> connection_;
  Y& fiber_;
  int result_id_;

  bool end_of_result_ = false;
  int last_insert_id_ = -1;
  int row_i_ = 0;
  int current_result_nrows_ = 0;
  PGresult* current_result_ = nullptr;
  std::vector<Oid> curent_result_field_types_;
  std::vector<int> curent_result_field_positions_;

  refcount refcount_;

private:

  // Wait for the next result.
  PGresult* wait_for_next_result();

  // Fetch a string from a result field.
  template <typename... A>
  void fetch_value(std::string& out, int field_i, Oid field_type);
  // Fetch a blob from a result field.
  template <typename... A> void fetch_value(sql_blob& out, int field_i, Oid field_type);
  // Fetch an int from a result field.
  void fetch_value(int& out, int field_i, Oid field_type);
  // Fetch an unsigned int from a result field.
  void fetch_value(unsigned int& out, int field_i, Oid field_type);
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HPP

#include "libpq-fe.h"
//#include <catalog/pg_type_d.h>

#define INT8OID 20
#define INT2OID 21
#define INT4OID 23

namespace li {

template <typename Y> PGresult* pgsql_result<Y>::wait_for_next_result() {
  return connection_->wait_for_next_pgresult(fiber_);
}

template <typename Y> void pgsql_result<Y>::flush_results() {
  if (end_of_result_) return;
  // println("pgsql_result<Y>::flush_results ", this->result_id_);
  this->connection_->wait_for_result(fiber_, this->result_id_);

  // println("pgsql_result<Y>::flush_results result ready for flush.");

  if (current_result_)
  {
    PQclear(current_result_);
    current_result_ = nullptr;
  }
  while (true)
  {
    if (connection_->error_ == 1) break;
    PGresult* res = connection_->wait_for_next_pgresult(fiber_, true);
    if (res)
      PQclear(res);
    else break;
  }

  // println("pgsql_result<Y>::flush_results OK ", this->result_id_);
  end_of_result_ = true; 
  this->connection_->end_of_current_result(fiber_);
}

// Fetch a string from a result field.
template <typename Y>
template <typename... A>
void pgsql_result<Y>::fetch_value(std::string& out, int field_i,
                                  Oid field_type) {
  // assert(!is_binary);
  // std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
  out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
                              PQgetlength(current_result_, row_i_, field_i)));
  // out = std::move(std::string(val, strlen(val)));
}

// Fetch a blob from a result field.
template <typename Y>
template <typename... A>
void pgsql_result<Y>::fetch_value(sql_blob& out, int field_i, Oid field_type) {
  // assert(is_binary);
  out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
                              PQgetlength(current_result_, row_i_, field_i)));
}

// Fetch an int from a result field.
template <typename Y>
void pgsql_result<Y>::fetch_value(int& out, int field_i, Oid field_type) {
  assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
  char* val = PQgetvalue(current_result_, row_i_, field_i);

  // TYPCATEGORY_NUMERIC
  // std::cout << "fetch integer " << length << " " << is_binary << std::endl;
  // std::cout << "fetch integer " << be64toh(*((uint64_t *) val)) << std::endl;
  if (field_type == INT8OID) {
    // std::cout << "fetch 64b integer " << std::hex << int(32) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << uint64_t(*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << (*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << be64toh(*((uint64_t *) val)) << std::endl;
    out = be64toh(*((uint64_t*)val));
  } else if (field_type == INT4OID)
    out = (uint32_t)ntohl(*((uint32_t*)val));
  else if (field_type == INT2OID)
    out = (uint16_t)ntohs(*((uint16_t*)val));
  else
    throw std::runtime_error("The type of request result does not match the destination type");
}

// Fetch an unsigned int from a result field.
template <typename Y>
void pgsql_result<Y>::fetch_value(unsigned int& out, int field_i, Oid field_type) {
  assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
  char* val = PQgetvalue(current_result_, row_i_, field_i);

  // if (length == 8)
  if (field_type == INT8OID)
    out = be64toh(*((uint64_t*)val));
  else if (field_type == INT4OID)
    out = ntohl(*((uint32_t*)val));
  else if (field_type == INT2OID)
    out = ntohs(*((uint16_t*)val));
  else
    assert(0);
}


template <typename B> template <typename T> bool pgsql_result<B>::fetch_next_result(T&& output) {

  // println("pgsql_result<B>::fetch_next_result", this->result_id_);
  if (end_of_result_) return false;
  // std::cout << "read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "connection_->current_result_id_ " << connection_->current_result_id_ << " this->result_id_ " << this->result_id_ << std::endl;

  // if (!current_result_)
  // First wait that the right result_id in the query result pipeline.
  connection_->wait_for_result(fiber_, this->result_id_);

  // std::cout << "GO read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "GO connection_->current_result_id_ " << connection_->current_result_id_ << " this->result_id_ " << this->result_id_ << std::endl;
  assert(connection_->current_result_id_ == this->result_id_);
  // std::cout << "currently reading result " << this->result_id_ << std::endl;
  if (current_result_ && row_i_ < current_result_nrows_ - 1)
  {
    this->row_i_++;
  }
  else {

    current_result_nrows_ = 0;
    while (current_result_nrows_ == 0)
    {
      if (current_result_) {
        PQclear(current_result_);
        current_result_ = nullptr;
      }

      assert(connection_->current_result_id_ == this->result_id_);
      current_result_ = wait_for_next_result();
      // println("got next result ", current_result_);
      // std::cout <<
      if (connection_->current_result_id_ != this->result_id_) 
      {
        std::cout << " in fetch_next_result: fiberid " << fiber_.continuation_idx << std::endl;
        std::cout << " connection_->current_result_id_ " << connection_->current_result_id_ << " this->result_id_: " << this->result_id_ << std::endl;
      }
      assert(connection_->current_result_id_ == this->result_id_);
      if (!current_result_)
      {
        current_result_nrows_ = 0;
        row_i_ = 0;
        end_of_result_ = true; 
        this->connection_->end_of_current_result(fiber_);
        // std::cout << " connection_->current_result_id_ " << connection_->current_result_id_ << " this->result_id_: " << this->result_id_ << std::endl;
        // assert(connection_->current_result_id_ != this->result_id_);
        return false;
      }
      row_i_ = 0;
      current_result_nrows_ = PQntuples(current_result_);
    }

    // Metamaps.
    if constexpr (is_metamap<std::decay_t<T>>::value) {
      curent_result_field_positions_.clear();
      li::map(std::forward<T>(output), [&](auto k, auto& m) {
        curent_result_field_positions_.push_back(PQfnumber(current_result_, symbol_string(k)));
        if (curent_result_field_positions_.back() == -1)
          throw std::runtime_error(std::string("postgresql errror : Field ") + symbol_string(k) +
                                    " not found in result.");

      });
    }

    if (curent_result_field_types_.size() == 0) {

      curent_result_field_types_.resize(PQnfields(current_result_));
      for (int field_i = 0; field_i < curent_result_field_types_.size(); field_i++)
        curent_result_field_types_[field_i] = PQftype(current_result_, field_i);
    }
  }
  return true;
}

template <typename B> template <typename T> bool pgsql_result<B>::read(T&& output) {

  if (end_of_result_) return false;
  // println("pgsql_result<B>::read");

  // Just for the first row, we fetch the result.
  // For the next calls fetch_next_result is called at the end of this function.
  if (!current_result_ && !this->fetch_next_result<T>(std::forward<T>(output))) return false;

  assert(connection_->current_result_id_ == this->result_id_);

  // std::cout << "got a row! " << std::endl;

  // Tuples
  if constexpr (is_tuple<T>::value) {
    int field_i = 0;

    int nfields = curent_result_field_types_.size();
    if (nfields != std::tuple_size_v<std::decay_t<T>>)
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
                               "field and the outputs.");

    tuple_map(std::forward<T>(output), [&](auto& m) {
      fetch_value(m, field_i, curent_result_field_types_[field_i]);
      field_i++;
    });
  } else { // Metamaps.
    int i = 0;
    li::map(std::forward<T>(output), [&](auto k, auto& m) {
      int field_i = curent_result_field_positions_[i];
      fetch_value(m, field_i, curent_result_field_types_[field_i]);
      i++;
    });
  }

  this->fetch_next_result<T>(std::forward<T>(output));
  return true;
}

// Get the last id of the row inserted by the last command.
template <typename Y> long long int pgsql_result<Y>::last_insert_id() {
  // while (PGresult* res = wait_for_next_result())
  //  PQclear(res);
  // PQsendQuery(connection_, "LASTVAL()");
  int t = 0;
  this->read(std::tie(t));
  return t;
  // PGresult *PQexec(connection_, const char *command);
  // this->operator()
  //   last_insert_id_ = PQoidValue(res);
  //   std::cout << "id " << last_insert_id_ << std::endl;
  //   PQclear(res);
  // }
  // return last_insert_id_;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_RESULT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HH


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH

#ifndef LI_SYMBOL_ATTR
#define LI_SYMBOL_ATTR
    LI_SYMBOL(ATTR)
#endif

#ifndef LI_SYMBOL_after_insert
#define LI_SYMBOL_after_insert
    LI_SYMBOL(after_insert)
#endif

#ifndef LI_SYMBOL_after_remove
#define LI_SYMBOL_after_remove
    LI_SYMBOL(after_remove)
#endif

#ifndef LI_SYMBOL_after_update
#define LI_SYMBOL_after_update
    LI_SYMBOL(after_update)
#endif

#ifndef LI_SYMBOL_auto_increment
#define LI_SYMBOL_auto_increment
    LI_SYMBOL(auto_increment)
#endif

#ifndef LI_SYMBOL_before_insert
#define LI_SYMBOL_before_insert
    LI_SYMBOL(before_insert)
#endif

#ifndef LI_SYMBOL_before_remove
#define LI_SYMBOL_before_remove
    LI_SYMBOL(before_remove)
#endif

#ifndef LI_SYMBOL_before_update
#define LI_SYMBOL_before_update
    LI_SYMBOL(before_update)
#endif

#ifndef LI_SYMBOL_charset
#define LI_SYMBOL_charset
    LI_SYMBOL(charset)
#endif

#ifndef LI_SYMBOL_computed
#define LI_SYMBOL_computed
    LI_SYMBOL(computed)
#endif

#ifndef LI_SYMBOL_connections
#define LI_SYMBOL_connections
    LI_SYMBOL(connections)
#endif

#ifndef LI_SYMBOL_database
#define LI_SYMBOL_database
    LI_SYMBOL(database)
#endif

#ifndef LI_SYMBOL_host
#define LI_SYMBOL_host
    LI_SYMBOL(host)
#endif

#ifndef LI_SYMBOL_id
#define LI_SYMBOL_id
    LI_SYMBOL(id)
#endif

#ifndef LI_SYMBOL_max_async_connections_per_thread
#define LI_SYMBOL_max_async_connections_per_thread
    LI_SYMBOL(max_async_connections_per_thread)
#endif

#ifndef LI_SYMBOL_max_connections
#define LI_SYMBOL_max_connections
    LI_SYMBOL(max_connections)
#endif

#ifndef LI_SYMBOL_max_sync_connections
#define LI_SYMBOL_max_sync_connections
    LI_SYMBOL(max_sync_connections)
#endif

#ifndef LI_SYMBOL_n_connections
#define LI_SYMBOL_n_connections
    LI_SYMBOL(n_connections)
#endif

#ifndef LI_SYMBOL_password
#define LI_SYMBOL_password
    LI_SYMBOL(password)
#endif

#ifndef LI_SYMBOL_port
#define LI_SYMBOL_port
    LI_SYMBOL(port)
#endif

#ifndef LI_SYMBOL_primary_key
#define LI_SYMBOL_primary_key
    LI_SYMBOL(primary_key)
#endif

#ifndef LI_SYMBOL_read_access
#define LI_SYMBOL_read_access
    LI_SYMBOL(read_access)
#endif

#ifndef LI_SYMBOL_read_only
#define LI_SYMBOL_read_only
    LI_SYMBOL(read_only)
#endif

#ifndef LI_SYMBOL_synchronous
#define LI_SYMBOL_synchronous
    LI_SYMBOL(synchronous)
#endif

#ifndef LI_SYMBOL_user
#define LI_SYMBOL_user
    LI_SYMBOL(user)
#endif

#ifndef LI_SYMBOL_validate
#define LI_SYMBOL_validate
    LI_SYMBOL(validate)
#endif

#ifndef LI_SYMBOL_write_access
#define LI_SYMBOL_write_access
    LI_SYMBOL(write_access)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH



namespace li {

/**
 * @brief Provide access to the result of a sql query.
 */
template <typename I> struct sql_result {

  I impl_;

  // sql_result() = delete;
  // sql_result& operator=(sql_result&) = delete;
  // sql_result(const sql_result&) = delete;

  inline ~sql_result() { 
    // this->flush_results(); 
    }

  inline void flush_results() { 
    impl_.flush_results(); 
  }

  /**
   * @brief Return the last id generated by a insert comment.
   * With postgresql, it requires the previous command to use the "INSERT [...] returning id;"
   * syntax.
   *
   * @return the last inserted id.
   */
  long long int last_insert_id() { return impl_.last_insert_id(); }

  /**
   * @brief read one row of the result set and advance to next row.
   * Throw an exception if the end of the result set is reached.
   *
   * @return If only one template argument is provided return this same type.
   *         otherwise return a std::tuple of the requested types.
   */
  template <typename T1, typename... T> auto read();
  /**
   * @brief Like read<T>() but do not throw is the eand of the result set is reached, instead
   * it wraps the result in a std::optional that is empty if no data could be fetch.
   *
   */
  template <typename T1, typename... T> auto read_optional();

  /**
   * @brief read one row of the result set and advance to next row.
   * Throw an exception if the end of the result set is reached or if another error happened.
   *
   * Valid calls are:
   *    read(std::tuple<...>& )
   *        fill the tuple according to the current row. The tuple must match
   *        the number of fields in the request.
   *    read(li::metamap<...>& )
   *        fill the metamap according to the current row. The metamap (value types and keys) must
   * match the fields (types and names) of the request. 
   *    read(A& a, B& b, C& c, ...) 
   *        fill a, b, c,...
   *        with each field of the current row. Types of a, b, c... must match the types of the fields.
   *        supported types : only values (i.e not tuples or metamaps) like std::string, integer and floating numbers.
   * @return T the result value.
   */
  template <typename T1, typename... T> bool read(T1&& t1, T&... tail);
  template <typename T> void read(std::optional<T>& o);

  /**
   * @brief Call \param f on each row of the set.
   * The function must take as argument all the select fields of the request, it should
   * follow one of the signature of read (check read documentation for more info).
   * \param f can take arguments by reference to avoid copies but keep in mind that
   * there references will be invalid at the end of the function scope.
   *
   * @example connection.prepare("Select id,post from post_items;")().map(
   *        [&](std::string id, std::string post) {
   *             std::cout << id << post << std::endl; });
   *
   * @param f the function.
   */
  template <typename F> void map(F f);
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP


namespace li {
template <typename B>
template <typename T1, typename... T>
bool sql_result<B>::read(T1&& t1, T&... tail) {

  // Metamap and tuples
  if constexpr (li::is_metamap<std::decay_t<T1>>::value || li::is_tuple<std::decay_t<T1>>::value) {
    static_assert(sizeof...(T) == 0);
    return impl_.read(std::forward<T1>(t1));
  }
  // Scalar values.
  else
    return impl_.read(std::tie(t1, tail...));
}

template <typename B> template <typename T1, typename... T> auto sql_result<B>::read() {
  auto t = [] {
    if constexpr (sizeof...(T) == 0)
      return T1{};
    else
      return std::tuple<T1, T...>{};
  }();
  if (!this->read(t))
    throw std::runtime_error("Trying to read a request that did not return any data.");
  return t;
}

template <typename B> template <typename T> void sql_result<B>::read(std::optional<T>& o) {
  o = this->read_optional<T>();
}

template <typename B>
template <typename T1, typename... T>
auto sql_result<B>::read_optional() {
  auto t = [] {
    if constexpr (sizeof...(T) == 0)
      return T1{};
    else
      return std::tuple<T1, T...>{};
  }();
  if (this->read(t))
    return std::make_optional(std::move(t));
  else
    return std::optional<decltype(t)>{};
}

namespace internal {

  template<typename T, typename F>
  constexpr auto is_valid(F&& f) -> decltype(f(std::declval<T>()), true) { return true; }

  template<typename>
  constexpr bool is_valid(...) { return false; }

}

#define IS_VALID(T, EXPR) internal::is_valid<T>( [](auto&& obj)->decltype(obj.EXPR){} )

template <typename B> template <typename F> void sql_result<B>::map(F map_function) {


  if constexpr (IS_VALID(B, map(map_function)))
    this->impl_.map(map_function);

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;
  typedef std::tuple_element_t<0, TP> TP0;

  auto t = [] {
    static_assert(std::tuple_size_v<TP> > 0, "sql_result map function must take at least 1 argument.");

    if constexpr (is_tuple<TP0>::value || is_metamap<TP0>::value)
      return TP0{};
    else
      return TP{};
  }();

  // int i = 0; 
  while (this->read(t)) {
    // println("read result ", i++);

    if constexpr (is_tuple<TP0>::value || is_metamap<TP0>::value)
      map_function(t);
    else
      std::apply(map_function, t);
  }

}
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH


namespace li {

struct pgsql_statement_data : std::enable_shared_from_this<pgsql_statement_data> {
  pgsql_statement_data(const std::string& s) : stmt_name(s) {}
  std::string stmt_name;
};

template <typename Y> struct pgsql_statement {

public:
  template <typename... T> sql_result<pgsql_result<Y>> operator()(T&&... args);

  std::shared_ptr<pgsql_connection_data> connection_;
  Y& fiber_;
  pgsql_statement_data& data_;

private:

  template <typename T>
  void free_bind_param(const T& m, const char** values);
  
  // Bind statement param utils.
  template <unsigned N>
  void bind_param(sql_varchar<N>&& m, const char** values, int* lengths, int* binary);
  template <unsigned N>
  void bind_param(const sql_varchar<N>& m, const char** values, int* lengths, int* binary);
  void bind_param(const char* m, const char** values, int* lengths, int* binary);
  template <typename T>
  void bind_param(const std::vector<T>& m, const char** values, int* lengths, int* binary);
  template <typename T> void bind_param(const T& m, const char** values, int* lengths, int* binary);
  template <typename T> unsigned int bind_compute_nparam(const T& arg);
  template <typename... T> unsigned int bind_compute_nparam(const metamap<T...>& arg);
  template <typename T> unsigned int bind_compute_nparam(const std::vector<T>& arg);
  // Bind parameter to the prepared statement and execute it.
  // FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HPP



namespace li {

// Execute a request with placeholders.
template <typename Y>
template <unsigned N>
void pgsql_statement<Y>::bind_param(sql_varchar<N>&& m, const char** values, int* lengths,
                                    int* binary) {
  // std::cout << "send param varchar " << m << std::endl;
  *values = m.c_str();
  *lengths = m.size();
  *binary = 0;
}
template <typename Y>
template <unsigned N>
void pgsql_statement<Y>::bind_param(const sql_varchar<N>& m, const char** values, int* lengths,
                                    int* binary) {
  // std::cout << "send param const varchar " << m << std::endl;
  *values = m.c_str();
  *lengths = m.size();
  *binary = 0;
}
template <typename Y>
void pgsql_statement<Y>::bind_param(const char* m, const char** values, int* lengths, int* binary) {
  // std::cout << "send param const char*[N] " << m << std::endl;
  *values = m;
  *lengths = strlen(m);
  *binary = 0;
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::bind_param(const std::vector<T>& m, const char** values, int* lengths,
                                    int* binary) {
  int tsize = [&] {
    if constexpr (is_metamap<T>::value)
      return metamap_size<T>();
    else
      return 1;
  }();

  int i = 0;
  for (int i = 0; i < m.size(); i++)
    bind_param(m[i], values + i * tsize, lengths + i * tsize, binary + i * tsize);
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::bind_param(const T& m, const char** values, int* lengths, int* binary) {
  if constexpr (is_metamap<std::decay_t<decltype(m)>>::value) {
    int i = 0;
    li::map(m, [&](auto k, const auto& m) {
      bind_param(m, values + i, lengths + i, binary + i);
      i++;
    });
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value or
                       std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
    // std::cout << "send param string: " << m << std::endl;
    *values = m.c_str();
    *lengths = m.size();
    *binary = 0;
  } else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
    // std::cout << "send param const char* " << m << std::endl;
    *values = m;
    *lengths = strlen(m);
    *binary = 0;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
    *values = (char*)new int(htonl(m));
    *lengths = sizeof(m);
    *binary = 1;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
    // FIXME send 64bit values.
    // std::cout << "long long int param: " << m << std::endl;
    *values = (char*)new int(htonl(uint32_t(m)));
    *lengths = sizeof(uint32_t);
    // does not work:
    // values = (char*)new uint64_t(htobe64((uint64_t) m));
    // lengths = sizeof(uint64_t);
    *binary = 1;
  }
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::free_bind_param(const T& m, const char** values) {
  if constexpr (is_metamap<std::decay_t<decltype(m)>>::value) {
    int i = 0;
    li::map(m, [&](auto k, const auto& m) {
      free_bind_param(m, values + i);
      i++;
    });
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value or
                       std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
  } else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
    delete *values;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
    delete *values;
  }
}

template <typename Y>
template <typename T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const T& arg) {
  return 1;
}
template <typename Y>
template <typename... T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const metamap<T...>& arg) {
  return sizeof...(T);
}
template <typename Y>
template <typename T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const std::vector<T>& arg) {
  return arg.size() * bind_compute_nparam(arg[0]);
}

// Bind parameter to the prepared statement and execute it.
template <typename Y>
template <typename... T>
sql_result<pgsql_result<Y>> pgsql_statement<Y>::operator()(T&&... args) {

  unsigned int nparams = 0;
  if constexpr (sizeof...(T) > 0)
    nparams = (bind_compute_nparam(std::forward<T>(args)) + ...);
  const char* values_[nparams];
  int lengths_[nparams];
  int binary_[nparams];

  const char** values = values_;
  int* lengths = lengths_;
  int* binary = binary_;

  int i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](const auto& a) {
    bind_param(a, values + i, lengths + i, binary + i);
    i += bind_compute_nparam(a);
  });

  // std::cout << "PQsendQueryPrepared" << std::endl; 
  if (!PQsendQueryPrepared(connection_->pgconn_, data_.stmt_name.c_str(), nparams, values, lengths, binary,
                           1)) {
    throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_->pgconn_));
  }

  i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](const auto& a) {
    free_bind_param(a, values + i);
    i += bind_compute_nparam(a);
  });

  // Not calling pqflush seems to work aswell...
  // connection_->flush(this->fiber_);

  // this->connection_->batch_query(this->fiber_, true);
  int query_result_id = this->connection_->batch_query(this->fiber_);
  // println("build result ", query_result_id);
  return sql_result<pgsql_result<Y>>{
      pgsql_result<Y>(this->connection_, this->fiber_, query_result_id)};
}

// FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_STATEMENT_HH


namespace li {

struct pgsql_tag {};

// template <typename Y> void pq_wait(Y& yield, PGconn* con) {
//   while (PQisBusy(con))
//     yield();
// }

template <typename Y> struct pgsql_connection {

  Y& fiber_;
  std::shared_ptr<pgsql_connection_data> data_;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>>& stm_cache_;
  PGconn* connection_;

  typedef pgsql_tag db_tag;

  inline pgsql_connection(const pgsql_connection&) = delete;
  inline pgsql_connection& operator=(const pgsql_connection&) = delete;
  inline pgsql_connection(pgsql_connection&& o) = default;

  inline pgsql_connection(Y& fiber, std::shared_ptr<pgsql_connection_data>& data)
      : fiber_(fiber), data_(data), stm_cache_(data->statements), connection_(data->pgconn_) {

  }

  // FIXME long long int last_insert_rowid() { return pgsql_insert_id(connection_); }

  // pgsql_statement<Y> operator()(const std::string& rq) { return prepare(rq)(); }

  auto operator()(const std::string& rq) {
    if (!PQsendQueryParams(connection_, rq.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1))
      throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
    return sql_result<pgsql_result<Y>>{
        pgsql_result<Y>{this->data_, this->fiber_, data_->error_}};
  }

  // PQsendQueryParams
  template <typename F, typename... K> pgsql_statement<Y> cached_statement(F f, K... keys) {
    if (data_->statements_hashmap(f, keys...).get() == nullptr) {
      pgsql_statement<Y> res = prepare(f());
      data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
      return res;
    } else
      return pgsql_statement<Y>{data_, fiber_, *data_->statements_hashmap(f, keys...)};
  }

  inline void end_of_batch() {
    this->data_->send_end_batch();
  }

  inline int batch_queue_size(Y& fiber) {
    if (this->data_->current_result_id_ == -1)
      this->data_->flush_ignored_results(fiber);
    return this->data_->batched_queries.size();
  }

  pgsql_statement<Y> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end()) {
      return pgsql_statement<Y>{data_, fiber_, *it->second};
    }
    std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

    if (!PQsendPrepare(connection_, stmt_name.c_str(), rq.c_str(), 0, nullptr)) {
      throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
    }

    // flush results.
    // FIXME do we need to flush in batch mode ?
    // while (PGresult* ret = pg_wait_for_next_result(connection_, fiber_, data_->error_))
    //   PQclear(ret);

    this->data_->batch_query(this->fiber_, true);

    auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name));
    return pgsql_statement<Y>{data_, fiber_, *pair.first->second};
  }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INT";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "DOUBLE";
  }
  inline std::string type_to_string(const std::string&) { return "TEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned S> inline std::string type_to_string(const sql_varchar<S>) {
    std::ostringstream ss;
    ss << "VARCHAR(" << S << ')';
    return ss.str();
  }
};

} // namespace li


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_CONNECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH


namespace li {
// thread local map of sql_database<I> -> sql_database_thread_local_data<I>;
// This is used to store the thread local async connection pool.
thread_local std::unordered_map<int, void*> sql_thread_local_data;
thread_local int database_id_counter = 0;

template <typename I> struct sql_database_thread_local_data {

  typedef typename I::connection_data_type connection_data_type;

  // Async connection pools.
  std::deque<connection_data_type*> async_connections_;

  int n_connections_on_this_thread_ = 0;
};

struct active_yield {
  typedef std::runtime_error exception_type;
  int continuation_idx = 0;
  void defer(std::function<void()>) {}
  void defer_fiber_resume(int fiber_id) {}
  void reassign_fd_to_fiber(int fd, int fiber_id) {}
  void epoll_add(int, int) {}
  void epoll_mod(int, int) {}
  void yield() {}
};

template <typename I> struct sql_database {
  I impl;

  typedef typename I::connection_data_type connection_data_type;
  typedef typename I::db_tag db_tag;

  // Sync connections pool.
  std::deque<connection_data_type*> sync_connections_;
  // Sync connections mutex.
  std::mutex sync_connections_mutex_;

  int n_sync_connections_ = 0;
  int max_sync_connections_ = 0;
  int max_async_connections_per_thread_ = 0;
  int database_id_ = 0;

  int next_connection_position_ = 0;
  template <typename... O> sql_database(O&&... opts) : impl(std::forward<O>(opts)...) {
    auto options = mmm(opts...);
    max_async_connections_per_thread_ = get_or(options, s::max_async_connections_per_thread, 2);
    max_sync_connections_ = get_or(options, s::max_sync_connections, 50);

    this->database_id_ = database_id_counter++;
  }

  ~sql_database() {
    clear_connections();
  }

  void clear_connections() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it != sql_thread_local_data.end())
    {
      auto store = (sql_database_thread_local_data<I>*) it->second;
      for (auto ptr : store->async_connections_)
        delete ptr;
      delete store;
      // delete (sql_database_thread_local_data<I>*) sql_thread_local_data[this->database_id_];
      sql_thread_local_data.erase(this->database_id_);
    }

    std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
    for (auto* ptr : this->sync_connections_)
      delete ptr;
    sync_connections_.clear();
    n_sync_connections_ = 0;
  }

  auto& thread_local_data() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it == sql_thread_local_data.end())
    {
      auto data = new sql_database_thread_local_data<I>;
      sql_thread_local_data[this->database_id_] = data;
      return *data;
    }
    else
      return *(sql_database_thread_local_data<I>*) it->second;
  }
  /**
   * @brief Build aa new database connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenever its constructor is called.
   *
   * @param fiber the fiber object providing the 3 non blocking logic methods:
   *
   *    - void epoll_add(int fd, int flags); // Make the current epoll fiber wakeup on
   *                                            file descriptor fd
   *    - void epoll_mod(int fd, int flags); // Modify the epoll flags on file
   *                                            descriptor fd
   *    - void yield() // Yield the current epoll fiber.
   *
   * @return the new connection.
   */
  template <typename Y> inline auto connect(Y& fiber) {

    auto pool = [this] {

      if constexpr (std::is_same_v<Y, active_yield>) // Synchonous mode
        return make_metamap_reference(
            s::connections = this->sync_connections_,
            s::n_connections = this->n_sync_connections_,
            s::max_connections = this->max_sync_connections_);
      else  // Asynchonous mode
        return make_metamap_reference(
            s::connections = this->thread_local_data().async_connections_,
            s::n_connections =  this->thread_local_data().n_connections_on_this_thread_,
            s::max_connections = this->max_async_connections_per_thread_);
    }();

    connection_data_type* data = nullptr;
    bool reuse = false;
    // std::cout << "Try CONNECT!" << std::endl;
    while (!data) {

    // std::cout << "Try CONNECT! "<< pool.connections.size() << std::endl;

#define SHARED_CONNECTION_BETWEEN_FIBER
#ifdef SHARED_CONNECTION_BETWEEN_FIBER

      // std::cout << pool.connections.size() << " " << pool.n_connections << " " <<  pool.max_connections << std::endl; 
      if (pool.n_connections < pool.max_connections) {
        for (int i = pool.n_connections; pool.n_connections < pool.max_connections; i++) {
          try {
            pool.n_connections++;
            // std::cout << "n_connections: " << pool.n_connections << " max: " << pool.max_connections << std::endl;
            data = impl.new_connection(fiber);
            if (data)
            {
              auto lock = [&pool, this] {
                if constexpr (std::is_same_v<Y, active_yield>)
                  return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
                else return 0;
              }();
              pool.connections.push_back(data);
            }
            else {
              pool.n_connections--;
              fiber.yield();
              // break;
            }
          } catch (typename Y::exception_type& e) {
            pool.n_connections--;
            throw std::move(e);
          }
        }
      }
      else if (pool.connections.size())
      {
        data = pool.connections[(next_connection_position_++) % pool.connections.size()];
      }
      else {
        fiber.yield();
      }

#else
      if (!pool.connections.empty()) {
        // std::cout << "Try to reuse!" << std::endl;
        auto lock = [&pool, this] {
          if constexpr (std::is_same_v<Y, active_yield>)
            return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
          else return 0;
        }();
        data = pool.connections.back();
        // pool.connections.pop_back();
        reuse = true;
      } else {
        if (pool.n_connections >= pool.max_connections) {
          if constexpr (std::is_same_v<Y, active_yield>)
            throw std::runtime_error("Maximum number of sql connection exeeded.");
          else
            std::cout << "Waiting for a free sql connection..." << std::endl;
            fiber.yield();
          continue;
        }
        pool.n_connections++;

        try {
          data = impl.new_connection(fiber);
        } catch (typename Y::exception_type& e) {
          pool.n_connections--;
          throw std::move(e);
        }

        if (!data)
          pool.n_connections--;
        else
          pool.connections.push_back(data);
      }
#endif
    }

    assert(data);
    assert(data->error_ == 0);
    
    // std::cout << "CONNECT!" << std::endl;
    auto sptr = std::shared_ptr<connection_data_type>(data, [pool, this](connection_data_type* data) {
#ifndef SHARED_CONNECTION_BETWEEN_FIBER
          // if (!data->error_) { // && pool.connections.size() < pool.max_connections) {
          //   auto lock = [&pool, this] {
          //     if constexpr (std::is_same_v<Y, active_yield>)
          //       return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
          //     else return 0;
          //   }();

          //   pool.connections.push_back(data);
            
          // } else {
          //   if (pool.connections.size() >= pool.max_connections)
          //     std::cerr << "Error: connection pool size " << pool.connections.size()
          //               << " exceed pool max_connections " << pool.max_connections << std::endl;
          //   // pool.n_connections--;
          //   // delete data;
          // }
#endif
        });

#ifndef SHARED_CONNECTION_BETWEEN_FIBER
    if (reuse) 
      fiber.epoll_add(impl.get_socket(sptr), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
#endif
    return impl.scoped_connection(fiber, sptr);
  }

  /**
   * @brief Provide a new mysql blocking connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenver its constructor is called.
   *
   * @return the connection.
   */
  inline auto connect() { active_yield yield; return this->connect(yield); }
};

}
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH


namespace li {

struct pgsql_database_impl {

  typedef pgsql_connection_data connection_data_type;

  typedef pgsql_tag db_tag;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::string character_set_;

  template <typename... O> inline pgsql_database_impl(O... opts) {

    auto options = mmm(opts...);
    static_assert(has_key(options, s::host), "open_pgsql_connection requires the s::host argument");
    static_assert(has_key(options, s::database),
                  "open_pgsql_connection requires the s::databaser argument");
    static_assert(has_key(options, s::user), "open_pgsql_connection requires the s::user argument");
    static_assert(has_key(options, s::password),
                  "open_pgsql_connection requires the s::password argument");

    host_ = options.host;
    database_ = options.database;
    user_ = options.user;
    passwd_ = options.password;
    port_ = get_or(options, s::port, 5432);
    character_set_ = get_or(options, s::charset, "utf8");

    if (!PQisthreadsafe())
      throw std::runtime_error("LibPQ is not threadsafe.");
  }

  inline int get_socket(const std::shared_ptr<pgsql_connection_data>& data) {
    return PQsocket(data->pgconn_);
  }

  template <typename Y> inline pgsql_connection_data* new_connection(Y& fiber) {

    PGconn* connection = nullptr;
    int pgsql_fd = -1;
    std::stringstream coninfo;
    coninfo << "postgresql://" << user_ << ":" << passwd_ << "@" << host_ << ":" << port_ << "/"
            << database_;
    // connection = PQconnectdb(coninfo.str().c_str());
    connection = PQconnectStart(coninfo.str().c_str());
    if (!connection) {
      std::cerr << "Warning: PQconnectStart returned null." << std::endl;
      return nullptr;
    }
    if (PQsetnonblocking(connection, 1) == -1) {
      std::cerr << "Warning: PQsetnonblocking returned -1: " << PQerrorMessage(connection)
                << std::endl;
      PQfinish(connection);
      return nullptr;
    }

    int status = PQconnectPoll(connection);

    pgsql_fd = PQsocket(connection);
    if (pgsql_fd == -1) {
      std::cerr << "Warning: PQsocket returned -1: " << PQerrorMessage(connection) << std::endl;
      // If PQsocket return -1, retry later.
      PQfinish(connection);
      return nullptr;
    }
    fiber.epoll_add(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);

    try {
      while (status != PGRES_POLLING_FAILED and status != PGRES_POLLING_OK) {
        int new_pgsql_fd = PQsocket(connection);
        if (new_pgsql_fd != pgsql_fd) {
          pgsql_fd = new_pgsql_fd;
          fiber.epoll_add(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
        }
        fiber.yield();
        status = PQconnectPoll(connection);
      }
    } catch (typename Y::exception_type& e) {
      // Yield thrown a exception (probably because a closed connection).
      PQfinish(connection);
      throw std::move(e);
    }
    fiber.epoll_mod(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    if (status != PGRES_POLLING_OK) {
      std::cerr << "Warning: cannot connect to the postgresql server " << host_ << ": "
                << PQerrorMessage(connection) << std::endl;
      PQfinish(connection);
      return nullptr;
    }

    // PQexec(connection, "SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED");

    if (PQenterPipelineMode(connection) == 0)
    {
      std::cerr << "PQenterPipelineMode error: Is the connection idle?" << std::endl;
      PQfinish(connection);
      return nullptr;      
    }

    // std::cout << "CONNECTED " << std::endl;

    // pgsql_set_character_set(pgsql, character_set_.c_str());
    return new pgsql_connection_data{connection, pgsql_fd};
  }

  template <typename Y>
  auto scoped_connection(Y& fiber, std::shared_ptr<pgsql_connection_data>& data) {
    return pgsql_connection<Y>(fiber, data);
  }
};

typedef sql_database<pgsql_database_impl> pgsql_database;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_DATABASE_HH


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_PGSQL_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH


namespace li {

constexpr int count_first_falses() { return 0; }

template <typename... B> constexpr int count_first_falses(bool b1, B... b) {
  if (b1)
    return 0;
  else
    return 1 + count_first_falses(b...);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E* a1, T&&... args) {
  return std::forward<E*>(a1);
}

template <typename E, typename... T>
decltype(auto) arg_get_by_type_(void*, const E* a1, T&&... args) {
  return std::forward<const E*>(a1);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E& a1, T&&... args) {
  return std::forward<E&>(a1);
}

template <typename E, typename... T>
decltype(auto) arg_get_by_type_(void*, const E& a1, T&&... args) {
  return std::forward<const E&>(a1);
}

template <typename E, typename T1, typename... T>
decltype(auto) arg_get_by_type_(std::enable_if_t<!std::is_same<E, std::decay_t<T1>>::value>*,
                                // void*,
                                T1&&, T&&... args) {
  return arg_get_by_type_<E>((void*)0, std::forward<T>(args)...);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type(T&&... args) {
  return arg_get_by_type_<std::decay_t<E>>(0, args...);
}

template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>& tuple) {
  typedef std::decay_t<E> DE;
  return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
}

template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>&& tuple) {
  typedef std::decay_t<E> DE;
  return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
}

template <typename T, typename U> struct tuple_embeds : public std::false_type {};

template <typename... T, typename U>
struct tuple_embeds<std::tuple<T...>, U>
    : public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
                                              sizeof...(T)> {};

template <typename U, typename... T> struct tuple_embeds_any_ref_of : public std::false_type {};
template <typename U, typename... T>
struct tuple_embeds_any_ref_of<std::tuple<T...>, U>
    : public tuple_embeds<std::tuple<std::decay_t<T>...>, std::decay_t<U>> {};

template <typename T> struct tuple_remove_references;
template <typename... T> struct tuple_remove_references<std::tuple<T...>> {
  typedef std::tuple<std::remove_reference_t<T>...> type;
};

template <typename T> using tuple_remove_references_t = typename tuple_remove_references<T>::type;

template <typename T> struct tuple_remove_references_and_const;
template <typename... T> struct tuple_remove_references_and_const<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> type;
};

template <typename T>
using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;

template <typename T, typename U, typename E> struct tuple_remove_element2;

template <typename... T, typename... U, typename E1>
struct tuple_remove_element2<std::tuple<E1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U...>, E1> {};

template <typename... T, typename... U, typename T1, typename E1>
struct tuple_remove_element2<std::tuple<T1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U..., T1>, E1> {};

template <typename... U, typename E1>
struct tuple_remove_element2<std::tuple<>, std::tuple<U...>, E1> {
  typedef std::tuple<U...> type;
};

template <typename T, typename E>
struct tuple_remove_element : public tuple_remove_element2<T, std::tuple<>, E> {};

template <typename T, typename... E> struct tuple_remove_elements;

template <typename... T, typename E1, typename... E>
struct tuple_remove_elements<std::tuple<T...>, E1, E...> {
  typedef typename tuple_remove_elements<typename tuple_remove_element<std::tuple<T...>, E1>::type,
                                         E...>::type type;
};

template <typename... T> struct tuple_remove_elements<std::tuple<T...>> {
  typedef std::tuple<T...> type;
};

template <typename A, typename B> struct tuple_minus;

template <typename... T, typename... R> struct tuple_minus<std::tuple<T...>, std::tuple<R...>> {
  typedef typename tuple_remove_elements<std::tuple<T...>, R...>::type type;
};

template <typename T, typename... E>
using tuple_remove_elements_t = typename tuple_remove_elements<T, E...>::type;

template <typename F, size_t... I, typename... T>
inline F tuple_map(std::tuple<T...>& t, F f, std::index_sequence<I...>) {
  return (void)std::initializer_list<int>{((void)f(std::get<I>(t)), 0)...}, f;
}

template <typename F, typename... T> inline void tuple_map(std::tuple<T...>& t, F f) {
  tuple_map(t, f, std::index_sequence_for<T...>{});
}

template <typename F, size_t... I, typename T>
inline decltype(auto) tuple_transform(T&& t, F f, std::index_sequence<I...>) {
  return std::make_tuple(f(std::get<I>(std::forward<T>(t)))...);
}

template <typename F, typename T> inline decltype(auto) tuple_transform(T&& t, F f) {
  return tuple_transform(std::forward<T>(t), f,
                         std::make_index_sequence<std::tuple_size<std::decay_t<T>>{}>{});
}

template <template <class> class F, typename T, typename I, typename R, typename X = void>
struct tuple_filter_sequence;

template <template <class> class F, typename... T, typename R>
struct tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<>, R> {
  using ret = R;
};

template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
                             std::index_sequence<R...>, std::enable_if_t<F<T1>::value>> {
  using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
                                             std::index_sequence<R..., I1>>::ret;
};

template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
                             std::index_sequence<R...>, std::enable_if_t<!F<T1>::value>> {
  using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
                                             std::index_sequence<R...>>::ret;
};

template <std::size_t... I, typename T>
decltype(auto) tuple_filter_impl(std::index_sequence<I...>, T&& t) {
  return std::make_tuple(std::get<I>(t)...);
}

template <template <class> class F, typename T> decltype(auto) tuple_filter(T&& t) {
  using seq = typename tuple_filter_sequence<
      F, std::decay_t<T>, std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>,
      std::index_sequence<>>::ret;
  return tuple_filter_impl(seq{}, t);
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH


namespace li {

struct sqlite_tag;
struct mysql_tag;
struct pgsql_tag;

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename SCHEMA, typename C> struct sql_orm {

  typedef decltype(std::declval<SCHEMA>().all_fields()) O;
  typedef O object_type;

  ~sql_orm() {

    //assert(0);
  }
  sql_orm(sql_orm&&) = default;
  sql_orm(const sql_orm&) = delete;
  sql_orm& operator=(const sql_orm&) = delete;

  sql_orm(SCHEMA& schema, C&& con) : schema_(schema), con_(std::forward<C>(con)) {}

  template <typename S, typename... A> void call_callback(S s, A&&... args) {
    if constexpr (has_key<decltype(schema_.get_callbacks())>(S{}))
      return schema_.get_callbacks()[s](args...);
  }

  inline auto& drop_table_if_exists() {
    con_(std::string("DROP TABLE IF EXISTS ") + schema_.table_name()).flush_results();
    return *this;
  }

  inline auto& create_table_if_not_exists() {
    std::ostringstream ss;
    ss << "CREATE TABLE if not exists " << schema_.table_name() << " (";

    bool first = true;
    li::tuple_map(schema_.all_info(), [&](auto f) {
      auto f2 = schema_.get_field(f);
      typedef decltype(f) F;
      typedef decltype(f2) F2;
      typedef typename F2::left_t K;
      typedef typename F2::right_t V;

      bool auto_increment = SCHEMA::template is_auto_increment<F>::value;
      bool primary_key = SCHEMA::template is_primary_key<F>::value;
      K k{};
      V v{};

      if (!first)
        ss << ", ";
      ss << li::symbol_string(k) << " ";
      
      if (!std::is_same<typename C::db_tag, pgsql_tag>::value or !auto_increment)
        ss << con_.type_to_string(v);

      if (std::is_same<typename C::db_tag, sqlite_tag>::value) {
        if (auto_increment || primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<typename C::db_tag, mysql_tag>::value) {
        if (auto_increment)
          ss << " AUTO_INCREMENT NOT NULL";
        if (primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<typename C::db_tag, pgsql_tag>::value) {
        if (auto_increment)
          ss << " SERIAL PRIMARY KEY ";
      }

      first = false;
    });
    ss << ");";
    try {
      con_(ss.str()).flush_results();
    } catch (std::runtime_error e) {
      std::cerr << "Warning: Lithium::sql could not create the " << schema_.table_name() << " sql table."
                << std::endl
                << "You can ignore this message if the table already exists."
                << "The sql error is: " << e.what() << std::endl;
    }
    return *this;
  }

  std::string placeholder_string() {

    if (std::is_same<typename C::db_tag, pgsql_tag>::value) {
      placeholder_pos_++;
      std::stringstream ss;
      ss << '$' << placeholder_pos_;
      return ss.str();
    }
    else return "?";
  }

  template <typename W> void where_clause(W&& cond, std::ostringstream& ss) {
    ss << " WHERE ";
    bool first = true;
    map(cond, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = " << placeholder_string();
    });
    ss << " ";
  }

  template <typename... W, typename... A> auto find_one(metamap<W...> where, A&&... cb_args) {

    auto stmt = con_.cached_statement([&] (){ 
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "SELECT ";
        bool first = true;
        O o;
        li::map(o, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });

        ss << " FROM " << schema_.table_name();
        where_clause(where, ss);
        ss << "LIMIT 1";
        return ss.str();
    });

    return [=, query=li::tuple_reduce(metamap_values(where), stmt)] () mutable {
      O result;
      bool read_success = query.template read(metamap_values(result));
      if (read_success)
      {
        call_callback(s::read_access, result, cb_args...);
        return std::make_optional<O>(std::move(result));
      }
      else {
        return std::optional<O>{};
      }
    };

  }

  template <typename A, typename B, typename... O, typename... W>
  auto find_one(metamap<O...>&& o, assign_exp<A, B>&& w1, W... ws) {
    return find_one(cat(o, mmm(w1)), std::forward<W>(ws)...);
  }
  template <typename A, typename B, typename... W> auto find_one(assign_exp<A, B>&& w1, W&&... ws) {
    return find_one(mmm(w1), std::forward<W>(ws)...);
  }

  template <typename W> bool exists(W&& cond) {

    O o;
    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "SELECT count(*) FROM " << schema_.table_name();
        where_clause(cond, ss);
        ss << "LIMIT 1";
        return ss.str();
    });

    return li::tuple_reduce(metamap_values(cond), stmt).template read<int>();
  }

  template <typename A, typename B, typename... W> auto exists(assign_exp<A, B> w1, W... ws) {
    return exists(mmm(w1, ws...));
  }
  // Save a ll fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N, typename... A> auto insert(N&& o, A&&... cb_args) {

    auto values = schema_.without_auto_increment();
    map(o, [&](auto k, auto& v) { values[k] = o[k]; });

    call_callback(s::validate, values, cb_args...);
    call_callback(s::before_insert, values, cb_args...);


    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        std::ostringstream vs;

        placeholder_pos_ = 0;
        ss << "INSERT into " << schema_.table_name() << "(";

        bool first = true;
        li::map(values, [&](auto k, auto v) {
          if (!first) {
            ss << ",";
            vs << ",";
          }
          first = false;
          ss << li::symbol_string(k);
          vs << placeholder_string();
        });

        ss << ") VALUES (" << vs.str() << ")";

        if (std::is_same<typename C::db_tag, pgsql_tag>::value &&
            has_key(schema_.all_fields(), s::id))
          ss << " returning id;";
        return ss.str();
    });

    auto request_res = li::reduce(values, stmt);

    call_callback(s::after_insert, o, cb_args...);

    if constexpr(has_key<decltype(schema_.all_fields())>(s::id))
      return request_res.last_insert_id();
    else return request_res.flush_results();
  };

  template <typename A, typename B, typename... O, typename... W>
  auto insert(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return insert(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  auto insert(assign_exp<A, B> w1, W... ws) {
    return insert(mmm(w1), ws...);
  }

  // template <typename S, typename V, typename... A>
  // long long int insert(const assign_exp<S, V>& a, A&&... tail) {
  //   auto m = mmm(a, tail...);
  //   return insert(m);
  // }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {

    typedef decltype(schema_.all_fields()) O;

    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        placeholder_pos_ = 0;
      
        ss << "SELECT ";
        bool first = true;
        O o;
        li::map(o, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });
      
        ss << " FROM " << schema_.table_name();
        return ss.str();
    });
    // stmt().map([&](const O& o) { f(o); });
    using values_tuple = tuple_remove_references_and_const_t<decltype(metamap_values(std::declval<O>()))>;
    using keys_tuple = decltype(metamap_keys(std::declval<O>()));
    stmt().map([&](const values_tuple& values) { f(forward_tuple_as_metamap(keys_tuple{}, values)); });

  }

  // Update N's members except auto increment members.
  // N must have at least one primary key named id.
  // Only postgres is supported for now.
  template <typename N, typename... CB> auto bulk_update(const N& elements, CB&&... args) {

    // if constexpr(!std::is_same<typename C::db_tag, pgsql_tag>::value)
    //   for (const auto& o : elements)
    //     this->update(o);
    // else
    {
      
      auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        ss << "UPDATE " << schema_.table_name() << " SET ";

        int nfields = metamap_size<decltype(elements[0])>();
        bool first = true;
        map(elements[0], [&](auto k, auto v) {
          if (!first)
            ss << ",";
          if (not li::has_key(schema_.primary_key(), k))
          {
            ss << li::symbol_string(k) << " = tmp." << li::symbol_string(k);
            first = false;
          }
        });

        ss << " FROM (VALUES ";
        for (int i = 0; i < elements.size(); i++)
        {
          if (i != 0) ss << ',';
          ss << "(";
          for (int j = 0; j < nfields; j++)
          {
            if (j != 0) ss << ",";
            ss << "$" <<  1+i*nfields+j << "::int";
          }
          ss << ")";
        }
      
        ss << ") AS tmp(";
        first = true;
        map(elements[0], [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });
        ss << ") WHERE tmp.id = " << schema_.table_name() << ".id";      
        // std::cout << ss.str() << std::endl;
        return ss.str();
      }, elements.size());

      for (const auto& o : elements)
      {
        call_callback(s::validate, o, args...);
        call_callback(s::write_access, o, args...);
        call_callback(s::before_update, o, args...);
      }

      return stmt(elements);
    }
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N, typename... CB> void update(const N& o, CB&&... args) {
    // check if N has at least one member of PKS.

    call_callback(s::validate, o, args...);
    call_callback(s::write_access, o, args...);
    call_callback(s::before_update, o, args...);

    // static_assert(metamap_size<decltype(intersect(o, schema_.read_only()))>(),
    //"You cannot give read only fields to the orm update method.");

    auto to_update = substract(o, schema_.read_only());
    auto pk = intersection(o, schema_.primary_key());

    auto stmt = con_.cached_statement([&] { 
        static_assert(metamap_size<decltype(pk)>() > 0,
                      "You must provide at least one primary key to update an object.");
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "UPDATE " << schema_.table_name() << " SET ";

        bool first = true;

        map(to_update, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k) << " = " << placeholder_string();
        });

        where_clause(pk, ss);
        return ss.str();
    });

    li::tuple_reduce(std::tuple_cat(metamap_values(to_update), metamap_values(pk)), stmt);

    call_callback(s::after_update, o, args...);
  }

  template <typename A, typename B, typename... O, typename... W>
  void update(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return update(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void update(assign_exp<A, B> w1, W... ws) {
    return update(mmm(w1), ws...);
  }

  inline int count() {
    return con_.prepare(std::string("SELECT count(*) from ") + schema_.table_name())().template read<int>();
  }

  template <typename N, typename... CB> void remove(const N& o, CB&&... args) {

    call_callback(s::before_remove, o, args...);

    auto stmt = con_.cached_statement([&] { 
      std::ostringstream ss;
      placeholder_pos_ = 0;
      ss << "DELETE from " << schema_.table_name() << " WHERE ";

      bool first = true;
      map(schema_.primary_key(), [&](auto k, auto v) {
        if (!first)
          ss << " and ";
        first = false;
        ss << li::symbol_string(k) << " = " << placeholder_string();
      });
      return ss.str();
    });

    auto pks = intersection(o, schema_.primary_key());
    li::reduce(pks, stmt);

    call_callback(s::after_remove, o, args...);
  }
  template <typename A, typename B, typename... O, typename... W>
  void remove(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return remove(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void remove(assign_exp<A, B> w1, W... ws) {
    return remove(mmm(w1), ws...);
  }

  auto& schema() { return schema_; }

  C& backend_connection() { return con_; }

  SCHEMA schema_;
  C con_;
  int placeholder_pos_ = 0;
};

template <typename... F> struct orm_fields {

  orm_fields(F... fields) : fields_(fields...) {
    static_assert(sizeof...(F) == 0 || metamap_size<decltype(this->primary_key())>() != 0,
                  "You must give at least one primary key to the ORM. Use "
                  "s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> auto get_field(M m) { return m; }
  template <typename M, typename T> auto get_field(assign_exp<M, T> e) { return e; }
  template <typename M, typename T, typename... A>
  auto get_field(assign_exp<function_call_exp<M, A...>, T> e) {
    return assign_exp<M, T>{M{}, e.right};
  }

  // template <typename M> struct get_field { typedef M ret; };
  // template <typename M, typename T> struct get_field<assign_exp<M, T>> {
  //   typedef assign_exp<M, T> ret;
  //   static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  // };
  // template <typename M, typename T, typename... A>
  // struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M,
  // T>> {
  // };

// get_field<E>::ctor();
// field attributes checks.
#define CHECK_FIELD_ATTR(ATTR)                                                                     \
  template <typename M> struct is_##ATTR : std::false_type {};                                     \
  template <typename M, typename T, typename... A>                                                 \
  struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>>                                      \
      : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};                        \
                                                                                                   \
  auto ATTR() {                                                                                    \
    return tuple_map_reduce(fields_,                                                               \
                            [this](auto e) {                                                       \
                              typedef std::remove_reference_t<decltype(e)> E;                      \
                              if constexpr (is_##ATTR<E>::value)                                   \
                                return get_field(e);                                               \
                              else                                                                 \
                                return skip{};                                                     \
                            },                                                                     \
                            make_metamap_skip);                                                    \
  }

  CHECK_FIELD_ATTR(primary_key);
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);
  CHECK_FIELD_ATTR(computed);
#undef CHECK_FIELD_ATTR

  // Do not remove this comment, this is used by the symbol generation.
  // s::primary_key s::read_only s::auto_increment s::computed

  auto all_info() { return fields_; }

  auto all_fields() {

    return tuple_map_reduce(fields_,
                            [this](auto e) {
                              // typedef std::remove_reference_t<decltype(e)> E;
                              return get_field(e);
                            },
                            [](auto... e) { return mmm(e...); });
  }

  auto without_auto_increment() { return substract(all_fields(), auto_increment()); }
  auto all_fields_except_computed() {
    return substract(substract(all_fields(), computed()), auto_increment());
  }

  std::tuple<F...> fields_;
};

template <typename DB, typename MD = orm_fields<>, typename CB = decltype(mmm())>
struct sql_orm_schema : public MD {

  sql_orm_schema(DB& db, const std::string& table_name, CB cb = CB(), MD md = MD())
      : MD(md), database_(db), table_name_(table_name), callbacks_(cb) {}

  inline auto connect() { return sql_orm{*this, database_.connect()}; }
  template <typename Y>
  inline auto connect(Y& y) { return sql_orm{*this, database_.connect(y)}; }

  const std::string& table_name() const { return table_name_; }
  auto get_callbacks() const { return callbacks_; }

  template <typename... P> auto callbacks(P... params_list) const {
    auto cbs = mmm(params_list...);
    auto allowed_callbacks = mmm(s::before_insert, s::before_remove, s::before_update,
                                 s::after_insert, s::after_remove, s::after_update, s::validate);

    static_assert(
        metamap_size<decltype(substract(cbs, allowed_callbacks))>() == 0,
        "The only supported callbacks are: s::before_insert, s::before_remove, s::before_update,"
        " s::after_insert, s::after_remove, s::after_update, s::validate");
    return sql_orm_schema<DB, MD, decltype(cbs)>(database_, table_name_, cbs,
                                                 *static_cast<const MD*>(this));
  }

  template <typename... P> auto fields(P... p) const {
    return sql_orm_schema<DB, orm_fields<P...>, CB>(database_, table_name_, callbacks_,
                                                    orm_fields<P...>(p...));
  }

  DB& database_;
  std::string table_name_;
  CB callbacks_;
};

}; // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH

