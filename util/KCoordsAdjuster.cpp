#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <concepts>
#include <optional>
#include <utility>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include "../concepts/BaseConcepts.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "Globals.cpp"
#include "KCoordsAccessor.cpp"
#include "CellTypeTraits.cpp"
#include "detail/CellDelta.cpp"


namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace>
class KCoordsAdjuster : public KCoordsAccessor<TKSpace> {
  typedef KCoordsAccessor<TKSpace> Base;
  typedef KCoordsAdjuster<TKSpace> Self;
 public:  
  using KSpace = Base::KSpace;
  using KPreSpace = Base::KPreSpace;
  using Point = Base::Point;
  using Sign = Base::Sign;
  using Cell = Base::Cell;
  using SCell = Base::SCell;
  using PreCell = Base::PreCell;
  using SPreCell = Base::SPreCell;
  using CellDelta = CellDeltaSelector<TKSpace>;

 public:  // <-- TODO:: private
  using Base::isPreSpace;

  template <concepts::CXXCell TCell>
  inline static const bool canRead = Base::template canRead<TCell>;

  template <concepts::CXXCell TCell>
  inline static const bool canWrite = Base::template canWrite<TCell>;

  template <concepts::CXXCell TFrom, concepts::CXXCell TTo>
  inline static const bool canCopy = canRead<TFrom> && canWrite<TTo> && (concepts::CUXCell<TTo> || concepts::CSXCell<TFrom>);

 public:  
  KCoordsAdjuster(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks), Base() {}
  KCoordsAdjuster() requires concepts::CPreCellularGridSpaceND<KSpace> : KCoordsAdjuster(&detail::preSpaces<KSpace>) {}
  KCoordsAdjuster(const KCoordsAdjuster& other) : myKPtr(other.myKPtr), Base() {}
    Self& operator=(const Self& other) {
      if (this != &other) {
          myKPtr = other.myKPtr; 
      }
      return *this;
  }

  // ----------------------- kCoords, plural  ---------------------------------
  template <concepts::CXPreCell TCell>
  requires canWrite<TCell> && isPreSpace
  static void setKCoords(TCell& cell, const Point& kCoords) {
    setKCoords_(cell, kCoords, myPreKPtr);
  }

  template <concepts::CXCell TCell>
  requires canWrite<TCell> && (!isPreSpace)
  void setKCoords(TCell& cell, const Point& kCoords) const {
    setKCoords_(cell, kCoords, myKPtr);
  }


  // ----------------------- kCoord, singular  --------------------------------
  template <concepts::CXPreCell TCell>
  requires canWrite<TCell> && isPreSpace
  static void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) {
    setKCoord_(cell, k, value, myPreKPtr);
  }

  template <concepts::CXCell TCell>
  requires canWrite<TCell> && (!isPreSpace)
  void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) const {
    setKCoord_(cell, k, value, myKPtr);
  }


  // ----------------------- inside/valid/periodic  ---------------------------
  bool isValid(const Point& kCoords) const {
    if constexpr (isPreSpace)
      return true;
    else
      return myKPtr->cIsValid(kCoords);
  }
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  bool isValid(const TCell& cell) const {
    if constexpr (isPreSpace)
      return true;
    else
      return isValid(Base::kCoords(cell));
  }
  void assertValid(const Point& kCoords) const {
    ASSERT(isValid(kCoords));
  }
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  void assertValid(const TCell& cell) const {
    ASSERT(isValid(cell));
  }

  bool isInside(const Point& kCoords) const {
    if constexpr (isPreSpace)
      return true;
    else
      return myKPtr->cIsInside(kCoords);
  }
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  bool isInside(const TCell& cell) const {
    if constexpr (isPreSpace)
      return true;
    else
      return isInside(Base::kCoords(cell));
  }
  void assertInside(const Point& kCoords) const {
    ASSERT(isInside(kCoords));
  }
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  void assertInside(const TCell& cell) const {
    ASSERT(isInside(cell));
  }

  bool isSpacePeriodic() const {
    if constexpr (isPreSpace) return false;
    return myKPtr->isAnyDimensionPeriodic();
  }
  bool isSpacePeriodic(DGtal::Dimension k) const {
    if constexpr (isPreSpace) return false;
    return myKPtr->isSpacePeriodic(k);
  }
  auto getBoundaries() {
    std::array<std::optional<typename KSpace::Closure>, KSpace::dimension> result;
    if constexpr (!isPreSpace) {
      for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
        result[k] = myKPtr->getClosure(k);
      }
    }
    return result;
  }


  // ----------------------- PreCell creation  --------------------------------
  static PreCell uPreCell(const Point& kCoords) {
    return KPreSpace::uCell(kCoords);
  }
  static SPreCell sPreCell(const Point& kCoords, const Sign sign = KSpace::POS) {
    return KPreSpace::sCell(kCoords, sign);
  }
  template <concepts::CXXCell TCell>
  requires canCopy<TCell, PreCell>
  static PreCell uPreCell(const TCell& aPreCell) {
    return KPreSpace::uCell(Base::kCoords(aPreCell));
  }
  template <concepts::CXXCell TCell>
  requires canCopy<TCell, SPreCell>
  static SPreCell sPreCell(const TCell& aPreCell) {
    return KPreSpace::sCell(Base::kCoords(aPreCell), Base::sign(aPreCell));
  }

  // ----------------------- KPreSpace: Cell creation (static)  ---------------
  static KSpace::Cell uCell(const Point& kCoords) 
  requires isPreSpace {
    return uPreCell(kCoords);
  }
  static KSpace::SCell sCell(const Point& kCoords, const Sign sign = KSpace::POS) 
  requires isPreSpace {
    return sPreCell(kCoords, sign);
  }

  template <concepts::CXXCell TCell>
  requires isPreSpace && canCopy<TCell, Cell>
  static KSpace::Cell uCell(const TCell& aCell) {
    return uPreCell(aCell);
  }
  template <concepts::CXXCell TCell>
  requires isPreSpace && canCopy<TCell, Cell>
  static KSpace::SCell sCell(const TCell& aCell) {
    return sPreCell(aCell);
  }

  // ----------------------- KSpace: Cell creation (instance)  ----------------
  KSpace::Cell uCell(const Point& kCoords) const 
  requires (!isPreSpace) {
    return myKPtr->uCell(kCoords);
  }
  KSpace::SCell sCell(const Point& kCoords, const Sign sign = KSpace::POS) const 
  requires (!isPreSpace) {
    return myKPtr->sCell(kCoords, sign);
  }
  template <concepts::CXXCell TCell>
  requires (!isPreSpace) && canCopy<TCell, Cell>
  KSpace::Cell uCell(const TCell& aCell) const {
    return myKPtr->uCell(Base::kCoords(aCell));
  }
  template <concepts::CXXCell TCell>
  requires (!isPreSpace) && canCopy<TCell, SCell>
  KSpace::SCell sCell(const TCell& aCell) const {
    return myKPtr->sCell(Base::kCoords(aCell), Base::sign(aCell));
  }

  // ----------------------- shift  -------------------------------------------
