#pragma once
#include "CIsOfDimension.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <concepts>
#include <type_traits>
#include "DGtalCPreCellularGridSpace.cpp"

namespace DGtalTrace {
namespace concepts {

namespace detail {

// ============================================================
// CCellularGridSpaceND — refinement adding bounds
// ============================================================
template <typename T>
concept CCellularGridSpaceND_Types =
    CPreCellularGridSpaceND<T> && 
    requires { 
      typename T::Size;
      CCell<typename T::Cell>;
      CSCell<typename T::SCell>;
    };

template <typename T>
concept CCellularGridSpaceND_ConstMethods =
    CCellularGridSpaceND_Types<T> &&
    requires(const T x, typename T::Cell c, typename T::SCell sc,
             DGtal::Dimension k) {
      { x.size(k) } -> std::convertible_to<typename T::Integer>;
      { x.min(k) } -> std::convertible_to<typename T::Integer>;
      { x.max(k) } -> std::convertible_to<typename T::Integer>;
      { x.lowerBound() } -> std::convertible_to<typename T::Point>;
      { x.upperBound() } -> std::convertible_to<typename T::Point>;
      { x.lowerCell() } -> std::same_as<typename T::Cell>;
      { x.upperCell() } -> std::same_as<typename T::Cell>;
      { x.isSpaceClosed() } -> std::same_as<bool>;
      // unsigned bounded geometry
      { x.uFirst(c) } -> std::same_as<typename T::Cell>;
      { x.uLast(c) } -> std::same_as<typename T::Cell>;
      { x.uGetMax(c, k) } -> std::same_as<typename T::Cell>;
      { x.uGetMin(c, k) } -> std::same_as<typename T::Cell>;
      { x.uDistanceToMax(c, k) } -> std::convertible_to<typename T::Integer>;
      { x.uDistanceToMin(c, k) } -> std::convertible_to<typename T::Integer>;
      // signed bounded geometry
      { x.sFirst(sc) } -> std::same_as<typename T::SCell>;
      { x.sLast(sc) } -> std::same_as<typename T::SCell>;
      { x.sGetMax(sc, k) } -> std::same_as<typename T::SCell>;
      { x.sGetMin(sc, k) } -> std::same_as<typename T::SCell>;
      { x.sDistanceToMax(sc, k) } -> std::convertible_to<typename T::Integer>;
      { x.sDistanceToMin(sc, k) } -> std::convertible_to<typename T::Integer>;
    };

} // namespace detail

template <typename T>
concept CCellularGridSpaceND = detail::CCellularGridSpaceND_ConstMethods<T> &&
                               requires(T x, typename T::Point p, bool b) {
                                 { x.init(p, p, b) } -> std::same_as<bool>;
                               };

} // namespace concepts
} // namespace DGtalTrace