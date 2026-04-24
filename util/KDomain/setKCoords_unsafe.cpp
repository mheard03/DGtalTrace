#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/helpers/StdDefs.h>
#include <stdexcept>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"

namespace DGtalTrace {

namespace detail {

template <concepts::CXCellularGridSpaceND KSpace>
const KSpace ksInfinite;

} // end namespace detail


template <concepts::CXCellularGridSpaceND KSpace>
const KSpace& getInfinite() {
  return detail::ksInfinite<KSpace>;
}

template <concepts::CXXCell TCell>
const TCell::CellularGridSpace& getInfinite() {
  return detail::ksInfinite<typename TCell::CellularGridSpace>;
}

template <concepts::CXXCell TCell>
void setKCoords_unsafe(TCell& cell, const typename TCell::Point& kCoords) {
  typedef typename TCell::CellularGridSpace KSpace;
  if constexpr (concepts::CUXCell<TCell>) {
    getInfinite<KSpace>().uSetKCoords(cell, kCoords);
    return;
  }
  if constexpr (concepts::CSXCell<TCell>) {
    getInfinite<KSpace>().sSetKCoords(cell, kCoords);
    return;
  }
}

template <concepts::CXXCell TCell>
void throwIfOutside(const TCell& cell, const typename TCell::CellularGridSpace* kPtr) {
  if (kPtr == nullptr) return;
  if constexpr (concepts::CXPreCell<TCell>) return;
  if constexpr (concepts::CUXCell<TCell>) {
    if (!kPtr->uIsInside(cell)) throw std::invalid_argument("throwIfOutside: cell coordinates were outside the space");
  }
  if constexpr (concepts::CSXCell<TCell>) {
    if (!kPtr->sIsInside(cell)) throw std::invalid_argument("throwIfOutside: cell coordinates were outside the space");
  }
  return;
}

} // namespace DGtalTrace