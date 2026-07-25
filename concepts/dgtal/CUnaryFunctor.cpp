#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/base/CUnaryFunctor.h>
#include "../BaseConcepts.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

/*
template <typename X, typename A, typename R>
concept CUnaryFunctorImpl =
    std::assignable_from<X&, const X&> &&
    requires(X x, std::add_lvalue_reference_t<A> a) {
      { x(a) } -> std::convertible_to<std::add_lvalue_reference_t<R>>;
    };
*/

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename X, typename A, typename R>
concept CUnaryFunctorImpl =
  std::assignable_from<X &, const X &> && requires(X x, A a) {
    { x(a) } -> ISBASICALLY<R>;
  };

#undef ISBASICALLY

} // namespace detail

template <typename X, typename A, typename R, typename = void>
struct CUnaryFunctor : std::false_type {};

template <typename X, typename A, typename R>
  requires detail::CUnaryFunctorImpl<X, A, R>
struct CUnaryFunctor<X, A, R> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CUnaryFunctor<X, A, R>));
};

} // namespace bridges

template <typename X, typename A, typename R>
concept CUnaryFunctor = requires {
  typename bridges::CUnaryFunctor<X, A, R>;
  requires CIsTrue<bridges::CUnaryFunctor<X, A, R>>;
};

} // namespace concepts
} // namespace DGtalTrace