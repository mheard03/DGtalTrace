#pragma once
#include <concepts>
#include <DGtal/base/Common.h>

namespace DGtalTrace {
namespace concepts {

template <typename T, DGtal::Dimension dim = 0>
concept CHasFixedSize = requires {
  typename T::value_type;
  typename std::tuple_size<T>::type;
  { std::tuple_size_v<T> } -> std::convertible_to<std::size_t>;  
  std::convertible_to<typename T::size_type, std::size_t>;
  requires (dim == 0 || std::tuple_size_v<T> == dim);
};

namespace {
  static_assert((CHasFixedSize<std::array<int, 3>> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 3> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 2> == false));
}

} // namespace concepts
} // namespace DGtalTrace