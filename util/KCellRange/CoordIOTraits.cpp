#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include <type_traits>
#include "../DimensionOf.cpp"
#include "../concepts/CHasDimension.cpp"
#include "../concepts/CCell.cpp"
#include "CoordInputTraits.cpp"

namespace DGtalTrace {

template <typename T>
struct CoordIOTraits;

namespace concepts {
namespace detail {
template <typename T>
concept CCoordIOTraits = CCoordInputTraits<T> 
  && requires {
    { T::IsStatic } -> IsBasically<bool>;
  }
  && requires (T traits, typename T::AdaptedType adapted, DGtal::Dimension k, typename T::Component value, typename T::Point point) {
    { traits.set(adapted, k, value) } -> std::same_as<void>;
    { traits.set(adapted, point) } -> std::same_as<void>;
  };
} // namespace detail

template <typename InputType>
concept CHasCoordIOTraits = requires {
  requires !std::same_as<CoordIOTraits<InputType>, void>;
  requires detail::CCoordIOTraits<CoordInputTraits<InputType>>;
};
} // namespace concepts

#define SET_ALL(prefix, constness) \
prefix void set(AdaptedType& obj, const AdaptedType& other) constness { \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    set(obj, k, get(other, k)); \
  } \
} \
prefix void set(AdaptedType& obj, const Point& p) constness \
requires (!std::convertible_to<Point, AdaptedType>) { \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    set(obj, k, p[k]); \
  } \
} \
template <typename TOther> \
requires (!std::convertible_to<TOther, AdaptedType>) \
  && (!std::convertible_to<TOther, Point>) \
  && concepts::CHasDimension<TOther, DimensionOf<AdaptedType>> \
  && concepts::CHasCoordInputTraits<TOther> \
prefix void set(AdaptedType& obj, const TOther& other) constness { \
  typedef CoordInputTraits<TOther> CIT; \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    set(obj, k, CIT::get(other, k)); \
  } \
} 
/*
prefix void set(AdaptedType& obj, const Point& other) { \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    using OtherAdapter = CoordInputTraits<TOther>; \
    CoordInputTraits<Point>
    set(obj, k, get(other, k)); \
  } \
}
template <typename TOther> \
requires (!std::same_as<TOther, AdaptedType>) \
  && std::convertible_to<TOther, AdaptedType> \
prefix void set(AdaptedType& obj, const TOther& other) { \
  set(obj, static_cast<AdaptedType>(other)); \
} \
template <concepts::CHasCoordInputTraits TOther> \
requires (!std::same_as<TOther, AdaptedType>) \
  && (!std::convertible_to<TOther, AdaptedType>) \
  && concepts::CHasDimension<TOther, DimensionOf<AdaptedType>> \
prefix void set(AdaptedType& obj, const TOther& other) { \
  using OtherAdapter = CoordInputTraits<TOther>; \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    set(obj, k, OtherAdapter::get(other, k)); \
  } \
} \
*/

template <CIsSubscriptable T>
struct CoordIOTraits<T> : public CoordInputTraits<T> {
 private:
  using Base = CoordInputTraits<T>;
 public:
  using AdaptedType = CoordInputTraits<T>::AdaptedType;
  using Component = CoordInputTraits<T>::Component;
  using Point = CoordInputTraits<T>::Point;
  static inline constexpr bool IsStatic = true;

  static inline void set(AdaptedType& t, const DGtal::Dimension k, const Component value) {
    t[k] = value;
  }
  SET_ALL(static,)
};

template <concepts::CXPreCell TCell>
struct CoordIOTraits<TCell> : public CoordInputTraits<TCell> {
  using AdaptedType = CoordInputTraits<TCell>::AdaptedType;
  using Component = CoordInputTraits<TCell>::Component;
  using Point = CoordInputTraits<TCell>::Point;
  static inline constexpr bool IsStatic = true;

  using KPreSpace = typename TCell::PreCellularGridSpace;

  static inline void set(TCell& c, const DGtal::Dimension k, const Component value) { 
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uSetKCoord(c, k, value);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sSetKCoord(c, k, value);
    }
  }
  template <typename TOther>
  requires concepts::CHasCoordInputTraits<TOther> ||
           std::constructible_from<AdaptedType, TOther> ||
           std::constructible_from<Point, TOther>
  static void set(AdaptedType &obj, const AdaptedType &other) {
  for (const DGtal ::Dimension k : dimsOf(obj)) {
    set(obj, k, get(other, k));
  }
}
static void set(AdaptedType &obj, const Point &p)
  requires(!std ::convertible_to<Point, AdaptedType>)
{
  for (const DGtal ::Dimension k : dimsOf(obj)) {
    set(obj, k, p[k]);
  }
}
template <typename TOther>
  requires(!std ::convertible_to<TOther, AdaptedType>) &&
          (!std ::convertible_to<TOther, Point>) &&
          concepts ::CHasDimension<TOther, DimensionOf<AdaptedType>> &&
          concepts ::CHasCoordInputTraits<TOther>
static void set(AdaptedType &obj, const TOther &other) {
  typedef CoordInputTraits<TOther> CIT;
  for (const DGtal ::Dimension k : dimsOf(obj)) {
    set(obj, k, CIT ::get(other, k));
  }
}
 /* SET_ALL(static,) */
};

template <concepts::CXCell TCell>
struct CoordIOTraits<TCell> : public CoordInputTraits<TCell> {
  using AdaptedType = CoordInputTraits<TCell>::AdaptedType;
  using Component = CoordInputTraits<TCell>::Component;
  using Point = CoordInputTraits<TCell>::Point;
  static inline constexpr bool IsStatic = false;

  using KSpace = typename TCell::CellularGridSpace;

  CoordIOTraits(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}

  inline void set(TCell& c, const DGtal::Dimension k, const Component value) const { 
    if constexpr (concepts::CUXCell<TCell>) {
      return myKPtr->uSetKCoord(c, k, value);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return myKPtr->sSetKCoord(c, k, value);
    }
  }
  SET_ALL(, const)
 private:
  const KSpace* myKPtr;
};

#undef SET_ALL
} // namespace DGtalTrace