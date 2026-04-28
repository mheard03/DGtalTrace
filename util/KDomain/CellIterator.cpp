#pragma once
#include <cstddef>
#include <exception>
#include <iterator>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <stdexcept>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "KRectangle.cpp"
#include "setKCoords_unsafe.cpp"

namespace DGtalTrace {

template <typename TCell>
  requires concepts::CXCell<TCell> && concepts::CXCellularGridSpaceND<typename TCell::CellularGridSpace>
class CellIterator {
public:
  typedef TCell Cell;
  typedef Cell::CellularGridSpace KSpace;

  using iterator_category = std::forward_iterator_tag;
  using value_type = Cell;
  using difference_type = std::ptrdiff_t;
  using reference = const Cell &;

  // End sentinel
  CellIterator() : myKPtr(nullptr), myDoneFlag(true) {}

  // Begin iterator (KSpace)
  CellIterator(DGtal::ConstAlias<KSpace> ks, const Cell &lower,
                     const Cell &upper)
    requires concepts::CXCell<Cell>
      : myKPtr(&ks), myLower(lower), myUpper(upper), myCell(lower),
        myDoneFlag(false) {}

  // Begin iterator (KPreSpace)
  CellIterator(const Cell &lower, const Cell &upper)
    requires concepts::CXPreCell<Cell>
      : myKPtr(nullptr), myLower(lower), myUpper(upper), myCell(lower),
        myDoneFlag(false) {}

  reference operator*() const { return myCell; }

  CellIterator &operator++() {
    if (!tryAdvance(myCell, myLower, myUpper))
      myDoneFlag = true;
    return *this;
  }

  CellIterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const CellIterator &o) const {
    if (myDoneFlag && o.myDoneFlag)
      return true;
    if (myDoneFlag != o.myDoneFlag)
      return false;
    return myCell == o.myCell;
  }

  bool operator!=(const CellIterator &o) const { return !(*this == o); }

private:
  inline bool tryAdvance(Cell &cell, const Cell &lower, const Cell &upper) const {
    using namespace DGtalTrace::concepts;
    if constexpr (CPreCell<TCell>)
      return KPreSpace::uNext(cell, lower, upper);
    if constexpr (CSPreCell<TCell>)
      return KPreSpace::sNext(cell, lower, upper);
    if constexpr (CCell<TCell>)
      return myKPtr->uNext(cell, lower, upper);
    if constexpr (CSCell<TCell>)
      return myKPtr->sNext(cell, lower, upper);
  }

private:
  const KSpace *myKPtr;
  Cell myLower, myUpper, myCell;
  bool myDoneFlag;
};


template <class TCell>
class CellRange {
  
public:
  // typedef DGtal::Z2i::KSpace::Cell Cell;
  typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef CellIterator<Cell> iterator;
  typedef KRectangle<KSpace> KRect;

  explicit CellRange(const Cell &templateCell, DGtal::ConstAlias<KSpace> ks)
  requires concepts::CCellularGridSpaceND<KSpace>
      : myKPtr(&ks), myBounds(makeBounds(templateCell, ks)) {}

  CellRange(const Cell &templateCell, const KRect& rect)
      : myKPtr(&detail::ksInfinite<KSpace>), myBounds(makeBounds(templateCell, rect)) {}

  size_t size() const {
    if (myBounds.isEmpty()) return 0;
    if (!myBounds.isValid(*myKPtr)) throw std::out_of_range("myBounds was inValid with respect to myKPtr");
    size_t result = 1;
    for (auto length : myBounds.extent()) {
      result *= length;
    }
    return result;
  }

  iterator begin() const { 
    if (myBounds.isEmpty()) return end();
    if (!myBounds.isValid(*myKPtr)) throw std::out_of_range("myBounds was inValid with respect to myKPtr");
    return iterator(myKPtr, myBounds.template lower<TCell>(), myBounds.template upper<TCell>()); 
  }
  iterator end()   const { return iterator(); }

  void selfDisplay(std::ostream& out) const {
    std::string strInvalid = (myBounds.isValid(*myKPtr)) ? "" : "INVALID ";
    std::string strSize = (strInvalid.size() == 0) ? "" : ("[" + std::to_string(size()) + "] ");
    out << "[" << strInvalid << "CellRange" << strSize << " " << myBounds << "]";    
  }
private:
  const KSpace *myKPtr;
  const KRect myBounds;

  static KRect makeBounds(const Cell &templateCell, const KRect& rect) {
    typename KSpace::Sign defaultSign = rect.defaultSign();
    if constexpr (concepts::CSXCell<Cell>) {
      defaultSign = KPreSpace::sSign(templateCell);
    }
    // DGtal::trace.info() << "makeBounds(templateCell, rect) " << templateCell << ", " << rect << std::endl;
    return KRect(rect.first(templateCell), rect.last(templateCell), defaultSign);
  }
  static KRect makeBounds(const Cell &templateCell, const KSpace& ks) {
    // DGtal::trace.info() << "makeBounds(templateCell, ks) " << templateCell << ", " << ks << std::endl;
    return makeBounds(templateCell, KRect(ks));
  }
  static KRect makeBounds(const KRect& rect) {
    // DGtal::trace.info() << "makeBounds(rect) " << rect << std::endl;
    return makeBounds(rect.template lower<Cell>(), rect, rect.defaultSign());
  }

};

template <typename TCell>
std::ostream& operator<<(std::ostream& out, const CellRange<TCell>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace

