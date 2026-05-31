#pragma once
// #include <DGtal/base/Common.h>
// #include <DGtal/helpers/StdDefs.h>
// #include <type_traits>
#include "concepts/CCell.cpp"
#include <DGtal/kernel/PointVector.h>

namespace DGtal {

template <DGtalTrace::concepts::CXPreCell TCell>
inline bool isLower(TCell const& lhs, TCell const& rhs) {
  typedef typename TCell::PreCellularGridSpace KPreSpace;
  if constexpr (DGtalTrace::concepts::CSPreCell<TCell>) {
    return isLower(KPreSpace::sKCoords(lhs), KPreSpace::sKCoords(rhs));
  }
  if constexpr (DGtalTrace::concepts::CUPreCell<TCell>) {
    return isLower(KPreSpace::uKCoords(lhs), KPreSpace::uKCoords(rhs));
  }
}

template <DGtalTrace::concepts::CXPreCell TCell>
inline bool isUpper(TCell const& lhs, TCell const& rhs) {
  typedef typename TCell::PreCellularGridSpace KPreSpace;
  if constexpr (DGtalTrace::concepts::CSPreCell<TCell>) {
    return isUpper(KPreSpace::sKCoords(lhs), KPreSpace::sKCoords(rhs));
  }
  if constexpr (DGtalTrace::concepts::CUPreCell<TCell>) {
    return isUpper(KPreSpace::uKCoords(lhs), KPreSpace::uKCoords(rhs));
  }
}

}