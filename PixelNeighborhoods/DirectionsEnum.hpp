#pragma once
#include <array>
#include <cstddef>


#define FOR_DIRECTIONS_INLINE(X, ...) X(NW, nw, __VA_ARGS__), X(N, n, __VA_ARGS__), X(NE, ne, __VA_ARGS__), X(W, w, __VA_ARGS__), X(C, c, __VA_ARGS__), X(E, e, __VA_ARGS__), X(SW, sw, __VA_ARGS__), X(S, s, __VA_ARGS__), X(SE, se, __VA_ARGS__)
#define FOR_DIRECTIONS(X...) X(NW, nw, __VA_ARGS__) \
  X(N, n, __VA_ARGS__) \
  X(NE, ne, __VA_ARGS__) \
  X(W, w, __VA_ARGS__) \
  X(C, c, __VA_ARGS__) \
  X(E, e, __VA_ARGS__) \
  X(SW, sw, __VA_ARGS__) \
  X(S, s, __VA_ARGS__) \
  X(SE, se, __VA_ARGS__)

namespace DGtalTrace { namespace Offsets {

enum class Directions : size_t {
  #define X(name, ...) name
  FOR_DIRECTIONS_INLINE(X, ...)
  #undef X
};

template <typename T>
struct DirectionsArray : public std::array<T, 9> {
 protected:
  typedef std::array<T, 9> Base;

 public:
  using Base::Base;
  constexpr DirectionsArray(std::initializer_list<T> list) : Base(list) {};

  // Public accessors.
  constexpr T& operator[] (Directions val) {
    return Base::operator[](static_cast<size_t>(val));
  }
  constexpr T const& operator[] (Directions val) const {
    return Base::operator[](static_cast<size_t>(val));
  }
};

} } // end namespace
