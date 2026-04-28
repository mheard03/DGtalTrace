#pragma once
#include <concepts>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <type_traits>
#include "../DimensionOf.cpp"

namespace DGtalTrace {
namespace concepts {
namespace detail {
  template <typename T, typename U>
  concept IsBasically = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
} // namespace detail

/*
template <typename T, DGtal::Dimension dim = 0>
concept CPoint = requires { 
  requires DGtal::IsAPointVector<T>::value == true; 
  requires (dim == 0 || DimensionOf<T> == dim);
};
*/
template <typename T>
concept CIsTrue = std::same_as<T, std::true_type> || (requires { T::value; } && T::value);

template <typename T>
concept CIsFalse = std::same_as<T, std::false_type> || (requires { T::value; std::convertible_to<T, bool>; } && !T::value);


template <typename T, DGtal::Dimension dim = 0>
concept CPoint = CIsTrue<DGtal::IsAPointVector<T>> && requires { requires (dim == 0 || DimensionOf<T> == dim); };



namespace {
  using namespace DGtal::Z2i;

  static_assert((detail::IsBasically<int, int> == true));
  static_assert((detail::IsBasically<int, const int> == true));
  static_assert((detail::IsBasically<int, int&> == true));
  static_assert((detail::IsBasically<int, uint> == false));

  static_assert((CPoint<Point> == true));
  static_assert((CPoint<Point, 2> == true));
  static_assert((CPoint<Point, 3> == false));
  static_assert((CPoint<typename KSpace::Cell> == false));

  static_assert((CIsTrue<std::string> == false));
  static_assert((CIsTrue<std::true_type> == true));
  static_assert((CIsTrue<std::false_type> == false));

  static_assert((CIsFalse<std::string> == false));
  static_assert((CIsFalse<std::false_type> == true));
  static_assert((CIsFalse<std::true_type> == false));
  
}

} // namespace concepts
} // namespace DGtalTrace