#pragma once
#include <type_traits>
#include <vector>
#include <ostream>

template<typename T, class = void>
struct is_container : public std::false_type {};

template<typename T>
struct is_container<T, std::void_t<typename T::iterator>> : public std::true_type {};

template<>
struct is_container<std::string, void> : public std::false_type {};

template<class... B>
constexpr bool is_container_v = is_container<B...>::value;

namespace {
  std::vector<int> values;
  auto i = values.begin();
}


template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair) {
  os << "{ " << pair.first << ", " << pair.second << " }";
  return os; 
}

template <typename T, typename = std::enable_if_t<is_container_v<T>>>
std::ostream& operator<<(std::ostream& os, const T& collection) {
  os << "[ ";
  auto size = collection.size();

  if (size < 13) {
    for (auto it = collection.begin(), next_it = it; it != collection.end(); it = next_it) {
      os << *it;
      next_it = ++it;
      if (next_it != collection.end()) os << ", ";
    }
  }
  else {
    auto it_end = std::next(collection.begin(), 7); 
    for (auto it = collection.begin(); it != it_end; ++it) {
      os << *it << ", ";
    }
    os << "<" << (size - 10) << " more...>";
    auto it = collection.begin();
    std::advance(it, collection.size() - 3);
    for (; it != collection.end(); ++it) {
      os << ", " << *it;
    }
  }
  os << " ]";
  return os;
}