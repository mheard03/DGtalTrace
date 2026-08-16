#pragma once
#include <DGtal/base/Common.h>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "Globals.cpp"
#include "CellTypeTraits.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace>
class KCoordsAccessor {
 public:  
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KSpace::Point Point;
  typedef KSpace::Sign  Sign;
  typedef KSpace::Cell  Cell;
  typedef KSpace::SCell SCell;
  typedef CellTypeTraits<Cell>::UPreSelf PreCell;
  typedef CellTypeTraits<Cell>::SPreSelf SPreCell;

protected:
  inline static constexpr bool isPreSpace = concepts::CPreCellularGridSpaceND<KSpace>;

  template <concepts::CXXCell TCell>
  inline static const bool canRead = concepts::CHasKPreSpace<KPreSpace, TCell>;

  template <concepts::CXXCell TCell>
  inline static const bool canWrite = canRead<TCell> && (concepts::CHasKSpace<KSpace, TCell> || concepts::CHasKSpace<KPreSpace, TCell>);

  KCoordsAccessor() {}

public:
  // ----------------------- kCoords, plural  ---------------------------------
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static Point kCoords(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uKCoords(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sKCoords(cell);
    }
  }

  // ----------------------- kCoord, singular  --------------------------------
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static Point::Component kCoord(const TCell& cell, const DGtal::Dimension k) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uKCoord(cell, k);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sKCoord(cell, k);
    }
  }

  // ----------------------- coords  ------------------------------------------
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
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
  requires canRead<TCell>
  static Sign sign(const TCell& cell) {
    return KPreSpace::sSign(cell);
  }
  
  template <concepts::CSXCell TCell>
  requires canWrite<TCell>
  static void setSign(TCell& cell, const Sign sign) {
    KPreSpace::sSetSign(cell, sign);
  }

  template <concepts::CSXCell TCell>
  requires canWrite<TCell>
  static CellTypeTraits<TCell>::USelf unsigns(const TCell& cell) {
    if constexpr (concepts::CSPreCell<TCell>) return KPreSpace::unsigns(cell);
    if constexpr (concepts::CSCell<TCell>) return infiniteKPtr->unsigns(cell);
  }

  template <concepts::CSXCell TCell>
  requires canWrite<TCell>
  static TCell sOpp(const TCell& cell) {
    if constexpr (concepts::CSPreCell<TCell>) return KPreSpace::sOpp(cell);
    if constexpr (concepts::CSCell<TCell>) return infiniteKPtr->sOpp(cell);
  }


  // ----------------------- dim, topology  -----------------------------------
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static DGtal::Dimension dim(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uDim(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sDim(cell);
    }
  }

  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static KPreSpace::Integer topology(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uTopology(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sTopology(cell);
    }
  }

  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static bool isSpel(const TCell& cell) {
    return dim(cell) == KPreSpace::dimension;
  }

  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static bool isSurfel(const TCell& cell) {
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uIsSurfel(cell);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sIsSurfel(cell);
    }
  }
    
  template <concepts::CXXCell TCell>
  requires canRead<TCell>
  static bool isPointel(const TCell& cell) {
    return dim(cell) == 0;
  }

 private:
  inline static const KSpace* infiniteKPtr = &detail::infiniteKSpaces<KSpace>;
  
};

} // namespace DGtalTrace

