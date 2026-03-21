#include "../CellIterator.cpp"
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/KhalimskySpaceND.h>


void writeCellIterator(CellIterator<DGtal::Z2i::KSpace::Cell> begin, CellIterator<DGtal::Z2i::KSpace::Cell> end) {
  using DGtal::trace;
  typedef DGtal::Z2i::KSpace::Cell TCell;
  typedef TCell Cell;
  typedef typename TCell::Point Point;

  // typedef TKSpace KSpace;
  // typedef typename KSpace::Cell Cell;
  Point first = begin->preCell().coordinates;
  auto y = first[1];
  
  int i = 0;
  for (auto it = begin; it != end; ++it) {
    Point p = it->preCell().coordinates;
    if (p[1] != y) {
      trace.info() << std::endl;
      y = p[1];
    }
    trace.info() << (p[0] == first[0] ? "  " : ", ") << *it;
    if (i++ > 200) {
      trace.info() << std::endl << "Too many cells, stopping output." << std::endl;
      break;
    }
  }
  trace.info() << std::endl;
}

template <typename TCell>
int testCellIterator(DGtal::ConstAlias<typename TCell::CellularGridSpace> ks, typename TCell::Point offset) {
  using DGtal::trace;
  typedef TCell Cell;
  typedef typename TCell::CellularGridSpace KSpace;

  trace.info() << "testCellIterator: " << &ks << " w/ offset " << offset << std::endl;
  Cell c = ks->uFirst(ks->lowerBound() + offset);
  
  auto begin = CellIterator<Cell>(ks, c);
  auto end = CellIterator<Cell>();
  writeCellIterator(begin, end);
  trace.info() << std::endl;
}

template <typename TKSpace>
int testCellIterator(DGtal::ConstAlias<TKSpace> ks, const typename TKSpace::Cell& lower, const typename TKSpace::Cell& upper) {
  using DGtal::trace;
  typedef TKSpace KSpace;
  typedef typename KSpace::Cell Cell;

  trace.info() << "testCellIterator: " << &ks << " w/ range " << lower << " to " << upper << std::endl;
  Cell c = lower;
  
  auto begin = CellIterator<Cell>(ks, lower);
  auto end = CellIterator<Cell>();
  writeCellIterator(begin, end);
  trace.info() << std::endl;
}

/*

template <typename TKSpace>
int testCellIterator() {
  typedef TKSpace KSpace;
  typedef typename KSpace::Cell Cell;
  
  KSpace K;
  
  // Closed K-space
  K.init({0, 0}, {5, 5}, true);
  testCellIterator(K);
  Cell lower({5, 5}), upper ({5, 5});
  testCellIterator(K, lower, upper);
  


  // K.init({0, 0}, {20, 20}, { KSpace::Closure::PERIODIC, KSpace::Closure::OPEN });
}
  */