#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <concepts>
#include <DGtal/base/Assert.h>
#include <DGtal/kernel/PointVector.h>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "KCoordsAdjuster.cpp"
#include "isLowerUpper.cpp"
#include "../dimsOf.cpp"


namespace DGtalTrace {

namespace {
  template <typename TCell>
  concept CLegalInputXCell = concepts::CHasKPreSpace<KPreSpace, TCell>;

  template <typename TCell>
  concept CLegalInputPreCell = concepts::CXPreCell<TCell> && CLegalInputXCell<TCell>;

  template <typename TCell>
  concept CLegalInputCell = concepts::CXCell<TCell> && CLegalInputXCell<TCell>;

  template <typename TArray, typename TValue>
  concept CIsArray = std::same_as<TArray, std::array<TValue, DimensionOf<TValue>>>;

  template <typename TArray>
  concept CLegalInputXCellArray = requires {
    typename TArray::value_type;
    requires CIsArray<TArray, typename TArray::value_type>;
    requires CLegalInputXCell<typename TArray::value_type>;    
  };

  template <typename TArray>
  concept CLegalInputPreCellArray = CLegalInputXCellArray<TArray> && CLegalInputPreCell<typename TArray::value_type>;

  template <typename TArray>
  concept CLegalInputCellArray = CLegalInputXCellArray<TArray> && CLegalInputCell<typename TArray::value_type>;
}

/** @brief Stores a pair of unsigned precells representing the absolute Khalimsky coordinates of a rectangle */
template <typename TUPreCell>
requires concepts::CUPreCell<TUPreCell> // && std::same_as<TUPreCell, typename CellTypeTraits<TUPreCell>::UPreSelf>
class KRectangle {
 protected:
  typedef KRectangle<TUPreCell> Self;
  // typedef DGtal::Z2i::KSpace::PreCell PreCell;
  typedef TUPreCell PreCell;
  typedef PreCell::PreCellularGridSpace KPreSpace;
  typedef typename KPreSpace::Point Point;
  typedef typename KPreSpace::Vector Vector;
  static constexpr DGtal::Dimension dimension = KPreSpace::dimension;
  typedef KCoordsAdjuster<KPreSpace> KCA;
  typedef DGtal::NumberTraits<typename Point::Component> NT;
  typedef std::array<PreCell, dimension> CellArray;

 protected:
  template <CLegalInputXCellArray TXCellArray>
  KRectangle(const TXCellArray& corners) : myLowerCell(KCA::template convert<PreCell>(corners[0])), myUpperCell(KCA::template convert<PreCell>(corners[1])) {}

 public:
  /** @brief Creates a KRectangle encompassing a subrange of KPreSpace
    * @note lowerCell and upperCell will be used without modification. */
  template <CLegalInputPreCell TXPreCell>
  explicit KRectangle(const TXPreCell& lowerCell, const TXPreCell& upperCell) : KRectangle(std::array<TXPreCell, 2>{lowerCell, upperCell}) {}

  /** @brief Creates a KRectangle encompassing the entirety of ks.
    * @param ks provides lowerCell and upperCell */
  template <concepts::CCellularGridSpaceND TKSpace>
  requires std::same_as<KPreSpace, typename TKSpace::PreCellularGridSpace>
  KRectangle(const TKSpace& ks) : KRectangle(std::array<PreCell, 2>{ks.lowerCell(), ks.upperCell()}) {}

  /** @brief Creates a KRectangle encompassing a subrange of KPreSpace
    * @pre lowerCell and upperCell are both valid cells in ks
    * @note lowerCell will be moved into the valid range of ks if necessary.
    * @note if upperCell is lower than lowerCell along a periodic dimension, it will be brought into the valid bounds of ks */
  template <CLegalInputXCell TXXCell, concepts::CCellularGridSpaceND TKSpace>
  requires concepts::CHasKSpace<TKSpace, TXXCell> || concepts::CHasKPreSpace<typename TKSpace::PreCellularGridSpace, TXXCell>
  KRectangle(const TXXCell& lowerCell, const TXXCell& upperCell, const TKSpace& ks) : KRectangle(normalizeBounds(lowerCell, upperCell, ks)) {}
 
  const PreCell lowerCell() const {
    return myLowerCell;
  }
  const PreCell upperCell() const {
    return myUpperCell;
  }
  
  bool isEmpty() const {
    return DGtal::isUpper(myLowerCell, myUpperCell);
  }
  Vector extent() const {
    return KCA::kCoords(myUpperCell) - KCA::kCoords(myLowerCell) + Vector::diagonal(1);
  }
  Integer size() const {
    if (isEmpty()) return 0;
    Integer size = 1;
    for (const Integer leg : extent()) {
      size *= leg;
    }
    return size;
  }

  bool contains(const PreCell& cell) const {
    return !(DGtal::isUpper(cell, myUpperCell) || DGtal::isLower(cell, myLowerCell));
  }
  bool contains(const Point& kCoords) const {
    return !(DGtal::isUpper(kCoords, KCA::kCoords(myUpperCell)) || DGtal::isLower(kCoords, KCA::kCoords(myLowerCell)));
  }

  template <concepts::CXXCell TCell>
  PreCell first(const TCell& templateCell) const {
    Point kCoords = KCA::kCoords(myLowerCell);
    for (DGtal::Dimension k : dimsOf<KSpace>()) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] += 1;
      }
    }
    return KPreSpace::uCell(kCoords);
  }

  template <concepts::CXXCell TCell>
  PreCell last(const TCell& templateCell) const {
    Point kCoords = KCA::kCoords(myUpperCell);
    for (DGtal::Dimension k : dimsOf<KSpace>()) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] -= 1;
      }
    }
    return KPreSpace::uCell(kCoords);
  }
  
 protected:
  template <CLegalInputXCell TXXCell, concepts::CCellularGridSpaceND TKSpace>
  requires concepts::CHasKSpace<TKSpace, TXXCell>
  static CellArray normalizeBounds(const TXXCell& lowerCell, const TXXCell& upperCell, const TKSpace& ks) {
    KCA adjuster(ks);
    adjuster.assertValid(lowerCell);
    adjuster.assertValid(upperCell);
    Point lowerKCoords = KCA::kCoords(lowerCell);
    Point upperKCoords = KCA::kCoords(upperCell);

    if (DGtal::isLower(upperCell, lowerCell) && ks.isAnyDimensionPeriodic()) {
      const Vector ksExtent = KCA::kCoords(ks.upperCell()) - KCA::kCoords(ks.lowerCell()) + Point::diagonal(1);
      for (DGtal::Dimension k : dimsOf<KSpace>()) {
        if (lowerKCoords[k] > upperKCoords[k] && ks.isSpacePeriodic(k)) {
          DGtal::trace.info() << "Extending on dimension " << k << std::endl;
          upperKCoords[k] += ksExtent[k];
        }
      }
    }
    return {KPreSpace::uCell(lowerKCoords), KPreSpace::uCell(upperKCoords)}; 
  }

 public:
  void selfDisplay(std::ostream& out) const {
    out << "[KRectangle " << myLowerCell << "x" << myUpperCell << "]";    
  }

 private:
  const PreCell myLowerCell;
  const PreCell myUpperCell;
};


template <typename TKSpace>
std::ostream& operator<<(std::ostream& out, const KRectangle<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}


} // namespace DGtalTrace
