#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "CIsOfDimension.cpp"

namespace DGtalTrace {
namespace concepts {
namespace detail {
  template <typename TCell, int dim = -1>
  concept CXXCellBase = requires {
    typename TCell::Integer;
    typename TCell::UnsignedInteger;
    typename TCell::Point;
    typename TCell::PreCellularGridSpace;
    typename TCell::CellularGridSpace;
    CHasDimension<TCell, TCell::Point::dimension>;
  };
}

template <typename TCell, int dim = -1>
concept CCell = detail::CXXCellBase<TCell, dim> && requires (TCell c) {
  typename TCell::Cell;
  typename TCell::PreCell;
  std::same_as<TCell, typename TCell::Cell>;
  { c.preCell() } -> std::convertible_to<typename TCell::SPreCell>;
};
template <typename TCell, int dim = -1>
concept CSCell = detail::CXXCellBase<TCell, dim> && requires (TCell c) {
  typename TCell::SCell;
  typename TCell::SPreCell;
  std::same_as<TCell, typename TCell::SCell>;
  { c.preCell() } -> std::convertible_to<typename TCell::SPreCell>;
};

template <typename TCell, int dim = -1>
concept CPreCell = detail::CXXCellBase<TCell, dim> && requires (TCell c) {
  typename TCell::PreCell;
  std::same_as<TCell, typename TCell::PreCell>;
  { c.coordinates } -> std::convertible_to<typename TCell::Point>;
};
template <typename TCell, int dim = -1>
concept CSPreCell = detail::CXXCellBase<TCell, dim> && requires (TCell c) {
  typename TCell::SPreCell;
  std::same_as<TCell, typename TCell::SPreCell>;
  { c.coordinates } -> std::convertible_to<typename TCell::Point>;
};

// Any signed or unsigned precell
template <typename TCell, int dim = -1>
concept CXPreCell = CPreCell<TCell, dim> || CSPreCell<TCell, dim>;

// Any signed or unsigned cell
template <typename TCell, int dim = -1>
concept CXCell = CCell<TCell, dim> || CSCell<TCell, dim>;

// Any signed cell or precell
template <typename TCell, int dim = -1>
concept CSXCell = CSCell<TCell, dim> || CSPreCell<TCell, dim>;

// Any signed or unsigned cell or precell
template <typename TCell, int dim = -1>
concept CXXCell = CXCell<TCell, dim> || CSXCell<TCell, dim>;

} // namespace concepts
} // namespace DGtalTrace