public:
  template <concepts::CXPreCell TCell>
  static void shift(TCell& cell, const CellDelta& delta) {
    setKCoord(cell, delta.dim, Base::kCoord(cell, delta.dim) + delta.delta);
  }
  template <concepts::CXCell TCell>
  void shift(TCell& cell, const CellDelta& delta) const {
    setKCoord(cell, delta.dim, Base::kCoord(cell, delta.dim) + delta.delta);
  }

  template <concepts::CXPreCell TCell>
  static TCell shifted(const TCell& aCell, const CellDelta& delta) {
    TCell result(aCell);
    shift(result, delta);
    return result;
  }

  template <concepts::CXCell TCell>
  TCell shifted(const TCell& aCell, const CellDelta& delta) const {
    TCell result(aCell);
    shift(result, delta);
    return result;
  }

 private:
  const KSpace* myKPtr = nullptr;
  static inline const KPreSpace* myPreKPtr = &detail::preSpaces<KSpace>;
  
  // ----------------------- kCoord, kCoords  ---------------------------------
  template <concepts::CXXCell TCell>
  static void setKCoords_(TCell& cell, const Point& kCoords, const TCell::CellularGridSpace* kPtr) {
    if constexpr (concepts::CUXCell<TCell>) {
      kPtr->uSetKCoords(cell, kCoords);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      kPtr->sSetKCoords(cell, kCoords);
    }
  }
  template <concepts::CXXCell TCell>
  static void setKCoord_(TCell& cell, const DGtal::Dimension k, const Point::Component value, const TCell::CellularGridSpace* kPtr) {
    if constexpr (concepts::CUXCell<TCell>) {
      kPtr->uSetKCoord(cell, k, value);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      kPtr->sSetKCoord(cell, k, value);
    }
  }

};

/*
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


  // canRead
  static_assert((requires (KCoordsAdjuster<DGtal::Z2i::KPreSpace> ks) { requires ks.canRead<DGtal::Z2i::KSpace::PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<DGtal::Z2i::KPreSpace> ks) { requires ks.canRead<DGtal::Z3i::KSpace::PreCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<DGtal::Z3i::KPreSpace> ks) { requires ks.canRead<DGtal::Z3i::KSpace::PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<DGtal::Z3i::KPreSpace> ks) { requires ks.canRead<DGtal::Z2i::KSpace::PreCell> == false; } ));

  // canWrite (where canRead is true)
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks) { requires ks.canWrite<KSpace::PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks) { requires ks.canWrite<KSpace::Cell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks) { requires ks.canWrite<KSpace::SPreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KPreSpace> ks) { requires ks.canWrite<KSpace::SCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canWrite<KSpace::PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canWrite<KSpace::Cell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canWrite<KSpace::SPreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canWrite<KSpace::SCell> == true; } ));

  // canCopy (where canWrite is true)
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<PreCell, PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<Cell, PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SPreCell, PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SCell, PreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<PreCell, Cell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<Cell, Cell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SPreCell, Cell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SCell, Cell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<PreCell, SPreCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<Cell, SPreCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SPreCell, SPreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SCell, SPreCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<PreCell, SCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<Cell, SCell> == false; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SPreCell, SCell> == true; } ));
  static_assert((requires (KCoordsAdjuster<KSpace> ks) { requires ks.canCopy<SCell, SCell> == true; } ));







  static_assert((requires (KCoordsAdjuster<KSpace> ks, KSpace::Cell c, KSpace::Point p) { ks.setKCoords(c, p); }));
}
*/


} // namespace DGtalTrace

