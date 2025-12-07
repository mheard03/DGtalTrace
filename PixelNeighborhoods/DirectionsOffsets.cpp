#pragma once
#include <array>
#include <cinttypes>
#include <cstdint>
#include "../CEVec.cpp"
#include "CWAnglesEnum.hpp"
#include "DirectionsEnum.hpp"
#include <type_traits>

namespace DGtalTrace { namespace Offsets {

template <typename Enum, typename T, size_t N>
struct EnumArray : public std::array<T, N> {
 protected:
  typedef std::array<T, N> Base;

 public:
  using Base::Base;

  // Public accessors.
  constexpr T& operator[] (Enum val) {
    return Base::operator[](static_cast<size_t>(val));
  }
  constexpr T const& operator[] (Enum val) const {
    return Base::operator[](static_cast<size_t>(val));
  }
};

namespace Impl {

constexpr const CEVec2 rotate(const CWAngles angle, const CEVec2::Container& coords) {
  const auto [x, y] = coords;
  if (angle == CWAngles::Rotate0) return CEVec2(x, y);
  if (angle == CWAngles::Rotate90) return CEVec2(-y, x);
  if (angle == CWAngles::Rotate180) return CEVec2(-x, -y);
  if (angle == CWAngles::Rotate270) return CEVec2(y, -x);
  throw "impossibru";
}

constexpr const DirectionsArray<CEVec2> makeDirArray(CEVec2 dW, CEVec2 dN) {
  DirectionsArray<CEVec2> result;
  CEVec2 dE = dW * -1;
  CEVec2 dS = dN * -1;  
  result[Directions::NW] = dN + dW;
  result[Directions::N] = dN;
  result[Directions::NE] = dN + dE;
  result[Directions::W] = dW;
  result[Directions::C] = CEVec2(0, 0);
  result[Directions::E] = dE;
  result[Directions::SW] = dS + dW;
  result[Directions::S] = dS;
  result[Directions::SE] = dS + dE;
  return result;
}

constexpr const CWAnglesArray<DirectionsArray<CEVec2>> makeDirArrays() {
  constexpr CEVec2 dW = CEVec2(-1, 0);
  constexpr CEVec2 dN = CEVec2(0, -1);

  CWAnglesArray<DirectionsArray<CEVec2>> result;
  #define X(angle, ...) result[CWAngles::angle] = makeDirArray(rotate(CWAngles::angle, dW), rotate(CWAngles::angle, dN));
  FOR_ANGLES(X, ...)
  #undef X

  return result;
}

}
constexpr CWAnglesArray<DirectionsArray<CEVec2>> DirectionsOffsets = Impl::makeDirArrays();

} } // end namespace
/*
struct Offsets {  

 public:

  CEVec2 rotate(const CEVec2::Container& coords) {
    const auto [x, y] = coords;
    if (Angle == CWAngles::Rotate0) return CEVec2(x, y);
    if (Angle == CWAngles::Rotate90) return CEVec2(-y, x);
    if (Angle == CWAngles::Rotate180) return CEVec2(-x, -y);
    if (Angle == CWAngles::Rotate270) return CEVec2(y, -x);
    throw "impossibru";
  }

  static constexpr CEVec2 offset(CEVec2 dir) { return rotate(dir); }
  static constexpr CEVec2 dC = CEVec2(0, 0);
  static constexpr CEVec2 dN = rotate({0, -1});
  static constexpr CEVec2 dW = rotate({-1, 0});
  static constexpr CEVec2 dS = dN * -1;
  static constexpr CEVec2 dE = dW * -1;
  static constexpr CEVec2 dNW = dN + dW;
  static constexpr CEVec2 dNE = dN + dE;
  static constexpr CEVec2 dSW = dS + dW;
  static constexpr CEVec2 dSE = dS + dE;    
};

template <class OffsetsImpl, CWAngles Angle>
struct Offsets {
  static inline constexpr CEVec2 offset(CEVec2 dir) { return StaticOffsets<Angle>::rotate(dir); }
  #define X(name, ...) static inline constexpr CEVec2 name() { return StaticOffsets<Angle>::d ## name; }
  FOR_DIRECTIONS(X)
  #undef X
};

#define X(Angle, angleNum, ...) struct Offsets ## angleNum : Offsets<Offsets ## angleNum, Angle> {};
FOR_ANGLES(X);
#undef X

}

typedef magic_enum::containers::array<CWAngles, Offsets> DirOffsetsByAngle;
*/
/*
  namespace Impl {
    using namespace magic_enum::bitwise_operators;
    typedef magic_enum::containers::array<Directions, CEVec2> DirOffsets;
    typedef magic_enum::containers::array<CWAngles, DirOffsets> DirOffsetsByAngle;

    constexpr const CEVec2 rotate(const CWAngles angle, const CEVec2::Container& coords) {
      const auto [x, y] = coords;
      if (angle == CWAngles::Rotate0) return CEVec2(x, y);
      if (angle == CWAngles::Rotate90) return CEVec2(-y, x);
      if (angle == CWAngles::Rotate180) return CEVec2(-x, -y);
      if (angle == CWAngles::Rotate270) return CEVec2(y, -x);
      throw "impossibru";
    }

    constexpr const DirOffsets getDirOffsets(const CWAngles angle) {
      const CEVec2 x = rotate(angle, {1, 0});
      const CEVec2 y = rotate(angle, {0, 1});

      DirOffsets result;
      for (auto dir : magic_enum::enum_values<Directions>()) {
        CEVec2 value(0, 0);
        if ((dir & Directions::XPos) == Directions::XPos) value += x;
        if ((dir & Directions::XNeg) == Directions::XNeg) value -= x;
        if ((dir & Directions::YPos) == Directions::YPos) value += y;
        if ((dir & Directions::YNeg) == Directions::YNeg) value -= y;
        result[dir] = value;
      }

      return result;
    }

    constexpr const DirOffsetsByAngle getDirOffsetsByAngle() {
      DirOffsetsByAngle result;

      for (auto angle : magic_enum::enum_values<CWAngles>()) {
        result[angle] = getDirOffsets(angle);
      }

      return result;
    }
  }

  constexpr Impl::DirOffsetsByAngle StaticOffsets = Impl::getDirOffsetsByAngle();

  auto vvv = StaticOffsets[CWAngles::Rotate0][Directions::N];
*/




