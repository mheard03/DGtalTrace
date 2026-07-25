#pragma once
#include <concepts>
#include <DGtal/base/Assert.h>
#include <DGtal/base/CLabel.h>
#include "../common/CBool.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename T>
concept CLabelImpl = std::default_initializable<T> && std::assignable_from<T&, T> && std::equality_comparable<T>;

} // namespace detail

template <typename T, typename = void>
struct CLabel : std::false_type {};

template <typename T>
requires detail::CLabelImpl<T>
struct CLabel<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CLabel<T>));
};

} // namespace bridges

template <typename T>
concept CLabel = requires {
  typename bridges::CLabel<T>;
  requires CIsTrue<bridges::CLabel<T>>;
};

namespace {
  static_assert(CLabel<int> == true);
  static_assert(CLabel<std::reference_wrapper<int>> == false);
}

}
}