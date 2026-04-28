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
  // typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KSpace::Sign Sign;
  typedef typename KSpace::Point Point;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef DGtal::NumberTraits<typename Point::Component> NT;

  // primary constructor
  KRectangle(const Point& lowerKCoords, const Point& upperKCoords, const KSpace::Sign defaultSign)
      : myLowerKCoords(lowerKCoords),
        myUpperKCoords(upperKCoords),
        myDefaultSign(defaultSign) {
    // if (DGtal::isLower(myUpperKCoords, myLowerKCoords)) throw std::invalid_argument("upperKCoords must be >= lowerKCoords in every dimension");
  }

 public:
  template <concepts::CXXCell TCell>
  KRectangle(const TCell& lowerCell, const TCell& upperCell, const KSpace::Sign defaultSign)
      : KRectangle(KCA::kCoords(lowerCell), KCA::kCoords(upperCell), defaultSign) {}

  template <concepts::CXXCell TCell>
  KRectangle(const TCell& lowerCell, const TCell& upperCell)
      : KRectangle(lowerCell, upperCell, inferDefaultSign(lowerCell)) {}

  template <concepts::CXXCell TCell>
  KRectangle(const std::array<TCell, 2> cells)
      : KRectangle(cells[0], cells[1]) {}

  explicit KRectangle(const KSpace& ks, const KSpace::Sign defaultSign = inferDefaultSign())
  requires concepts::CCellularGridSpaceND<KSpace>
      : KRectangle(ks.lowerCell(), ks.upperCell(), defaultSign) {}


 public:
  template <concepts::CXXCell TCell>
  const TCell lower() const {
    return makeCell<TCell>(myLowerKCoords);
  }

  template <concepts::CXXCell TCell>
  const TCell upper() const {
    return makeCell<TCell>(myUpperKCoords);
  }

  const KSpace::Sign defaultSign() const {
    return myDefaultSign;
  }
  
  bool isEmpty() const {
    return DGtal::isUpper(myLowerKCoords, myUpperKCoords);
  }

  bool isValid(const KSpace &ks) const {
    return ks.cIsInside(myLowerKCoords) && ks.cIsInside(myUpperKCoords);
  }

  bool isInside(const Point &kCoords) const {
    if (DGtal::isLower(kCoords, myLowerKCoords)) return false;
    if (DGtal::isUpper(kCoords, myUpperKCoords)) return false;
    return true;
  }

  Point extent() const {
    return myUpperKCoords - myLowerKCoords + Point::diagonal(1);
  }

  template <concepts::CXXCell TCell>
  Point extent(const TCell& templateCell) const {
    return KCA::kCoords(last(templateCell)) - KCA::kCoords(first(templateCell)) + Point::diagonal(1);
  }

  template <concepts::CXXCell TCell>
  TCell first(const TCell& templateCell) const {
    Point kCoords = myLowerKCoords;
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] += 1;
      }
    }
    return makeCell<TCell>(kCoords);
  }

  template <concepts::CXXCell TCell>
  TCell last(const TCell& templateCell) const {
    Point kCoords = myUpperKCoords;
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(KCA::kCoord(templateCell, k)) != NT::odd(kCoords[k])) {
        kCoords[k] -= 1;
      }
    }
    return makeCell<TCell>(kCoords);
  }

  void selfDisplay(std::ostream& out) const {
    std::string validity = isEmpty() ? "EMPTY " : "";
    out << "[KRectangle " << myLowerKCoords << "x"
        << myUpperKCoords << " " << (myDefaultSign ? "+" : "-") << "]";    
  }

 private:
  Point myLowerKCoords;
  Point myUpperKCoords;
  static inline const KCoordsAdjuster<KSpace> myAdjuster = KCA::infinite();
  const KSpace::Sign myDefaultSign;

  template <concepts::CXXCell TCell>
  TCell makeCell(const Point& kCoords) const {
    TCell result = makeCell<TCell>();
    myAdjuster.setKCoords(result, kCoords);
    return result;
  }

  template <concepts::CXXCell TCell>
  TCell makeCell() const {
    TCell result;
    if constexpr (concepts::CSXCell<TCell>) {
      myAdjuster.setSign(result, myDefaultSign);
    }
    return result;
  }

  template <concepts::CXXCell TCell>
  static KSpace::Sign inferDefaultSign(const TCell& cell) {
    if constexpr (concepts::detail::CellIsSigned<TCell>) {
      return KPreSpace::sSign(cell);
    }
    return inferDefaultSign();
  }

  static KSpace::Sign inferDefaultSign() {
    return KPreSpace::sSign(KPreSpace::SCell(0));
  }
};


template <typename TKSpace>
std::ostream& operator<<(std::ostream& out, const KRectangle<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}


} // namespace DGtalTrace
