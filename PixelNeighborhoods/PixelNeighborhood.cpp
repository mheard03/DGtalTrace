#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "../traceHelper.cpp"
#include "../CEVec.cpp"
#include "CWAnglesEnum.hpp"
#include "DirectionsEnum.hpp"
#include "DirectionsOffsets.cpp"

#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>


using namespace DGtal;
using namespace DGtal::Z2i;

typedef ImageSelector<Domain, uint32_t>::Type RgbImage;

namespace DGtalTrace { namespace Offsets {

  template <typename T>
  struct Offsetter {
    virtual T at(const CEVec2 dirRotate0) const {};
    #define X(dir, ...) virtual T dir() const { throw "not implemented"; };
    FOR_DIRECTIONS(X)
    #undef X
    Offsetter() = delete;
  };

  template <CWAngles Angle>
  struct AngleOffsetter : Offsetter<CEVec2> {
    virtual CEVec2 at(const CEVec2 dirRotate0) const override {
      return Impl::rotate(Angle, dirRotate0);
    };
    #define X(dir, ...) virtual CEVec2 dir() const override { return DirectionsOffsets[Angle][Directions::dir]; };
    FOR_DIRECTIONS(X)
    #undef X

    AngleOffsetter() {};
  };

  Offsetter o = AngleOffsetter<CWAngles::Rotate90>();
  auto v = o.at(CEVec2(1, 1));




struct UnboundedSampler {
  typedef RgbImage::Value Value;
  typedef KPreSpace PK;

  UnboundedSampler(const CEVec2 centerPoint, const CWAngles angle = CWAngles::Rotate0) : centerPoint(centerPoint), angle(angle) {}
  
  inline const CEVec2 getCoords(const CEVec2 centerPoint, const CEVec2 offset) {
    return PK::uCoords(PK::uSpel(centerPoint + Impl::rotate(angle, offset)));    
  }

  const Value getValue(const CEVec2 centerPoint, const CEVec2 offset, const RgbImage& image) {
    return image.operator()(getCoords(centerPoint, offset));
  }

 private:
  const CWAngles angle;
  const CEVec2 centerPoint;

};


template <int MarginN = 0, int MarginE = MarginN, int MarginS = MarginN, int MarginW = MarginE>
struct Sampler : public UnboundedSampler {
 private:
  typedef UnboundedSampler Base;
  static constexpr const Impl::EnumArray<CEVec2, Directions, 9> getEdgeAndCornerOffsets() {
    Impl::EnumArray<CEVec2, Directions, 9> result;
    result[Directions::NW] = CEVec2(-MarginW, -MarginN);
    result[Directions::N]  = CEVec2(0, -MarginN);
    result[Directions::NE] = CEVec2(MarginE, -MarginN);
    result[Directions::W] = CEVec2(-MarginW, 0);
    result[Directions::C]  = CEVec2(0, 0);
    result[Directions::E] = CEVec2(MarginE, 0);
    result[Directions::SW] = CEVec2(-MarginW, MarginS);
    result[Directions::S]  = CEVec2(0, MarginS);
    result[Directions::SE] = CEVec2(MarginE, MarginS);
    return result;
  }

  static constexpr const Impl::EnumArray<CEVec2, Directions, 9> edgeAndCornerOffsets = getEdgeAndCornerOffsets();
    

    constexpr CEVec2 lowerOffset(-MarginW, -MarginN);
    constexpr CEVec2 upperOffset(MarginE, MarginS);

    if (lowerOffset[0] == lowerOffset[1] && lowerOffset == upperOffset * -1) {
      return { lowerOffset, upperOffset };
    }

    typedef CEVec2::Component val;
    val N = std::min(lowerOffset[1], upperOffset[1]);
    val W = std::min(lowerOffset[0], upperOffset[0]);
    val S = std::max(lowerOffset[1], upperOffset[1]);
    val E = std::max(lowerOffset[0], upperOffset[0]);
    
    const std::array<CEVec2, 4> corners = { CEVec2(W, N), CEVec2(W, S), CEVec2(E, S), CEVec2(E, N) };

    size_t lo_i = static_cast<size_t>(CWAngle);
    size_t up_i = (lo_i + 2) % 4;

    const std::array<CEVec2, 2> result { getRotated<CWAngle>(corners[lo_i]), getRotated<CWAngle>(corners[up_i]) };
    return result;
  }

 public:
  static constexpr std::array<const CEVec2, 2> offsets = getRotatedOffsets();
 
