#pragma once
#include <array>
#include <cstddef>
#include <iostream>
#include <initializer_list>

namespace DGtalTrace { namespace Offsets {

#define FOR_ANGLES_INLINE(X, ...) X(Rotate0, 0, __VA_ARGS__), X(Rotate90, 90, __VA_ARGS__), X(Rotate180, 180, __VA_ARGS__), X(Rotate270, 270, __VA_ARGS__)
#define FOR_ANGLES(X, ...) X(Rotate0, 0, __VA_ARGS__) \
  X(Rotate90, 90, __VA_ARGS__) \
  X(Rotate180, 180, __VA_ARGS__) \
  X(Rotate270, 270, __VA_ARGS__)

enum class CWAngles : size_t { 
  #define X(name, value, ...) name = value,
  FOR_ANGLES(X, ...)
  #undef X
};

template <typename T>
struct CWAnglesArray : public std::array<T, 4> {
 protected:
  typedef std::array<T, 4> Base;

 public:
  using Base::Base;
  constexpr CWAnglesArray(std::initializer_list<T> list) : Base(list) {};

  // Public accessors.
  constexpr T& operator[] (CWAngles val) {
    return Base::operator[](static_cast<size_t>(val) / 90);
  }
  constexpr T const& operator[] (CWAngles val) const {
    return Base::operator[](static_cast<size_t>(val) / 90);
  }
};

} } // end namespace

std::ostream& operator<<(std::ostream& os, const DGtalTrace::Offsets::CWAngles& angle) {
  typedef DGtalTrace::Offsets::CWAngles Angles;
  #define X(enum, value, ...) case(Angles::enum): os << #enum; break;
  switch(angle) {
    FOR_ANGLES(X)
  }
  #undef X
  return os;
}
