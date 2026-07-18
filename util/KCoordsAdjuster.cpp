#pragma once
#include <DGtal/topology/KhalimskySpaceND.h>
#include <concepts>
#include <optional>
#include <utility>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include "concepts/BaseConcepts.cpp"
#include "concepts/CCell.cpp"
#include "concepts/CXCellularGridSpaceND.cpp"
#include "CellTypeTraits.cpp"

namespace DGtalTrace {
namespace detail {
template <concepts::CPreCellularGridSpaceND KSpace>
constexpr KSpace preSpaces;
} // end namespace detail

template <concepts::CXCellularGridSpaceND TKSpace>
class KCoordsAdjuster {
 public:  
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KSpace::Point Point;
  typedef KSpace::Sign  Sign;
  typedef KSpace::Cell  Cell;
  typedef KSpace::SCell SCell;
  typedef CellTypeTraits<Cell>::UPreSelf PreCell;
  typedef CellTypeTraits<Cell>::SPreSelf SPreCell;

 public:
  template <concepts::CXXCell TCell>
  inline static const bool canRead = concepts::CHasKPreSpace<KPreSpace, TCell>;

  template <concepts::CXXCell TCell>
  inline static const bool canWrite = canRead<TCell> && (concepts::CXPreCell<TCell> || concepts::CHasKSpace<KSpace, TCell>);

  template <concepts::CXXCell TFrom, concepts::CXXCell TTo>
  inline static const bool canCopy = canRead<TFrom> && canWrite<TTo> && (concepts::CUXCell<TTo> || concepts::CSXCell<TFrom>);

 public:  
  KCoordsAdjuster(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}
  KCoordsAdjuster() requires concepts::CPreCellularGridSpaceND<KSpace> : KCoordsAdjuster(preKPtr) {}
  static KCoordsAdjuster<KSpace> infinite() {
    return KCoordsAdjuster(*preKPtr);
  }


  // ----------------------- kCoords, plural  ---------------------------------
  template <concepts::CXXCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static Point kCoords(const TCell& cell) {
    return getKCoords_(cell);
  }

  template <concepts::CXPreCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static void setKCoords(TCell& cell, const Point& kCoords) {
    setKCoords_(cell, kCoords, preKPtr);
  }

  template <concepts::CXCell TCell>
  requires concepts::CHasKSpace<KSpace, TCell>
  void setKCoords(TCell& cell, const Point& kCoords) const {
    setKCoords_(cell, kCoords, myKPtr);
  }


  // ----------------------- kCoord, singular  --------------------------------
  template <concepts::CXXCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static Point::Component kCoord(const TCell& cell, const DGtal::Dimension k) {
    return getKCoord_(cell, k);
  }

  template <concepts::CXPreCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) {
    setKCoord_(cell, k, value, preKPtr);
  }

  template <concepts::CXCell TCell>
  requires concepts::CHasKSpace<KSpace, TCell>
  void setKCoord(TCell& cell, const DGtal::Dimension k, const Point::Component value) const {
    setKCoord_(cell, k, value, myKPtr);
  }


  // ----------------------- coords  ------------------------------------------
  template <concepts::CXXCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static Point coords(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uCoords(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sCoords(cell);
    }
  }


  // ----------------------- sign  --------------------------------------------
  template <concepts::CSXCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static Sign sign(const TCell& cell) {
    return KPreSpace::sSign(cell);
  }
  
  template <concepts::CSXCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static void setSign(TCell& cell, const Sign sign) {
    KPreSpace::sSetSign(cell, sign);
  }


  // ----------------------- make/convert PreCell  ----------------------------
  template <concepts::CUPreCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static TCell make(const Point& kCoords) {
    return make_inner<TCell>(kCoords, preKPtr);
  }
  template <concepts::CSPreCell TCell>
  requires concepts::CHasKPreSpace<KPreSpace, TCell>
  static TCell make(const Point& kCoords, const Sign sign = KSpace::POS) {
    return make_inner<TCell>(kCoords, sign, preKPtr);
  }

  template <concepts::CUPreCell TCell, concepts::CXXCell XCell>
  requires concepts::CBool<canCopy<XCell, TCell>>
  static TCell convert(const XCell& cell) {
    return make<TCell>(kCoords(cell));
  }
  template <concepts::CSPreCell TCell, concepts::CSXCell XCell>
  requires concepts::CBool<canCopy<XCell, TCell>>
  static TCell convert(const XCell& cell) {
    return make<TCell>(kCoords(cell), sign(cell));
  }
  template <concepts::CSPreCell TCell, concepts::CXXCell XCell>
  requires concepts::CBool<canRead<XCell>> && concepts::CBool<canWrite<TCell>>
  static TCell convert(const XCell& cell, const Sign sign) {
    return make<TCell>(kCoords(cell), sign);
  }

  // ----------------------- make/convert Cell  -------------------------------
  template <concepts::CUCell TCell>
  requires concepts::CHasKSpace<KSpace, TCell>
  TCell make(const Point& kCoords) const {
    return make_inner<TCell>(kCoords, myKPtr);
  }
  template <concepts::CSCell TCell>
  requires concepts::CHasKSpace<KSpace, TCell>
  TCell make(const Point& kCoords, const Sign sign = KSpace::POS) const {
    return make_inner<TCell>(kCoords, sign, myKPtr);
  }
  
