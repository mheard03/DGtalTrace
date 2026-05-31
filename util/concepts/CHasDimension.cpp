#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include "../DimensionOf.cpp"

namespace DGtalTrace {
namespace concepts {

template <typename T, DGtal::Dimension dim = 0>
concept CHasDimension = requires { 
  requires detail::CHasDimensionField<T>;
  requires (dim == 0 || DimensionOf<T> == dim);
};

namespace {
  /*
  using namespace DGtal::Z2i;

  static_assert((CHasDimension<Point> == true));
  static_assert((CHasDimension<Point, 2> == true));
  static_assert((CHasDimension<Point, 3> == false));

  static_assert((CHasDimension<Domain> == true));
  static_assert((CHasDimension<KSpace::Cell> == true));
  */
}

} // namespace concepts
} // namespace DGtalTrace