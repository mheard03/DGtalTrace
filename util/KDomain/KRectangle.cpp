#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <stdexcept>
#include "../traceHelper.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "KCoordsAdjuster.cpp"
#include "setKCoords_unsafe.cpp"


namespace DGtalTrace {

template <typename TKSpace> requires concepts::CXCellularGridSpaceND<TKSpace>
class KRectangle {
 protected:
  typedef DGtal::Z2i::KSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KSpace::Point Point;
  typedef typename KPreSpace::Cell PreCell;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef DGtal::NumberTraits<typename Point::Component> NT;

 public:
  KRectangle(const PreCell& lowerCell, const PreCell& upperCell)
      : myLowerCell(lowerCell), myUpperCell(upperCell) {}

  template <concepts::CXXCell TCell>
  KRectangle(const TCell& lowerCell, const TCell& upperCell)
      : KRectangle(KCA::convert<PreCell>(lowerCell), KCA::convert<PreCell>(upperCell)) {}

  KRectangle(const TKSpace& ks) requires concepts::CCellularGridSpaceND<TKSpace>
      : KRectangle(ks.lowerCell(), ks.upperCell()) {}

  template <concepts::CXXCell TCell>
  KRectangle(const std::array<TCell, 2> cells)
      : KRectangle(cells[0], cells[1]) {}

 public:
  const PreCell lower() const {
    return myLowerCell;
  }

  const PreCell upper() const {
    return myUpperCell;
  }
  
  Point extent() const {
    return KCA::kCoords(myUpperCell) - KCA::kCoords(myLowerCell) + Point::diagonal(1);
  }
  bool isEmpty() const {
    return DGtal::isUpper(KCA::kCoords(myLowerCell), KCA::kCoords(myUpperCell));
  }
  bool isValid(const KSpace &ks) const {
    return ks.uIsInside(myLowerCell) && ks.uIsInside(myUpperCell);
  }

  template <concepts::CXXCell TCell>
  TCell lower(const TCell& templateCell) const {
    TCell result = templateCell;
    myAdjuster.setKCoords(result, KCA::kCoords(lower()));
    return result;
  }

  template <concepts::CXXCell TCell>
  TCell upper(const TCell& templateCell) const {
    TCell result = templateCell;
    myAdjuster.setKCoords(result, KCA::kCoords(upper()));
    return result;
  }

  template <concepts::CXXCell TCell>
  TCell first(const TCell& templateCell) const {
    Point kCoords = KCA::kCoords(myLowerCell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] += 1;
      }
    }
    TCell result = templateCell;
    myAdjuster.setKCoords(result, kCoords);
    return result;
  }

  template <concepts::CXXCell TCell>
  TCell last(const TCell& templateCell) const {
    Point kCoords = KCA::kCoords(myUpperCell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] -= 1;
      }
    }
    TCell result = templateCell;
    myAdjuster.setKCoords(result, kCoords);
    return result;
  }

  void selfDisplay(std::ostream& out) const {
    out << "[KRectangle " << myLowerCell << "x" << myUpperCell << "]";    
  }

 private:
  PreCell myLowerCell;
  PreCell myUpperCell;
  static inline const KCoordsAdjuster<KSpace> myAdjuster = KCA::infinite();
};


template <typename TKSpace>
std::ostream& operator<<(std::ostream& out, const KRectangle<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}


} // namespace DGtalTrace
