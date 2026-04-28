#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "util/KDomain/KCoordsAdjuster.cpp"
#include "util/concepts/CXCellularGridSpaceND.cpp"
#include "util/KDomain/KRectangle.cpp"
#include "util/KDomain/CellIterator.cpp"
#include "util/traceHelper.cpp"

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
  
  
  KSpace ks = ksPeriodic;
  trace.info() << "ks: " << ks << std::endl;
  trace.info() << "  lowerCell: " << ks.lowerCell() << std::endl;
  trace.info() << "  upperCell: " << ks.upperCell() << std::endl;  

  KRectangle<KSpace> kRect(ks);
  trace.info() << "kRect: " << kRect << std::endl;
  trace.info() << "  total extent: " << kRect.extent() << std::endl;
  trace.info() << "  cell extent: " << kRect.extent(ks.lowerCell()) << std::endl;


  typedef DGtalTrace::CellIterator<SCell> CIS;
  CIS i(ks, ks.sCell({0, 0}, KSpace::NEG), ks.sCell({10, 10}, KSpace::NEG));
  CIS end;
  std::vector<SCell> sCells;
  std::copy(i, end, std::back_inserter(sCells));
  trace.info() << "naked: " << std::endl;
  for (auto cell : sCells) {
    trace.info() << cell; 
  }
  trace.info() << std::endl;



  DGtalTrace::CellRange<Cell> crRect(ksFinite.uCell({1, 1}), kRect);
  trace.info() << "crRect: " << crRect << std::endl;
  for (auto cell : crRect) {
    trace.info() << cell; 
  }
  trace.info() << std::endl;

  DGtalTrace::CellRange<SCell> crKS(ks.sCell({0, 0}, KSpace::NEG), ks);
  trace.info() << "crKS: " << crKS << std::endl;
  for (auto cell : crKS) {
    trace.info() << cell; 
  }
  trace.info() << std::endl; 

}

