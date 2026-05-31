#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include "concepts/BaseConcepts.cpp"
#include "concepts/detail_CHasDimensionField.cpp"

namespace DGtalTrace {

namespace {
/*
template <typename T>
concept CHasFixedSize = requires {
  typename T::value_type;
  typename std::tuple_size<T>::type;
  { std::tuple_size_v<T> } -> std::convertible_to<std::size_t>;  
  std::convertible_to<typename T::size_type, std::size_t>;
};
*/

template <typename T>
concept CHasPointField = requires {
  typename T::Point;
  requires concepts::CPoint<typename T::Point>;
};

template <CHasPointField T>
struct PointField_ {
  typedef T::Point Point;
};



} // namespace detail

namespace {
  using namespace DGtal::Z2i;

  static_assert((detail::CHasDimensionField<bool> == false));
  static_assert((detail::CHasDimensionField<Point> == true));
  static_assert((detail::CHasDimensionField<Domain> == true));
  static_assert((detail::CHasDimensionField<KSpace::Cell> == true));
}

} // namespace concepts
} // namespace DGtalTrace

}

template <typename T>
requires concepts::detail::CHasDimensionField<T> || CHasFixedSize<T>
constexpr DGtal::Dimension DimensionOf = [] {
  if constexpr      (CHasFixedSize<T>)
    return std::tuple_size_v<T>;
  else if constexpr (requires { { T::dimension } -> std::convertible_to<DGtal::Dimension>; })
    return T::dimension;
  else if constexpr (requires { { T::Point::dimension } -> std::convertible_to<DGtal::Dimension>; })
    return T::Point::dimension;
  else if constexpr (requires { { T::Space::dimension } -> std::convertible_to<DGtal::Dimension>; })
    return T::Space::dimension;
  else
     return 0;
}();

template <typename T, typename U>
requires concepts::detail::CHasDimensionField<T> && concepts::detail::CHasDimensionField<U>
constexpr bool HasSameDimension = DimensionOf<T> == DimensionOf<U>;

} // namespace DGtalTrace