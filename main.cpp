#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <ranges>
#include "util/KDomain/KCoordsAdjuster.cpp"
#include "util/concepts/CXCellularGridSpaceND.cpp"
#include "util/KDomain/KRectangle.cpp"
#include "util/KDomain/CellIterator.cpp"
#include "util/KDomain/CellRange.cpp"
#include "util/traceHelper.cpp"
#include "util/makeRange.cpp"

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

  typedef DGtalTrace::CellIterator<SCell> CIS;
  CIS i(ks, ks.sCell({0, 0}, KSpace::NEG), ks.sCell({10, 10}, KSpace::NEG));
  CIS end;
  std::vector<SCell> sCells;
  std::copy(i, end, std::back_inserter(sCells));
  trace.info() << "naked: " << std::endl;
  for (auto cell : sCells) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 



  DGtalTrace::detail::CellRange_single<Cell> crRect(ksFinite.uCell({1, 1}), kRect);
  trace.info() << "crRect: " << crRect << std::endl;
  for (auto cell : crRect) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  const SCell cell1 = ks.sCell({1, 1}, KSpace::NEG);
  const SCell cell2 = ks.sCell({1, 1}, KSpace::POS);

  CellRange<SCell> crJoined({cell1, cell2}, ks);
  trace.info() << "crJoined: " << crJoined.size() <<  std::endl;
  for (auto cell : crJoined) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  typedef KCoordsAdjuster<KSpace> KCA;
  const auto filter = [](const SCell& cell) -> bool { return KCA::dim(cell) == 1 && KCA::sign(cell) == KSpace::POS; };
  auto templateCells = CellRange<SCell>::allStartCells([](const SCell& cell) -> bool { return KCA::dim(cell) == 1 && KCA::sign(cell) == KSpace::POS; });

  CellRange<SCell> crJoined2(templateCells, ks);
  trace.info() << "crJoined2: " << crJoined2.size() <<  std::endl;
  for (auto cell : crJoined2) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  /*
  std::vector<DGtalTrace::detail::CellRange_single<SCell>> tmp = { crKN, crKP };

  CellRange crJoined({crKN, crKP});
  trace.info() << "crJoined: " << crJoined.size() <<  std::endl;
  for (auto cell : crJoined) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  std::vector<std::vector<Cell>> tmp2;
  for (auto i = ks.min(0); i <= ks.max(0); i++) {
    std::vector<Cell> cols;
    for (auto j = ks.min(1); j <= ks.max(1); j++) {
      cols.push_back(ks.uCell({i, j}));
    }
    tmp2.push_back(cols);
  }
  CellRange crJoined2(tmp2);
  trace.info() << "crJoined2: " << crJoined2.size() << std::endl;
  for (auto cell : crJoined2) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 



  trace.info() << "allStartCells<Cell>: " << std::endl;
  for (auto cell : CellRange<Cell>::allStartCells()) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  
  trace.info() << "allStartCells<SCell>: " << std::endl;
  for (auto cell : CellRange<SCell>::allStartCells()) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  trace.info() << "allStartCells<Cell>(1): " << std::endl;
  for (auto cell : CellRange<Cell>::allStartCells(1)) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  
  trace.info() << "allStartCells<SCell>(1): " << std::endl;
  for (auto cell : CellRange<SCell>::allStartCells(1)) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  */
}

