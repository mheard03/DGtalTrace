#pragma once
#include <concepts>
#include <type_traits>

namespace DGtalTrace {
namespace concepts {

template <bool T>
concept CBool = T;

namespace {
template <typename T>
concept CHasBoolValue = requires { 
  T::value;
  CBool<T::value>;
};
}

template <typename T>
concept CIsTrue = std::same_as<T, std::true_type> || (CHasBoolValue<T> && CBool<T::value>);

template <typename T>
concept CIsFalse = std::same_as<T, std::false_type> || (CHasBoolValue<T> && !CBool<T::value>);

}
}