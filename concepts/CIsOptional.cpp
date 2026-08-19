#pragma once
#include <optional>

namespace DGtalTrace {
namespace concepts {

namespace {
  template <typename T> struct is_optional : std::false_type {};
  template <typename T> struct is_optional<std::optional<T>> : std::true_type {};
} // end anonymous namespace

template <typename T> concept CIsOptional = is_optional<std::remove_cvref_t<T>>::value;

} // end namespace concepts
} // end namespace DGtalTrace