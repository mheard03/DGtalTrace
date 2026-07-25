#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/NumberTraits.h>
#include "BaseConcepts.cpp"
#include "../util/DimensionOf.cpp"

namespace DGtalTrace {

namespace concepts {
  
template <typename T>
concept CCoordTraits = requires {
  typename T::Component;
  requires std::integral<typename T::Component> || std::floating_point<typename T::Component>;

  T::dimension;
  requires DGtal::NumberTraits<decltype(T::dimension)>::IsUnsigned;
  requires DGtal::NumberTraits<decltype(T::dimension)>::IsIntegral;

  typename T::Point;
  requires CPoint<typename T::Point>;
  
  typename T::Vector;
  std::same_as<typename T::Point, typename T::Vector>;
};

template <typename T>
struct CoordTraits;

template <typename T>
concept CHasCoordTraits = requires {
  sizeof(CoordTraits<T>);
};

} // concepts
} // DGtalTrace