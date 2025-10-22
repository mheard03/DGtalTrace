#pragma once
#include <DGtal/base/Common.h>
#include <cstdarg>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <unistd.h>
#include <vector>
#include <numeric>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include "traceHelper.cpp"
#include "CEVec.cpp"

/*
struct CEVec2 : DGtal::Z2i::KSpace::Vector::Container {
  typedef typename DGtal::Z2i::KSpace KSpace;
  typedef typename KSpace::Vector PointVector;
  typedef typename KSpace::Vector::Container Container;
  typedef typename KSpace::Vector::Component Component;
  static constexpr DGtal::Dimension Size = KSpace::dimension;  

  constexpr CEVec2() : Container() {}
  constexpr CEVec2(const Component x, const Component y) : Container({ x, y }) {}
  constexpr CEVec2(const Container& coords) : Container(coords) {}
  constexpr CEVec2(const PointVector& pointVector) : Container({pointVector[0], pointVector[1]}) {}
  constexpr CEVec2(Container&& coords) : Container(std::move(coords)) {}

  operator const DGtal::PointVector<Size, Component>() const { return DGtal::PointVector<Size, Component>(this->operator[](0), this->operator[](1)); }

  constexpr CEVec2& operator+=(const CEVec2& rhs) {
    for (DGtal::Dimension i = 0; i < Size; i++) {
      this->operator[](i) += rhs[i];
    }
    return *this;
  }
  friend constexpr CEVec2 operator+(CEVec2 lhs, const CEVec2& rhs) {
    lhs += rhs;
    return lhs;
  }

  constexpr CEVec2& operator-=(const CEVec2& rhs) {
    for (DGtal::Dimension i = 0; i < Size; i++) {
      this->operator[](i) -= rhs[i];
    }
    return *this;
  }
  friend constexpr CEVec2 operator-(CEVec2 lhs, const CEVec2& rhs) {
    lhs -= rhs;
    return lhs;
  }

  constexpr CEVec2& operator*=(const Component scalar) {
    for (DGtal::Dimension i = 0; i < Size; i++) {
      this->operator[](i) *= scalar;
    }
    return *this;
  }
  friend constexpr CEVec2 operator*(CEVec2 lhs, const Component scalar) {
    lhs *= scalar;
    return lhs;
  }
};
*/

template <int Angle = 0>
struct Rotation {
 protected:
  typedef typename CEVec2::Component Component;
  static_assert(Angle == 0 || Angle == 90 || Angle == 180 || Angle == 270);

 public:
  static inline constexpr CEVec2 apply(const CEVec2::Container coords) {
    const auto [x, y] = coords;
    if constexpr (Angle == 0) return CEVec2(x, y);
    if constexpr (Angle == 90) return CEVec2(y, -x);
    if constexpr (Angle == 180) return CEVec2(-x, -y);
    if constexpr (Angle == 270) return CEVec2(-y, x);
  }
};

template <int angle = 0, int scale = 1>
struct Direction {
 private:
  static constexpr auto r = Rotation<angle>::apply;

 public:
  static constexpr const CEVec2 NW = r({-1, -1}) * scale;
  static constexpr const CEVec2 N =  r({+0, -1}) * scale;
  static constexpr const CEVec2 NE = r({+1, -1}) * scale;
  static constexpr const CEVec2 W =  r({-1, +0}) * scale;
  static constexpr const CEVec2 C =  r({+0, +0}) * scale;
  static constexpr const CEVec2 E =  r({+1, +0}) * scale;
  static constexpr const CEVec2 SW = r({-1, +1}) * scale;
  static constexpr const CEVec2 S =  r({+0, +1}) * scale;
  static constexpr const CEVec2 SE = r({+1, +1}) * scale;
  static constexpr const std::array<const CEVec2*, 9> all = {&NW, &N, &NE, &W, &C, &E, &SW, &S, &SE};
};

struct Directions {
  typedef Direction<0> D0;
  typedef Direction<90> DCW;
  typedef Direction<180> D180;
  typedef Direction<270> DCCW;
};

template <typename T, typename = void>
struct is_cell : std::false_type {};

template <typename T>
struct is_cell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::Cell>,
  std::is_same<T, typename T::CellularGridSpace::SCell>,
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_precell : std::false_type {};

template <typename T>
struct is_precell<T, std::void_t<typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

