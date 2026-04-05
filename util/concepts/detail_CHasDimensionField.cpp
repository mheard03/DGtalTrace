#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>

namespace DGtalTrace {
namespace concepts {
namespace detail {
template <typename T>
concept CHasDimensionField = requires {
  requires
    requires { { T::dimension        } -> std::convertible_to<DGtal::Dimension>; } ||
    requires { { T::Space::dimension } -> std::convertible_to<DGtal::Dimension>; } ||
    requires { { T::Point::dimension } -> std::convertible_to<DGtal::Dimension>; };
};
} // namespace detail
} // namespace concepts
} // namespace DGtalTrace