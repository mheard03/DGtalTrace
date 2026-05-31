#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include "concepts/CHasFixedSize.cpp"
#include "concepts/CIsInitializerList.cpp"

namespace DGtalTrace {

template <typename T>
requires concepts::CHasFixedSize<T> || concepts::CIsInitializerList<T>
constexpr DGtal::Dimension SizeOf = [] {
  if constexpr      (CHasFixedSize<T>)
    return std::tuple_size_v<T>;
  else if constexpr (concepts::detail::CIsInitializerList<T>)
    return T::size();
  else
     return 0;
}();

namespace {
  auto a = {1,2,3};
  static_assert((CHasFixedSize<std::array<int, 3>> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 3> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 2> == false));

  auto a = {1, 2, 3};
  typedef decltype(a) il;
  static_assert((CHasFixedSize<il> == true));
  static_assert((CHasFixedSize<KSpace::Cell> == true));
}


} // namespace DGtalTrace