/*
struct Offsetter {};

template <typename KSpace = DGtal::Z2i::KSpace>
struct OffsetterBase {
 protected:
  inline CEVec2 apply(const CEVec2& source, const CEVec2& dir, const KSpace& K) {
    return source + dir;
  }
  inline typename K::Cell apply(const K::Cell& source, const CEVec2& dir, const KSpace& K) {
    ASSERT(K::uDim(source) == K::dimension);
    return K::uSpel(dir + K::uCoords(source));
  }
  inline typename K::SCell apply(const K::SCell& source, const CEVec2& dir, const KSpace& K) {
    ASSERT(K::sDim(source) == K::dimension);
    return K::sSpel(dir + K::sCoords(source), K::sSign(source));
  }
};

 
  template <typename T>
  static inline auto unwrapCoords(const T& cellOrPoint) {
    ASSERT(K::uDim(cellOrPoint) == K::dimension);
    if constexpr (is_cell<T>::value) return K::uCoords(cellOrPoint);
    return cellOrPoint;
  }
  template <typename T>
  static inline T wrapCoords(const T& cellOrPoint) {
    if constexpr (is_cell<T>::value) return K::uSpel(cellOrPoint);
    return cellOrPoint;
  }

*/

template <typename T, typename Dirs>
struct DirApplier {
 protected:
  virtual inline CEVec2 unwrap(const T& source) {
    if constexpr (std::is_convertible_v<T, CEVec2>) {
      return source;
    }
    throw "need to override DirApplier::unwrap when T is not convertible to CEVec2";
  }
  virtual inline T rewrap(const CEVec2& coords, const T& source) {
    if constexpr (std::is_convertible_v<CEVec2, T>) {
      return coords;
    }
    throw "need to override DirApplier::rewrap when CEVec2 is not convertible to T";
  }
  virtual inline T apply(const T& source, const CEVec2& dir) {
    return rewrap(unwrap(source) + dir, source);
  }

 public:
  inline T NW(const T& source) { return apply(source, Dirs::NW); };
  inline T N(const T& source)  { return apply(source, Dirs::N); };
  inline T NE(const T& source) { return apply(source, Dirs::NE); };
  inline T W(const T& source)  { return apply(source, Dirs::W); };
  inline T C(const T& source)  { return apply(source, Dirs::C); };
  inline T E(const T& source)  { return apply(source, Dirs::E); };
  inline T SW(const T& source) { return apply(source, Dirs::SW); };
  inline T S(const T& source)  { return apply(source, Dirs::S); };
  inline T SE(const T& source) { return apply(source, Dirs::SE); };
  std::array<T, 9> all(const T& source) {
    std::array<T, 9> result;
    for (size_t i = 0; i < 9; i++) {
      result[i] = apply(source, *Dirs::all[i]);
    }
    return result;
  };

  std::array<CEVec2, 2> getBounds(const T corner, const T oppositeCorner) {
    std::array<CEVec2, 2> bounds;
    return bounds;
  }
};

template <int Angle = 0, typename Dirs = Direction<Angle>>
struct PointOffsetter : public DirApplier<DGtal::Z2i::Point, Dirs> {
 /*
 protected:
  inline DGtal::Z2i::Point apply(const DGtal::Z2i::Point& source, const CEVec2& dir) override {
    return source + dir;
  }
    */
};


template <typename TCell, int Angle = 0, typename Dirs = Direction<Angle, 2>>
struct CellOffsetter : public DirApplier<TCell, Dirs> {
  
 private:
  typedef typename TCell::CellularGridSpace KSpace;
  typedef PointOffsetter<Angle, Dirs> PointOffsetter;
  static constexpr bool is_signed = std::is_same_v<TCell, typename KSpace::SCell>;
  
  static constexpr typename TCell::PreCellularGridSpace PK;
  const KSpace* K;

 protected:
   inline CEVec2 unwrap(const TCell& source) override {
    if constexpr (is_signed) return K->sKCoords(source);
    return K->uKCoords(source);
  }
  virtual inline TCell rewrap(const CEVec2& coords, const TCell& source) override {
    if constexpr (is_signed) return K->sCell(coords, K->sSign(source));
    return K->uCell(coords);
  }

  inline TCell apply(const TCell& source, const CEVec2& dir) override {
    if constexpr (is_signed) {
      return K->sCell(K->sKCoords(source) + dir, K->sSign(source));
    }
    return K->uCell(K->uKCoords(source) + dir);
  }

 public:
  CellOffsetter(const KSpace& K) : K(&K) {}
  
  template <typename U = TCell, typename std::enable_if_t<is_precell<U>::value, bool> = true>
  CellOffsetter() : K(&PK) {}
};