#pragma once
#include "CIsOfDimension.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <concepts>
#include <type_traits>

namespace DGtalTrace {
namespace concepts {

template <typename T>
concept CPoint = requires {
  DGtal::IsAPointVector<T>::value == true;
};

template <typename X, typename A, typename R>
concept CUnaryFunctor =
    std::assignable_from<X &, const X &> &&
    requires(X x, std::add_lvalue_reference_t<A> a) {
      { x(a) } -> std::convertible_to<std::add_lvalue_reference_t<R>>;
    };

template <typename T>
concept CPointFunctor = requires {
  typename T::Point;
  typename T::Value;
  CPoint<typename T::Point>;
  CUnaryFunctor<T, typename T::Point, typename T::Value>;
} && CHasDimension<typename T::Point>;

} // namespace concepts
} // namespace DGtalTrace