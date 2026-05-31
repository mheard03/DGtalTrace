#pragma once
#include <ranges>
#include <vector>

namespace DGtalTrace {
namespace concepts {
template <typename T>
concept CAsRangeable = requires (T t) {
  asRange(t);
};
}

template <std::ranges::sized_range TList>
static TList asRange(const TList& values) { //} requires (!std::same_as<std::remove_cv_t<TList>, std::string>) {
  return values;
}
template <typename TValue>
std::vector<TValue> asRange(const std::initializer_list<TValue>& values) {
  return std::vector<TValue>{values};
}
std::vector<std::string> asRange(const std::string& value) {
  return {value};
}
template <typename TValue>
std::vector<TValue> asRange(const TValue& value) requires (!std::ranges::range<TValue>) {
  return {value};
}

bool isArray = std::ranges::range<std::array<int, 1>>;
typedef std::ranges::range_value_t<std::array<int, 1>> art;


auto result1 = asRange(std::array<int, 1>{3});
auto result2 = asRange({3});
auto result3 = asRange(3);
auto result4 = asRange("potato");

}