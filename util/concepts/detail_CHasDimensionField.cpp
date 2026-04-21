#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>

namespace DGtalTrace {
namespace concepts {
namespace detail {
template <typename T>
concept CHasDimensionField = 
    requires { { T::dimension        } -> std::convertible_to<DGtal::Dimension>; } ||
    requires { { T::Space::dimension } -> std::convertible_to<DGtal::Dimension>; } ||
    requires { { T::Point::dimension } -> std::convertible_to<DGtal::Dimension>; };

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