#pragma once
#include <DGtal/base/Common.h>
#include <boost/move/detail/meta_utils.hpp>
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

#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>

enum class Angles : uint8_t { Rotate0, Rotate90, Rotate180, Rotate270 };

template <Angles CWAngle>
inline constexpr void rotate(CEVec2::Container& coords) {
  const auto [x, y] = coords;
  if constexpr (CWAngle == Angles::Rotate0) return;
  if constexpr (CWAngle == Angles::Rotate90) { coords[0] = -y; coords[1] = x; }
  if constexpr (CWAngle == Angles::Rotate180) { coords[0] = -x; coords[1] = -y; }
  if constexpr (CWAngle == Angles::Rotate270) { coords[0] = y; coords[1] = -x; }
}

template <Angles CWAngle>
inline constexpr const CEVec2 getRotated(const CEVec2::Container& coords) {
  const auto [x, y] = coords;
  if constexpr (CWAngle == Angles::Rotate0) return CEVec2(x, y);
  if constexpr (CWAngle == Angles::Rotate90) return CEVec2(-y, x);
  if constexpr (CWAngle == Angles::Rotate180) return CEVec2(-x, -y);
  if constexpr (CWAngle == Angles::Rotate270) return CEVec2(y, -x);
}

template <Angles CWAngle>
inline constexpr void offset(CEVec2& coords, const CEVec2& offset) {
  const CEVec2 rotOffset = getRotated<CWAngle>(offset);
  coords[0] += rotOffset[0];
  coords[1] += rotOffset[1];
}

template <Angles CWAngle>
inline constexpr const CEVec2::Container getOffset(const CEVec2& coords, const CEVec2& offset) {
  return getRotated<CWAngle>(offset) + coords;
}

template <Angles CWAngle>
constexpr const std::array<CEVec2, 2> getRotatedOffsets(const CEVec2& lowerOffset, const CEVec2& upperOffset) {
  if (lowerOffset[0] == lowerOffset[1] && lowerOffset == getRotated<Angles::Rotate180>(upperOffset)) {
    return { lowerOffset, upperOffset };
  }

  typedef CEVec2::Component val;
  val N = std::min(lowerOffset[1], upperOffset[1]);
  val W = std::min(lowerOffset[0], upperOffset[0]);
  val S = std::max(lowerOffset[1], upperOffset[1]);
  val E = std::max(lowerOffset[0], upperOffset[0]);
  
  std::array<CEVec2, 4> corners = { CEVec2(W, N), CEVec2(W, S), CEVec2(E, S), CEVec2(E, N) };

  size_t lo_i = static_cast<size_t>(CWAngle);
  size_t up_i = (lo_i + 2) % 4;
  return { getRotated<CWAngle>(corners[lo_i]), getRotated<CWAngle>(corners[up_i]) };
}

template <Angles CWAngle>
constexpr const std::array<CEVec2, 2> getRotatedBounds(const CEVec2& center, const CEVec2& lowerOffset, const CEVec2& upperOffset) {
  std::array<CEVec2, 2> result = getRotatedOffsets<CWAngle>(lowerOffset, upperOffset);
  result[0] += center;
  result[1] += center;
  return result;
}

template <Angles CWAngle>
struct CompassOffsets {
 private:
  static constexpr auto r = getRotated<CWAngle>;

 public:
  static constexpr const CEVec2 oNW = r({-1, -1});
  static constexpr const CEVec2 oN =  r({+0, -1});
  static constexpr const CEVec2 oNE = r({+1, -1});
  static constexpr const CEVec2 oW =  r({-1, +0});
  static constexpr const CEVec2 oC =  r({+0, +0});
  static constexpr const CEVec2 oE =  r({+1, +0});
  static constexpr const CEVec2 oSW = r({-1, +1});
  static constexpr const CEVec2 oS =  r({+0, +1});
  static constexpr const CEVec2 oSE = r({+1, +1});
};

using namespace DGtal;
using namespace DGtal::Z2i;

typedef ImageSelector<Domain, uint32_t>::Type RgbImage;

