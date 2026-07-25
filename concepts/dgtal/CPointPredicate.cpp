#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/CPointPredicate.h>
#include "../common/CBool.cpp"
#include "CPointVector.cpp"
#include "CUnaryFunctor.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename T>
concept CPointPredicateImpl = requires {
  typename T::Point;
  requires DGtalTrace::concepts::CPointVector<typename T::Point>;
  requires DGtalTrace::concepts::CUnaryFunctor<T, typename T::Point, bool>;
};

} // namespace detail

template <typename T, typename = void>
struct CPointPredicate : std::false_type {};

template <typename T>
requires detail::CPointPredicateImpl<T>
struct CPointPredicate<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CPointPredicate<T>));
};

} // namespace bridges

template <typename T>
concept CPointPredicate = requires {
  requires CIsTrue<bridges::CPointPredicate<T>>;
};

} // namespace concepts
} // namespace DGtalTrace