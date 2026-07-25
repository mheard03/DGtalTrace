#pragma once
#include <array>
#include <concepts>
#include <initializer_list>
#include <DGtal/base/Common.h>

namespace DGtalTrace {
namespace concepts {

template <typename T, DGtal::Dimension dim = 0>
concept CIsInitializerList = requires {
  typename T::value_type;
  requires (std::integral<typename T::value_type> || std::floating_point<typename T::value_type>);
  requires std::same_as<T, std::initializer_list<typename T::value_type>>;
  requires requires { T::size() == 0; };
  requires requires { (dim == 0 || T::size() == dim); };
};

namespace {
  auto a = {1, 2, 3};
  auto b = {"a", "b", "c"};

  typedef decltype(a) T;
  auto result = std::to_array({1,2,3});

  auto s = T::size();
  constexpr bool test = CIsInitializerList<decltype(a)>;

  static_assert((CIsInitializerList<decltype(a)> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 3> == true));
  static_assert((CHasFixedSize<std::array<int, 3>, 2> == false));
}

} // namespace concepts
} // namespace DGtalTrace