template <Angles CWAngle, int MarginN, int MarginE = MarginN, int MarginS = MarginN, int MarginW = MarginE>
struct Sampler {
 public:
  static constexpr const CEVec2 lowerOffset0 = CEVec2(-MarginW, -MarginN);
  static constexpr const CEVec2 upperOffset0 = CEVec2(MarginE, MarginS);
  const CEVec2 centerPoint;
  const std::array<CEVec2, 2> bounds;
  const RgbImage* image;
  const KSpace* K;
  typedef KPreSpace PK;

 public:
  static constexpr Angles cwAngle = CWAngle;

  Sampler(const CEVec2 centerPoint, const RgbImage& image, const KSpace& K) :
    centerPoint(centerPoint), 
    bounds(getRotatedBounds<CWAngle>(centerPoint, lowerOffset0, upperOffset0)),
    image(&image),
    K(&K) {};
  
  virtual bool isValid() const {
    return K->uIsInside(PK::uSpel(bounds[0])) && K->uIsInside(PK::uSpel(bounds[1]));
  }

  inline const CEVec2 getCoords(const CEVec2 offset) const {
    return K->uCoords(K->uSpel(centerPoint + getRotated<CWAngle>(offset)));    
  }

  const RgbImage::Value operator()(const CEVec2 offset) const {
    return image->operator()(getCoords(offset));
  }
};

template <Angles CWAngle>
struct NeighborSampler : public Sampler<CWAngle, 1> {
 private:
  typedef Sampler<CWAngle, 1> Base;
  typedef RgbImage::Value Value;
  static constexpr auto r = getRotated<CWAngle>;

