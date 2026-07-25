#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/topology/CSurfelPredicate.h>
#include "../common/CBool.cpp"
#include "../CCell.cpp"
#include "CPredicate.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename T>
concept CSurfelPredicateImpl = requires {
  typename T::Surfel;
  requires DGtalTrace::concepts::CXXCell<typename T::Surfel>;
  requires DGtalTrace::concepts::CUnaryFunctor<T, typename T::Surfel, bool>;
};

} // namespace detail

template <typename T, typename = void>
struct CSurfelPredicate : std::false_type {};

template <typename T>
requires detail::CSurfelPredicateImpl<T>
struct CSurfelPredicate<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CSurfelPredicate<T>));
};

} // namespace bridges

template <typename T>
concept CSurfelPredicate = requires {
  requires CIsTrue<bridges::CSurfelPredicate<T>>;
};

} // namespace concepts
} // namespace DGtalTrace