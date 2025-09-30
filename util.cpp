#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/images/Image.h>
#include <DGtal/kernel/PointVector.h>
#include <type_traits>
#include <iostream>
#include <vector>
#include <functional>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/images/ImageContainerBySTLVector.h>

namespace util {
  namespace detail {
    // is_container
    template<typename T, class = void>
    struct is_container : std::false_type {};

    template<typename T>
    struct is_container<T, std::void_t<typename T::iterator, typename T::value_type>> : std::true_type {};

    template<>
    struct is_container<std::string, void> : std::false_type {};

    template<class... B>
    constexpr bool is_container_v = is_container<T>::value;


    // inner_type
    template<class T, typename = void>
    struct inner_type {
      using type = T;
    };

    template<class T>
    struct inner_type<T, std::void_t<typename T::value_type>> : inner_type<typename T::value_type> {};

    template<class T>
    using inner_type_t = typename inner_type<T>::type;


    // has_selfDisplay
    template<class T, typename = void>
    struct has_selfDisplay : std::false_type {};

    template<class T>
    struct has_selfDisplay<T,
        std::void_t<decltype(std::declval<T>().selfDisplay(std::declval<std::ostream&>()))>
    > : std::true_type {};

    template<class T>
    constexpr bool has_selfDisplay_v = has_selfDisplay<T>::value;


    // Detection idiom for operator<<
    template <typename T, typename = void>
    struct has_free_ostream_operator : std::false_type {};

    template <typename T>
    struct has_free_ostream_operator<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>> : std::true_type {};

    template<typename T>
    constexpr bool has_free_ostream_operator_v = has_free_ostream_operator<T>::value;


    // can_selfDisplay
    template<typename T, typename = void, typename = void>
    struct can_selfDisplay : std::false_type {};

    template<typename T>
    struct can_selfDisplay<T, std::enable_if_t<has_selfDisplay_v<T> | has_free_ostream_operator_v<T>>> : std::true_type {};

    template<typename T>
    constexpr bool can_selfDisplay_v = can_selfDisplay<T>::value;
  }
}

std::ostream& operator<< (std::ostream& aOs, ProbingMode const& aMode)
 std::ostream& operator<< ( std::ostream & out, const CColor & aColor );

  template <typename T, std::enable_if_t<!util::detail::has_free_ostream_operator_v<T>>>
  std::ostream& operator<<(std::ostream& os, const T& obj) {
    obj.selfDisplay(os);
    return os;
  }

/*
  template <typename T, std::enable_if_t<util::detail::has_selfDisplay_v<const T> & !util::detail::has_free_ostream_operator_v<const T>>>
  std::ostream& operator<<(std::ostream& os, const T& obj) {
    obj.selfDisplay(os);
    return os;
  }

  template <typename T, std::enable_if_t<util::detail::has_selfDisplay_v<T> & !util::detail::has_free_ostream_operator_v<T>>>
  std::ostream& operator<<(std::ostream& os, T& obj) {
    obj.selfDisplay(os);
    return os;
  }

  /*
  template <typename T, std::enable_if_t<detail::is_container_v<T> & detail::can_selfDisplay_v<T>>
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
  };


  template <typename T, std::enable_if_t<detail::has_selfDisplay_v<T>>>
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
  };
  */
