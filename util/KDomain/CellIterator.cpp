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

} // namespace DGtalTrace

