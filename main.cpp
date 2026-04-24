#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "util/KDomain/KCoordsAdjuster.cpp"
#include "util/concepts/CXCellularGridSpaceND.cpp"

using namespace DGtal;
using namespace DGtalTrace;

template <DGtalTrace::concepts::CXXCell TCell, DGtalTrace::concepts::CXCellularGridSpaceND TKSpace>
void printCell(const TCell& cell, const TKSpace& ks, const std::string& prefix = "") {
  if (prefix.size() > 0) {
    trace.info() << prefix;
  }
  trace.info() << cell;
  if constexpr (DGtalTrace::concepts::CCellularGridSpaceND<TKSpace>) {
    trace.info() << " valid:" << ks.uIsValid(cell) << ", inside:" << ks.uIsInside(cell);
  }
  trace.info() << std::endl;
}

int main(int argc, char** argv) {
  
  typedef DGtal::Z2i::KSpace KSpace;
  typedef KSpace::Cell Cell;

  KSpace ksInfinite;
  KSpace ksFinite({0,0}, {5,5}, true);
  KSpace ksPeriodic;
  ksPeriodic.init({0,0}, {5,5}, KSpace::Closure::PERIODIC);
  
  
  KSpace ks = ksFinite;
  KCoordsAdjuster<KSpace> ksa(ks);
  Cell cell;
  printCell(cell, ks, "initial:");
  ksa.setKCoords(cell, {5, 5});
  printCell(cell, ks, "after move 1:");
  ksa.setKCoords(cell, {50, 50});
  printCell(cell, ks, "after move 2:");
  if constexpr (DGtalTrace::concepts::CXPreCell<Cell>) {
    //KSpace::PreCellularGridSpace::uSetKCoords(cell., {50, 50});
    printCell(cell, ks, "after uSetKCoords:");
  }
  if constexpr (DGtalTrace::concepts::CXCell<Cell>) {
    ks.uSetKCoords(cell, {50, 50});
    printCell(cell, ks, "after uSetKCoords:");
  }
}

