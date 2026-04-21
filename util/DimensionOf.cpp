#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "concepts/detail_CHasDimensionField.cpp"

namespace DGtalTrace {

template <typename T>
requires concepts::detail::CHasDimensionField<T>
constexpr DGtal::Dimension DimensionOf = [] {
  if constexpr      (requires { { T::dimension } -> std::convertible_to<DGtal::Dimension>; })
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