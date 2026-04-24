#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <boost/concept/assert.hpp>
#include <concepts>
#include <ranges>
#include <utility>
#include "../CellTypeTraits.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "setKCoords_unsafe.cpp"

namespace DGtalTrace {

namespace detail {

template<typename T, typename ... U>
concept is_among = (std::same_as<T, U> || ...);

} // end namespace detail

template <concepts::CXCellularGridSpaceND TKSpace>
class KCoordsAdjuster {
 public:  
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KSpace::Point Point;
  typedef KSpace::Cell  Cell;
  typedef KSpace::SCell SCell;
  typedef CellTypeTraits<Cell>::UPreSelf PreCell;
  typedef CellTypeTraits<Cell>::SPreSelf SPreCell;

 public:  
  KCoordsAdjuster(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}
  KCoordsAdjuster() requires concepts::CPreCellularGridSpaceND<KSpace> : KCoordsAdjuster(infiniteKPtr) {}

  template <concepts::CXPreCell TCell>
  requires std::same_as<KPreSpace, typename TCell::PreCellularGridSpace>
  void setKCoords(TCell& cell, const Point& kCoords) {
    setKCoords(cell, kCoords, preKPtr);
  }

  template <concepts::CXCell TCell>
  requires std::same_as<KSpace, typename TCell::CellularGridSpace>
  void setKCoords(TCell& cell, const Point& kCoords) {
    setKCoords(cell, kCoords, infiniteKPtr);
    if (myKPtr != infiniteKPtr) throwIfOutside(cell, myKPtr);
  }

 private:
  inline static const KSpace* infiniteKPtr = &detail::ksInfinite<KSpace>;
  inline static const KPreSpace* preKPtr = &detail::ksInfinite<KPreSpace>;
  const KSpace* myKPtr;

  template <concepts::CXXCell TCell>
  void setKCoords(TCell& cell, const Point& kCoords, const TCell::CellularGridSpace* kPtr) {
    if constexpr (concepts::CUXCell<TCell>) {
      kPtr->uSetKCoords(cell, kCoords);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      kPtr->sSetKCoords(cell, kCoords);
    }
  }
};

namespace {
  using namespace DGtal::Z2i;
    
  static_assert((requires (KCoordsAdjuster<KSpace> ks, KSpace::Cell c, KSpace::Point p) { ks.setKCoords(c, p); }));
  static_assert((requires (KCoordsAdjuster<KSpace> ks, KSpace::SCell c, KSpace::Point p) { ks.setKCoords(c, p); }));
  static_assert((requires (KCoordsAdjuster<KSpace> ks, KSpace::PreCell c, KSpace::Point p) { ks.setKCoords(c, p); }));
  static_assert((requires (KCoordsAdjuster<KSpace> ks, KSpace::SPreCell c, KSpace::Point p) { ks.setKCoords(c, p); }));
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks, KPreSpace::Cell c, KSpace::Point p) { ks.setKCoords(c, p); }));
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks, KPreSpace::SCell c, KSpace::Point p) { ks.setKCoords(c, p); }));
}

} // namespace DGtalTrace