  template <concepts::CUCell TCell, concepts::CXXCell XCell>
  requires concepts::CBool<canCopy<XCell, TCell>>
  TCell convert(const XCell& cell) const {
    return make<TCell>(kCoords(cell));
  }
  template <concepts::CSCell TCell, concepts::CSXCell XCell>
  requires concepts::CBool<canCopy<XCell, TCell>>
  TCell convert(const XCell& cell) const {
    return make<TCell>(kCoords(cell), sign(cell));
  }
  template <concepts::CSCell TCell, concepts::CXXCell XCell>
  requires concepts::CBool<canRead<XCell>> && concepts::CBool<canWrite<TCell>>
  TCell convert(const XCell& cell, const Sign sign) const {
    return make<TCell>(kCoords(cell), sign);
  }


  // ----------------------- dim, topology  -----------------------------------
  template <concepts::CXXCell TCell>
  static DGtal::Dimension dim(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uDim(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sDim(cell);
    }
  }

  template <concepts::CXXCell TCell>
  static KPreSpace::Integer topology(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uTopology(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sTopology(cell);
    }
  }


  // ----------------------- inside/valid/periodic  ---------------------------
  template <concepts::CXXCell TCell>
  bool isValid(const TCell& cell) const {
    return isValid_(cell, myKPtr);
  }
  template <concepts::CXXCell TCell>
  void assertValid(const TCell& cell) const {
    assertValid_(cell, myKPtr);
  }

  template <concepts::CXXCell TCell>
  bool isInside(const TCell& cell) const {
    return isInside_(cell, myKPtr);
  }
  template <concepts::CXXCell TCell>
  void assertInside(const TCell& cell) const {
    assertInside_(cell, myKPtr);
  }

  bool isSpacePeriodic(DGtal::Dimension k) const {
    if constexpr (concepts::CPreCellularGridSpaceND<KSpace>) return false;
    return myKPtr->isSpacePeriodic(k);
  }

  /*
  // ----------------------- misc  --------------------------------------------
  template <concepts::CXCellularGridSpaceND TKSpaceOther>
  static const bool isInfinite(const TKSpaceOther& other) {
    // Prespaces are always infinite
    if constexpr (concepts::CPreCellularGridSpaceND<TKSpaceOther>) return true;
    
    // If this is the global infinite space, we're done
    if (&infiniteKPtr == &other) return true;

    std::array<typename TKSpaceOther::Closure, TKSpaceOther::dimension> closure;
    for (DGtal::Dimension k; k < TKSpaceOther::dimension; k++) {
      closure[k] = other.getClosure(k);
    }
    TKSpaceOther tmp

    if (infiniteKPtr->lowerCell() != other->lowerCell()) return false;
    if (infiniteKPtr->upperCell() != other->upperCell()) return false;
    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (infiniteKPtr->getClosure(k) != other->getClosure(k)) return false;
    }
    return true;
  }
  */
    
 private:
  inline static const KPreSpace* preKPtr = &detail::preSpaces<KPreSpace>;
  const KSpace* myKPtr = nullptr;

  
  // ----------------------- kCoord, kCoords  ---------------------------------
  template <concepts::CXXCell TCell>
  static Point getKCoords_(TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return preKPtr->uKCoords(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return preKPtr->sKCoords(cell);
    }
  }
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
  static Point::Component getKCoord_(TCell& cell, const DGtal::Dimension k) {
    if constexpr (concepts::CUXCell<TCell>) {
      return preKPtr->uKCoord(cell, k);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return preKPtr->sKCoord(cell, k);
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

  // ----------------------- make_inner  --------------------------------------
  template <concepts::CUXCell TCell>
  static TCell make_inner(const Point& kCoords, const TCell::CellularGridSpace* kPtr) {
    return kPtr->uCell(kCoords);
  }
  template <concepts::CSXCell TCell>
  static TCell make_inner(const Point& kCoords, const Sign sign, const TCell::CellularGridSpace* kPtr) {
    return kPtr->sCell(kCoords, sign);
  }

  // ----------------------- inside/valid  ------------------------------------
  template <concepts::CXXCell TCell, concepts::CXCellularGridSpaceND TTKSpace>
  static bool isValid_(const TCell& cell, const TTKSpace* kPtr) {
    if constexpr (concepts::CPreCellularGridSpaceND<TTKSpace>) return true;
    if constexpr (concepts::CCellularGridSpaceND<TTKSpace>) {
      return kPtr->cIsValid(kCoords(cell));
    }
  }
  template <concepts::CXXCell TCell, concepts::CXCellularGridSpaceND TTKSpace>
  static void assertValid_(const TCell& cell, const TTKSpace* kPtr) {
    ASSERT(isValid_(cell, kPtr));
  }


  template <concepts::CXXCell TCell, concepts::CXCellularGridSpaceND TTKSpace>
  static bool isInside_(const TCell& cell, const TTKSpace* kPtr) {
    if constexpr (concepts::CPreCellularGridSpaceND<TTKSpace>) return true;
    if constexpr (concepts::CCellularGridSpaceND<TTKSpace>) {
      return kPtr->cIsInside(kCoords(cell));
    }
  }
  template <concepts::CXXCell TCell, concepts::CXCellularGridSpaceND TTKSpace>
  static void assertInside_(const TCell& cell, const TTKSpace* kPtr) {
    ASSERT(isInside_(cell, kPtr));
  }

};

namespace {
  /*
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
  */
}



} // namespace DGtalTrace

