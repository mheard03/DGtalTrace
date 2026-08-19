#pragma once
#include <optional>

namespace DGtalTrace {
namespace detail {

namespace {
  template <typename T>
  struct unwrap_optional_type {
      using type = T;
  };

  template <typename T>
  struct unwrap_optional_type<std::optional<T>> {
      using type = typename unwrap_optional_type<T>::type;
  };
}

template <typename T>
using UnwrapOptValue = typename unwrap_optional_type<T>::type;

template <typename T>
using OptValue = std::optional<UnwrapOptValue<T>>;



} // end namespace detail
} // end namespace DGtalTrace