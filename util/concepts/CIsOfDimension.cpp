#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "../DimensionOf.cpp"

namespace DGtalTrace {
namespace concepts {

template <typename T, DGtal::Dimension dim = 0> 
concept CHasDimension = requires { 
  detail::CHasDimensionField<T>;
  dim == 0 || DimensionOf<T> == dim;
};


} // namespace concepts
} // namespace DGtalTrace