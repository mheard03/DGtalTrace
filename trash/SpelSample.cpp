#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <sys/types.h>
#include <type_traits>

template <typename T, size_t N, size_t Radius>
class SampleArray;

template <typename T, typename = void>
struct get_array_depth {
  static constexpr int value = std::is_array_v<T>0;
}

template <typename T, size_t N, size_t Radius>
class SampleArray : public std::array<T, N> {
 protected:
  static constexpr bool is_sample_array = true;
  typedef std::array<T, N> Base;
  friend get_depth<T>;
 public:
  static constexpr size_t depth = 0;

};

typedef SampleArray<int, 2, 1> sa;
typedef sa::size_type sast;




/*
  using Base::Base;
  T& operator[](size_t index) {
    return data[index];
  }
};

template<typename T, typename TSpace, size_t Radius>
struct dimensions : std::integral_constant<std::size_t, 0> {};

template<typename T>
struct dimensions<std::vector<T>> : std::integral_constant<std::size_t, 1 + dimensions<T>::value> {};

template<typename T>
inline constexpr std::size_t dimensions_v = dimensions<T>::value; // (C++17)


/*
  typedef typename Base::size_type size_type;
  typedef typename Base::value_type value_type;
  typedef typename Base::difference_type difference_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::pointer pointer;
  typedef typename Base::const_pointer const_pointer;
  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
template <typename T, class = void>
struct make_bigsigned {
  typedef type
}
std::enable_if<std::is_integral_v<T>>>
struct make_bigsigned {
 protected:
  typedef std::tuple<int16_t, int16_t, int32_t, int64_t, int64_t, std::ptrdiff_t> typeTuple;
  static_assert(std::is_integral_v<T>, "SignedType: T must be an integral type");
  static constexpr bool is_signed = std::is_signed_v<T>; 
  static constexpr auto width = std::numeric_limits<T>::digits / 8;  
};

make_bigsigned_base<int> t;


template <typename T, bool allow_overflow> struct make_bigsigned {};

template <typename T, bool allow_overflow = true> struct make_bigsigned { 
 private:
  // signed type required to hold the maximum value of an unsigned 0, 8, 16, 24, and 32-bit integer
  // ptrdiff_t is provided as a fallback when allow_overflow = true
  typedef std::tuple<int16_t, int16_t, int32_t, int64_t, int64_t, std::ptrdiff_t> typeTuple;
 
  static_assert(std::is_integral_v<T>, "SignedType: T must be an integral type");
  static constexpr bool is_signed = std::is_signed_v<T>; 
  static constexpr auto width = std::numeric_limits<T>::digits / 8;

  static_assert(allow_overflow || is_signed || width <= 4, "SignedType doesn't work with sizes exceeding 32 bits");
  static constexpr auto index = width > 5 ? 5 : width;

 public:
  using type = typename std::conditional_t<is_signed, T, std::tuple_element<index, typeTuple>>;
};

template <typename T>
using bigsigned_t = make_bigsigned<typename Base::size_type>;
*/