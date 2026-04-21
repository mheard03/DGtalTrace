#pragma once
#include <DGtal/base/BasicFunctors.h>
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include "CHasDimension.cpp"
#include "CCell.cpp"

namespace DGtalTrace {
namespace concepts {

template <typename X, typename A, typename R>
concept CUnaryFunctor =
    std::assignable_from<X &, const X &> &&
    requires(X x, std::add_lvalue_reference_t<A> a) {
      { x(a) } -> std::convertible_to<std::add_lvalue_reference_t<R>>;
    };

template <typename T>
concept CCellFunctor = requires {
  typename T::Cell;
  typename T::Quantity;
  CXXCell<typename T::Cell>;
  CUnaryFunctor<T, typename T::Point, typename T::Value>;
};

template <typename T>
concept CPointFunctor = requires {
  typename T::Point;
  typename T::Value;
  CPoint<typename T::Point>;
  CUnaryFunctor<T, typename T::Point, typename T::Value>;
};

} // namespace concepts
} // namespace DGtalTrace