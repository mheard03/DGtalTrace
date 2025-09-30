#pragma once
#include <type_traits>
#include <ostream>

template<typename T, class = void>
struct is_container : public std::false_type {};

template<typename T>
struct is_container<T, std::void_t<typename T::iterator>> : public std::true_type {};

template<>
struct is_container<std::string, void> : public std::false_type {};

template<class... B>
constexpr bool is_container_v = is_container<B...>::value;

template <typename T, typename = std::enable_if_t<is_container_v<T>>>
std::ostream& operator<<(std::ostream& os, const T& collection) {
  os << "[ ";
  auto size = collection.size();

  if (size < 13) {
    for (auto it = collection.begin(); it != collection.end(); ++it) {
      os << *it;
      if (it != collection.end()) os << ", ";
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