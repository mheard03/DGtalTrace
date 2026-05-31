#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include <boost/concept/assert.hpp>
#include <concepts>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include "concepts/BaseConcepts.cpp"

namespace DGtalTrace {

namespace concepts {
namespace point_adapter_detail {
enum CoordsModes { NONE = 0, CONSTRUCT, ASSIGN, LOOP };

template <typename TComponent, typename OtherComponent>
concept CIsConvertible = std::same_as<TComponent, DGtal::ArithmeticConversionType<TComponent, OtherComponent>>;

template <typename T>
concept CHasIndexOperator = requires(T t, std::size_t i) { t[i]; };

template <typename T>
using element_type = std::conditional_t<CHasIndexOperator<T>, std::decay_t<decltype(std::declval<T&>()[0])>, void>;

template <typename TSource, typename TPoint>
concept CCanManuallyCopyFrom = concepts::CPoint<TPoint> && 
  requires (TSource& source, std::size_t i) {
    std::ssize(source);
    std::begin(source);
    requires !std::same_as<element_type<TSource>, void>;
    requires CIsConvertible<typename TPoint::Component, element_type<TSource>>;
  };

template <typename TSource, typename TPoint>
concept CHasDimensionMismatch = requires {
  { std::tuple_size<TSource>::value } -> std::convertible_to<std::size_t>;
  requires std::tuple_size_v<TSource> != DimensionOf<TPoint>;
};


template <typename TPoint, typename TCoords>
struct PointAdapterMode {
 private:
  static constexpr CoordsModes detectExtractMode() {
    if constexpr (CHasDimensionMismatch<TCoords, TPoint>) return CoordsModes::NONE;
    if constexpr (std::constructible_from<TPoint, TCoords>) return CoordsModes::CONSTRUCT;
    if constexpr (std::assignable_from<TPoint, TCoords>) return CoordsModes::ASSIGN;
    if constexpr (CCanManuallyCopyFrom<TCoords, TPoint>) return CoordsModes::LOOP;
    return CoordsModes::NONE;
  }
  static constexpr CoordsModes detectSetMode() {
    if constexpr (CHasDimensionMismatch<TCoords, TPoint>) return CoordsModes::NONE;
    if (std::assignable_from<TPoint, TCoords>) return CoordsModes::ASSIGN;
    if (CCanManuallyCopyFrom<TCoords, TPoint>) return CoordsModes::LOOP;
    return CoordsModes::NONE;
  }
 public:
  static inline constexpr CoordsModes extract = detectExtractMode();
  static inline constexpr CoordsModes set = detectSetMode();
};
  

} // namespace adapter_detail

  template <typename TPoint, typename TOther>
  concept CHasCoordsAdapter = requires (TPoint& point, TOther& other) {
    getCoords<TPoint>(other);
    setCoords(point, other);
  };

} // namespace concepts

namespace {
  using namespace concepts::point_adapter_detail;
}

template <concepts::CPoint TPoint, typename TCoords>
requires requires { PointAdapterMode<TPoint, TCoords>::extract != CoordsModes::NONE; }
TPoint extractCoords(const TCoords& coords) {
  constexpr CoordsModes mode = PointAdapterMode<TPoint, TCoords>::extract;
  if constexpr (mode == CoordsModes::CONSTRUCT) {
    return TPoint(coords);
  }
  if constexpr (mode == CoordsModes::ASSIGN) {
    TPoint result = coords;
    return result;
  }
  if constexpr (mode == CoordsModes::LOOP) {
    if (!std::ssize(coords) == TPoint::dimension) throw std::invalid_argument("Dimensions didn't match");
    TPoint result;
    for (DGtal::Dimension k = 0; k < TPoint::dimension; k++) {
      result[k] = coords[k];
    }
    return result;
  }
}

template <concepts::CPoint TPoint, typename TCoords>
requires requires { PointAdapterMode<TPoint, TCoords>::set != CoordsModes::NONE; }
TPoint setCoords(TPoint& point, const TCoords& coords) {
  if (!std::ssize(coords) == TPoint::dimension) throw std::invalid_argument("Dimensions didn't match");
  constexpr CoordsModes mode = PointAdapterMode<TPoint, TCoords>::set;

  if constexpr (mode == CoordsModes::ASSIGN) {
    point = coords;
    return;
  }
  if constexpr (mode == CoordsModes::LOOP) {    
    for (DGtal::Dimension k = 0; k < TPoint::dimension; k++) {
      point[k] = coords[k];
    }
  }
}


bool test = concepts::CHasCoordsAdapter<DGtal::Z2i::Point, std::array<int, 2>>;
typedef DGtal::Z2i::Point Point;
typedef std::array<int, 2> Coords;
Point point;
Coords coords;

Point p = extractCoords<Point>(coords);

} // namespace DGtalTrace