 public:
  typedef typename Base::Value Value;
  typedef typename Base::PK PK;
  Sampler() = delete;
  
  static const bool isValid(const CEVec2 centerPoint, const KSpace& K) {
    return K.uIsInside(PK::uSpel(centerPoint + offsets[0])) && K.uIsInside(PK::uSpel(centerPoint + offsets[1]));
  }
};

#define FOR_DIRECTIONS_INLINE(X) X(nw), X(n), X(ne), X(w), X(c), X(e), X(sw), X(s), X(se)
#define FOR_DIRECTIONS(X) X(nw) \
  X(n) \
  X(ne) \
  X(w) \
  X(c) \
  X(e) \
  X(sw) \
  X(s) \
  X(se)

template <Angles CWAngle>
struct NeighborSampler;

struct NeighborSample {
  typedef RgbImage::Value Value;

 private:
  static constexpr const CEVec2 o_nw = CEVec2(-1, -1);
  static constexpr const CEVec2 o_n =  CEVec2(+0, -1);
  static constexpr const CEVec2 o_ne = CEVec2(+1, -1);
  static constexpr const CEVec2 o_w =  CEVec2(-1, +0);
  static constexpr const CEVec2 o_c =  CEVec2(+0, +0);
  static constexpr const CEVec2 o_e =  CEVec2(+1, +0);
  static constexpr const CEVec2 o_sw = CEVec2(-1, +1);
  static constexpr const CEVec2 o_s =  CEVec2(+0, +1);
  static constexpr const CEVec2 o_se = CEVec2(+1, +1);

#define FIELD(name) Value name;
#define ARG(name) const Value name
#define ASSIGN(name) name(name)
#define READ(name) Sampler<CWAngle, 1>::getValue(centerPoint, o_##name, K, image)

 public:
  FOR_DIRECTIONS(FIELD)

  NeighborSample() = default;
  NeighborSample(FOR_DIRECTIONS_INLINE(ARG)) : FOR_DIRECTIONS_INLINE(ASSIGN) {}

  template <Angles CWAngle>
  static NeighborSample read(const CEVec2 centerPoint, const KSpace& K, const RgbImage& image) {
    return NeighborSample(FOR_DIRECTIONS_INLINE(READ));
  }

  template <Angles CWAngle>
  static bool isValid(const CEVec2 centerPoint, const KSpace& K) {
    return Sampler<CWAngle, 1>::isValid(centerPoint, K);
  }

  template <Angles CWAngle>
  static bool tryRead(NeighborSample& sample, const CEVec2 centerPoint, const KSpace& K, const RgbImage& image) {
    if (!isValid<CWAngle>(centerPoint, K)) return false;
    sample = read<CWAngle>(centerPoint, K, image);
    return true;
  }
  
  inline static bool tryRead(NeighborSample& sample, const Angles cwAngle, const CEVec2 centerPoint, const KSpace& K, const RgbImage& image){
    switch (cwAngle) {
      case Angles::Rotate0: return tryRead<Angles::Rotate0>(sample, centerPoint, K, image);
      case Angles::Rotate90: return tryRead<Angles::Rotate90>(sample, centerPoint, K, image);
      case Angles::Rotate180: return tryRead<Angles::Rotate180>(sample, centerPoint, K, image);
      case Angles::Rotate270: return tryRead<Angles::Rotate270>(sample, centerPoint, K, image);
    }
  }

#undef FIELD
#undef ARG
#undef ASSIGN
};

typedef Sampler<Angles::Rotate0, 1> s;




/*
template <Angles CWAngle>
struct NeighborSampler : public Sampler<CWAngle, 1> {
 private:
  typedef Sampler<CWAngle, 1> Base;
  static constexpr auto r = getRotated<CWAngle>;

 public:
  typedef typename Base::Value Value;
  typedef typename Base::PK PK;
  NeighborSampler() = delete;

#define dir(name, dx, dy) \
  static inline const Value name(const CEVec2 centerPoint, const KSpace& K, const RgbImage& image) { \
    return Base::getValue(centerPoint, {dx, dy}, K, image); \
  }

  dir(nw, -1, -1);
  dir(n,  +0, -1);
  dir(ne, +1, -1);
  dir(w,  -1, +0);
  dir(c,  +0, +0);
  dir(e,  +1, +0);
  dir(sw, -1, +1);
  dir(s,  +0, +1);
  dir(se, +1, +1);

  NeighborSample read() const {
    #define X(name) name()
    return NeighborSample(FOR_DIRECTIONS_INLINE(X));
    #undef X
  }
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

} } // end namespace