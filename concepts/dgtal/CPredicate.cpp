#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/base/CPredicate.h>
#include "../common/CBool.cpp"
#include "CUnaryFunctor.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename T, typename TElement>
concept CPredicateImpl = requires {
  typename T::Element;
  std::same_as<TElement, typename T::Element>;
  requires DGtalTrace::concepts::CUnaryFunctor<T, TElement, bool>;
};

} // namespace detail

template <typename T, typename TElement, typename = void>
struct CPredicate : std::false_type {};

template <typename T, typename TElement>
requires detail::CPredicateImpl<T, TElement>
struct CPredicate<T, TElement> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CPredicate<T, TElement>));
};

} // namespace bridges

template <typename T, typename TElement>
concept CPredicate = requires {
  requires CIsTrue<bridges::CPredicate<T, TElement>>;
};


} // namespace concepts
} // namespace DGtalTrace