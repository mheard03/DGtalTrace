#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/CPointFunctor.h>
#include "../common/CBool.cpp"
#include "CPointVector.cpp"
#include "CUnaryFunctor.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename T>
concept CPointFunctorImpl = requires {
  typename T::Point;
  typename T::Value;
  requires DGtalTrace::concepts::CPointVector<typename T::Point>;
  requires DGtalTrace::concepts::CUnaryFunctor<T, typename T::Point, typename T::Value>;
};

} // namespace detail

template <typename T, typename = void>
struct CPointFunctor : std::false_type {};

template <typename T>
requires detail::CPointFunctorImpl<T>
struct CPointFunctor<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CPointFunctor<T>));
};

} // namespace bridges

template <typename T>
concept CPointFunctor = requires {
  typename bridges::CPointFunctor<T>;
  requires CIsTrue<bridges::CPointFunctor<T>>;
};

} // namespace concepts
} // namespace DGtalTrace