 public:
  using Base::Base;

#define dir(name, dx, dy) \
 private: \
  static constexpr const CEVec2 name ## Offset = r({dx, dy}); \
 public: \
  inline const CEVec2 name ## Coords() const { return this->K->uCoords(this->K->uSpel(this->centerPoint + name ## Offset)); } \
  inline const Value  name() const { return this->image->operator()(name ## Coords()); }

  dir(nw, -1, -1);
  dir(n,  +0, -1);
  dir(ne, +1, -1);
  dir(w,  -1, +0);
  dir(c,  +0, +0);
  dir(e,  +1, +0);
  dir(sw, -1, +1);
  dir(s,  +0, +1);
  dir(se, +1, +1);
};

/*
template <Angles CWAngle, uint8_t MarginN, uint8_t MarginE = MarginN, uint8_t MarginS = MarginN, uint8_t MarginW = MarginE>
struct BoundedSampler : public SamplerBase<CWAngle> {
 protected:
  static constexpr const CEVec2 lowerOffset = { -(int)MarginW, -(int)MarginN };
  static constexpr const CEVec2 upperOffset = { (int)MarginE, (int)MarginS };

 public:
  using SamplerBase<CWAngle>::SamplerBase;
  const CEVec2 lowerBound = centerPoint + lowerOffset;
  const CEVec2 upperBound = centerPoint + upperOffset;

  virtual bool isValid() const override {
    return *this->K->isInside(lowerBound) && *this->K->isInside(upperBound);
  }
};

/*
struct RotationFunctions {
  static CEVec2 rotate(CEVec2::Container& coords) {
    return coords;
  }
  std::function<const CEVec2()

CEVec2 (*)(CEVec2::Container &) (aka CEVec<PointVector<2U, int>> (*)(std::array<int, 2> &))

  static auto _rotate = RotationFunctions::rotate;

  const int value = []() -> int { return 0; } ();
  // constexpr std::array<
};

inline constexpr CEVec2 rotate(const CEVec2::Container& coords, const Angles angle) {
  const auto [x, y] = coords;
  if constexpr (angle == Angles::Rotate0) return CEVec2(x, y);
  if constexpr (angle == Angles::Rotate90) return CEVec2(-y, x);
  if constexpr (angle == Angles::Rotate180) return CEVec2(-x, -y);
  if constexpr (angle == Angles::Rotate270) return CEVec2(y, -x);
}
*/

/*
template <Angles CWAngle = Angles::Rotate0>
struct CWRotation {
 protected:
  typedef typename CEVec2::Component Component;

 public:
  static constexpr const Angles cwAngle = CWAngle;
  static inline constexpr CEVec2 apply(const CEVec2::Container coords) {
    const auto [x, y] = coords;
    if constexpr (CWAngle == Angles::Rotate0) return CEVec2(x, y);
    if constexpr (CWAngle == Angles::Rotate90) return CEVec2(-y, x);
    if constexpr (CWAngle == Angles::Rotate180) return CEVec2(-x, -y);
    if constexpr (CWAngle == Angles::Rotate270) return CEVec2(y, -x);
  }
};

struct CWRotation2 {
 protected:
  typedef typename CEVec2::Component Component;

 public:
  static inline constexpr CEVec2 apply(const CEVec2::Container coords, const Angles angle) {
    const auto [x, y] = coords;
    if constexpr (angle == Angles::Rotate0) return CEVec2(x, y);
    if constexpr (angle == Angles::Rotate90) return CEVec2(-y, x);
    if constexpr (angle == Angles::Rotate180) return CEVec2(-x, -y);
    if constexpr (angle == Angles::Rotate270) return CEVec2(y, -x);
  }
};

struct Rotations {
  typedef CWRotation<Angles::Rotate0> None;
  typedef CWRotation<Angles::Rotate90> CW;
  typedef CWRotation<Angles::Rotate180> CW180;
  typedef CWRotation<Angles::Rotate270> CCW;
}

template <int CWAngle = 0>
std::array<CEVec2, 2> getRotatedBounds(const CEVec2& lowerOffset, const CEVec2& upperOffset) {
  typedef CEVec2::Component val;
  val N = std::min(lowerOffset[1], upperOffset[1]);
  val W = std::min(lowerOffset[0], upperOffset[0]);
  val S = std::max(lowerOffset[1], upperOffset[1]);
  val E = std::max(lowerOffset[0], upperOffset[0]);
  
  std::array<CEVec2, 4> corners = { CEVec2(W, N), CEVec2(W, S), CEVec2(E, S), CEVec2(E, N) };

  auto r = [](const CEVec2& offset) -> CEVec2 { return CWRotation<CWAngle>::apply(offset); };
  int lo_i = (CWAngle / 90);
  int up_i = (lo_i + 2) % 4;
  return { r(corners[lo_i]), r(corners[up_i]) };
}

template <int CWAngle = 0, int Scale = 1>
struct Direction {
 private:
  static constexpr auto r = CWRotation<CWAngle>::apply;

 public:
  static constexpr const int cwAngle = CWAngle;
  static constexpr const int scale = Scale;
  static constexpr const CEVec2 NW = r({-1, -1}) * Scale;
  static constexpr const CEVec2 N =  r({+0, -1}) * Scale;
  static constexpr const CEVec2 NE = r({+1, -1}) * Scale;
  static constexpr const CEVec2 W =  r({-1, +0}) * Scale;
  static constexpr const CEVec2 C =  r({+0, +0}) * Scale;
  static constexpr const CEVec2 E =  r({+1, +0}) * Scale;
  static constexpr const CEVec2 SW = r({-1, +1}) * Scale;
  static constexpr const CEVec2 S =  r({+0, +1}) * Scale;
  static constexpr const CEVec2 SE = r({+1, +1}) * Scale;
  static constexpr const std::array<const CEVec2*, 9> all = {&NW, &N, &NE, &W, &C, &E, &SW, &S, &SE};
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

template <typename T, typename = void>
struct unsigned_precell { typedef void type; };

template <typename T>
struct unsigned_precell<T, std::void_t<typename T::PreCellularGridSpace>> { typedef typename T::PreCellularGridSpace::Cell type; };

template <typename T, typename Dirs, typename = void>
struct DirApplier {};

template <typename T, typename Dirs>
struct DirApplier<T, Dirs, typename std::enable_if_t<std::is_convertible_v<T, CEVec2>>> {
 protected:
  static constexpr auto rotate = CWRotation<Dirs::cwAngle>::apply;

  static inline T apply(const T& source, const CEVec2& dir) {
    return T(source + dir);
  }

 public:
  static inline T NW(const T& source) { return apply(source, Dirs::NW); }
  static inline T N(const T& source)  { return apply(source, Dirs::N); }
  static inline T NE(const T& source) { return apply(source, Dirs::NE); }
  static inline T W(const T& source)  { return apply(source, Dirs::W); }
  static inline T C(const T& source)  { return apply(source, Dirs::C); }
  static inline T E(const T& source)  { return apply(source, Dirs::E); }
  static inline T SW(const T& source) { return apply(source, Dirs::SW); }
  static inline T S(const T& source)  { return apply(source, Dirs::S); }
  static inline T SE(const T& source) { return apply(source, Dirs::SE); }
  
  static std::array<T, 9> all(const T& source) {
    std::array<T, 9> result;
    for (size_t i = 0; i < 9; i++) {
      result[i] = apply(source, *Dirs::all[i]);
    }
    return result;
  }
  
  static inline T go(const T& source, const CEVec2& offset) {
    return apply(source, rotate(offset) * Dirs::scale); 
  }

  static std::array<CEVec2, 2> getRotatedBounds(const CEVec2& lowerOffset, const CEVec2& upperOffset) {
    typedef CEVec2::Component val;
    val N = std::min(lowerOffset[1], upperOffset[1]);
    val W = std::min(lowerOffset[0], upperOffset[0]);
    val S = std::max(lowerOffset[1], upperOffset[1]);
    val E = std::max(lowerOffset[0], upperOffset[0]);   
    std::array<CEVec2, 4> corners = { CEVec2(W, N), CEVec2(W, S), CEVec2(E, S), CEVec2(E, N) };
    
    auto r = [](const CEVec2& offset) -> CEVec2 { return rotate(offset); };
    int lo_i = (Dirs::cwAngle / 90);
    int up_i = (lo_i + 2) % 4;
    return { r(corners[lo_i]), r(corners[up_i]) };
  }

  static std::array<T, 2> getRotatedBounds(const T& source, const CEVec2& lowerOffset, const CEVec2& upperOffset) {
    std::array<CEVec2, 2> offsets = getRotatedBounds(lowerOffset, upperOffset);
    std::array<T, 2> result;
    for (int i = 0; i < CEVec2::dimension; i++) {
      result[i] = T(source + offsets[i] * Dirs::scale);
    }
    return result;
  }
};

template <int CWAngle = 0, typename Dirs = Direction<CWAngle>>
struct PointOffsetter : public DirApplier<CEVec2::PointVector, Dirs> {};

template <typename TCell, int CWAngle = 0, int Scale = 2>
struct PreCellOffsetter : public DirApplier<TCell, Direction<CWAngle, Scale>> {
 private:
  typedef typename TCell::PreCellularGridSpace PKSpace;
  static constexpr bool is_signed = std::is_same_v<TCell, typename PKSpace::SCell>;
  static constexpr PKSpace PK;

 protected:
  inline TCell apply(const TCell& source, const CEVec2& dir)
  
   {
    if constexpr (is_signed) {
      return PKSpace::sCell(PKSpace::sKCoords(source) + dir, PKSpace::sSign(source));
    }
    return PKSpace::uCell(PKSpace::uKCoords(source) + dir);
  }
};

/*
template <int CWAngle = 0, typename Dirs = Direction<CWAngle>>
struct PointOffsetter : public DirApplier<CEVec2::PointVector, Dirs> {
 private:
  typedef CEVec2::PointVector Point;

 public:
  using DirApplier<CEVec2::PointVector, Dirs>::getRotatedBounds;
  std::array<Point, 2> getRotatedBounds(const Point& source, const CEVec2& lowerOffset, const CEVec2& upperOffset) {
    std::array<CEVec2, 2> offsets = getRotatedBounds(lowerOffset, upperOffset);
    std::array<Point, 2> result;
    for (int i = 0; i < Point::dimension; i++) {
      result[i] = source + offsets[i] * Dirs::scale;
    }
    return result;
  }

};

template <typename TCell, int CWAngle = 0, int Scale = 2>
struct CellOffsetter : public DirApplier<TCell, Direction<CWAngle, Scale>> {
 private:
  typedef typename TCell::CellularGridSpace KSpace;
  static constexpr bool is_signed = std::is_same_v<TCell, typename KSpace::SCell>;
  
  static constexpr typename TCell::PreCellularGridSpace PK;
  const KSpace* K;

 protected:
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
*/