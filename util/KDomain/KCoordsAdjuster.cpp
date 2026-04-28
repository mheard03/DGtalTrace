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
  requires concepts::CHasKPreSpace<TCell, KPreSpace>
  void setKCoords(TCell& cell, const Point& kCoords) const {
    setKCoords(cell, kCoords, preKPtr);
  }

  template <concepts::CXCell TCell>
  requires concepts::CHasKSpace<TCell, KSpace>
  void setKCoords(TCell& cell, const Point& kCoords) const {
    setKCoords(cell, kCoords, infiniteKPtr);
    if (myKPtr != infiniteKPtr) throwIfOutside(cell, myKPtr);
  }

  template <concepts::CXPreCell TCell>
  requires concepts::CHasKPreSpace<TCell, KPreSpace>
  void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) const {
    setKCoord(cell, k, value, preKPtr);
  }

  template <concepts::CXCell TCell>
  requires concepts::CHasKSpace<TCell, KSpace>
  void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) const {
    setKCoord(cell, k, value, infiniteKPtr);
    if (myKPtr != infiniteKPtr) throwIfOutside(cell, myKPtr);
  }

  template <concepts::CXXCell TCell>
  requires concepts::CHasKPreSpace<TCell, KPreSpace>
  static Point kCoords(const TCell& cell) {
    return getKCoords(cell);
  }

  template <concepts::CXXCell TCell>
  requires concepts::CHasKPreSpace<TCell, KPreSpace>
  static Point::Component kCoord(const TCell& cell, const DGtal::Dimension k) {
    return getKCoord(cell, k);
  }

  template <concepts::CSXCell TCell>
  static void setSign(TCell& cell, const KSpace::Sign sign) {
    if constexpr (concepts::CXPreCell<TCell>) {
      preKPtr->sSetSign(cell, sign);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      infiniteKPtr->sSetSign(cell, sign);
    }
  }

  static KCoordsAdjuster<KSpace> infinite() {
    return KCoordsAdjuster(*infiniteKPtr);
  }

  template <concepts::CXCellularGridSpaceND TKSpaceOther>
  static const bool isInfinite(const TKSpaceOther& other) {
    if constexpr (concepts::CPreCellularGridSpaceND<TKSpaceOther>) return true;
    return &detail::ksInfinite<TKSpaceOther> == &other;
  }

 private:
  inline static const KSpace* infiniteKPtr = &detail::ksInfinite<KSpace>;
  inline static const KPreSpace* preKPtr = &detail::ksInfinite<KPreSpace>;
  const KSpace* myKPtr;

  template <concepts::CXXCell TCell>
  void setKCoords(TCell& cell, const Point& kCoords, const TCell::CellularGridSpace* kPtr) const {
    if constexpr (concepts::CUXCell<TCell>) {
      kPtr->uSetKCoords(cell, kCoords);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      kPtr->sSetKCoords(cell, kCoords);
    }
  }

  template <concepts::CXXCell TCell>
  void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value, const TCell::CellularGridSpace* kPtr) const {
    if constexpr (concepts::CUXCell<TCell>) {
      kPtr->uSetKCoord(cell, k, value);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      kPtr->sSetKCoord(cell, k, value);
    }
  }

  template <concepts::CXXCell TCell>
  static Point getKCoords(TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return preKPtr->uKCoords(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return preKPtr->sKCoords(cell);
    }
  }

  template <concepts::CXXCell TCell>
  static Point::Component getKCoord(TCell& cell, const DGtal::Dimension k) {
    if constexpr (concepts::CUXCell<TCell>) {
      return preKPtr->uKCoord(cell, k);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return preKPtr->sKCoord(cell, k);
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

  static_assert((requires (KSpace::Cell c) { KCoordsAdjuster<KSpace>::kCoords(c); }));
  static_assert((requires (KSpace::SCell c) { KCoordsAdjuster<KSpace>::kCoords(c); }));
  static_assert((requires (KSpace::PreCell c) { KCoordsAdjuster<KSpace>::kCoords(c); }));
  static_assert((requires (KSpace::SPreCell c) { KCoordsAdjuster<KSpace>::kCoords(c); }));
  static_assert((requires (KSpace::Cell c) { KCoordsAdjuster<KPreSpace>::kCoords(c); }));
  static_assert((requires (KSpace::SCell c) { KCoordsAdjuster<KPreSpace>::kCoords(c); }));
  static_assert((requires (KSpace::PreCell c) { KCoordsAdjuster<KPreSpace>::kCoords(c); }));
  static_assert((requires (KSpace::SPreCell c) { KCoordsAdjuster<KPreSpace>::kCoords(c); }));
}

} // namespace DGtalTrace