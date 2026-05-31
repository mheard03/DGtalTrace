#pragma once
#include <iterator>
#include <DGtal/base/Common.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <type_traits>
// #include "../util/KCellConstraints.cpp"

template <class TCell>
class MultiCellIterator;

template <class TCell>
class CellIterator {
 public:
  typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef KSpace::Cell UCell;
  typedef KSpace::SCell SCell;
  typedef std::forward_iterator_tag iterator_category;
  typedef Cell value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const Cell* pointer;
  typedef const Cell& reference;

  // End sentinel
  CellIterator() : myKPtr(nullptr), myFlagIsDone(true) {}

  CellIterator(DGtal::ConstAlias<KSpace> ks, const UCell &cell)
      : myKPtr(&ks), myCurrentCell(cell), myLowerCell(ks->uFirst(cell)), myUpperCell(ks->uLast(cell)), myFlagIsDone(false) {}

  CellIterator(DGtal::ConstAlias<KSpace> ks, const SCell &cell)
      : myKPtr(&ks), myCurrentCell(cell), myLowerCell(ks->sFirst(cell)), myUpperCell(ks->sLast(cell)), myFlagIsDone(false) {}

  reference operator*() const { return myCurrentCell; }
  pointer operator->() const { return &myCurrentCell; }

  CellIterator &operator++() requires(std::is_same_v<TCell, UCell>) {
    if (!myKPtr->uNext(myCurrentCell, myLowerCell, myUpperCell)) {
      myFlagIsDone = true;
    }
    return *this;
  }
  CellIterator &operator++() requires(std::is_same_v<TCell, SCell>){
    if (!myKPtr->sNext(myCurrentCell, myLowerCell, myUpperCell)) {
      myFlagIsDone = true;
    }
    return *this;
  }

  CellIterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const CellIterator &o) const {
    if (myFlagIsDone && o.myFlagIsDone) return true;
    return myFlagIsDone == o.myFlagIsDone &&
           myCurrentCell == o.myCurrentCell && 
           myKPtr == o.myKPtr &&
           myLowerCell == o.myLowerCell && 
           myUpperCell == o.myUpperCell;
  }

  bool operator!=(const CellIterator &o) const { return !(*this == o); }

private:
  const KSpace *myKPtr;
  const Cell myLowerCell, myUpperCell;
  Cell myCurrentCell;
  bool myFlagIsDone;
};


template <class TCell>
class CellRange {
public:
  typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef CellIterator<Cell> iterator;

  // Explicit bounds
  CellRange(DGtal::ConstAlias<KSpace> ks, const Cell &cell)
      : myKPtr(&ks), myCell(cell) {}

  iterator begin() const { return iterator(myKPtr, myCell); }
  iterator end()   const { return iterator(); }

private:
  const KSpace *myKPtr;
  const Cell myCell;
};


/*
template <typename TCell, std::enable_if_t<is_cell_v<TCell> && !is_precell_v<TCell>> = true>
class CellIterator : CellIteratorBase {
  typedef DGtal::Z2i::Cell::CellularGridSpace cgs;
public:
  typedef TCell Cell;
  typedef Cell::CellularGridSpace KSpace;
  typedef std::forward_iterator_tag iterator_category;
  typedef Cell value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const Cell* pointer;
  typedef const Cell& reference;

  // End sentinel
  CellIterator() : myKPtr(nullptr) {
    myFlagIsDone = true;
  }

  // Begin iterator with explicit bounds
  CellIterator(DGtal::ConstAlias<KSpace> ks, const Cell &lower, const Cell &upper)
      : myKPtr(&ks), myLowerCell(lower), myUpperCell(upper), myCurrentCell(lower) {}

  // Begin iterator defaulting to full space
  CellIterator(DGtal::ConstAlias<KSpace> ks)
      : CellIterator(ks, ks.lowerCell(), ks.upperCell()) {}

  reference operator*() const { return myCurrentCell; }
  pointer operator->() const { return &myCurrentCell; }

  template <std::enable_if_t<!std::is_signed_v<Cell>> = true>
  CellIterator &operator++() {
    if (!myKPtr->uNext(myCurrentCell, myLowerCell, myUpperCell))
      myFlagIsDone = true;
    return *this;
  }

  template <std::enable_if_t<std::is_signed_v<Cell>> = true>
  CellIterator &operator++() {
    if (!myKPtr->sNext(myCurrentCell, myLowerCell, myUpperCell))
      myFlagIsDone = true;
    return *this;
  }

  CellIterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const CellIterator &o) const {
    if (myFlagIsDone && o.myFlagIsDone)
      return true;
    if (myFlagIsDone != o.myFlagIsDone)
      return false;
    return myCurrentCell == o.myCurrentCell;
  }

  bool operator!=(const CellIterator &o) const { return !(*this == o); }

private:
  const KSpace *myKPtr;
  const Cell myLowerCell, myUpperCell;
  Cell myCurrentCell;
};
*/

/*
// ---------------------------------------------------------------------------
// Range wrapper — enables range-based for loops
// ---------------------------------------------------------------------------
template <typename KSpace>
class CellRange {
public:
  using Cell = typename KSpace::Cell;
  using iterator = CellIterator<KSpace>;

  // Explicit bounds
  CellRange(DGtal::ConstAlias<KSpace> ks, const Cell &lower, const Cell &upper)
      : ks_(ks), lower_(lower), upper_(upper) {}

  // Default to full space
  CellRange(DGtal::ConstAlias<KSpace> ks)
      : CellRange(ks, ks.lowerCell(), ks.upperCell()) {}

  iterator begin() const { return iterator(ks_, lower_, upper_); }
  iterator end() const { return iterator(); }

private:
  const KSpace *ks_;
  Cell lower_, upper_;
};
*/