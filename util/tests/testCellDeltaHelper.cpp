#include <magic_enum/magic_enum.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "../../concepts/CCell.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include "../CellTypeTraits.cpp"
#include "../KCoordsAdjuster.cpp"
#include "../detail/CellDelta.cpp"
#include "../detail/CellDeltaHelper.cpp"
#include "../traceHelper.cpp"

namespace DGtalTrace {

template<DGtalTrace::concepts::CXXCell TCell>
constexpr std::string CellTypeName = []() {
  std::string result;
  if (concepts::CSXCell<TCell>)
    result += "S";
  else
    result += "U";
  if (concepts::CXPreCell<TCell>) result += "Pre";
  result += "Cell";
  return result;
}();

template<concepts::CXXCell TCell, typename TKSpace = typename TCell::CellularGridSpace, typename TKCA, typename TCDH>
std::vector<std::string> testCdhDeltaBasicsImpl(const TKSpace ks, const TKCA kca, const TCDH cdh) {
  typedef TKSpace KSpace;
  typedef typename KSpace::Point Point;

  auto makeCell = [&kca](auto p) { 
    if constexpr(std::same_as<TCell, typename KSpace::Cell>) return kca.uCell(p);
    if constexpr(std::same_as<TCell, typename KSpace::SCell>) return kca.sCell(p);
  };

  std::vector<std::string> errors;

  int i = -1;
  for (auto x : { 0, 5, 12 }) {
    for (auto y : { 0, 6, 12 }) {  
      const TCell startCell = makeCell(Point(x, y));
      for (auto dim : { 0, 1 }) {
        for (auto dir : { -2, -1, 1, 2 }) {
          Point endKCoords = Point(x,y);
          endKCoords[dim] += dir;
          if (!kca.isInside(endKCoords)) continue;
          
          i++;
          const TCell endCell = makeCell(endKCoords);
          
          auto writeError = [&](std::string message, CellDelta<KSpace>& delta) {
            std::stringstream err;
            err << "testCdhDeltaBasicsImpl<" << CellTypeName<TCell> << ">" << std::endl;
            err << message << std::endl;
            err << "Loop " << i << ": x=" << x << ", y=" << y << ", dim =" << dim << ", dir =" << dir << std::endl;
            err << "  Cells:     " << startCell << " -> " << endCell << std::endl;
            err << "  Delta:     " << delta << std::endl;
            err << "  Raw Delta: ";
            cdh.getRawDelta(startCell, endCell).selfDisplay(err);
            err << std::endl;
            DGtal::trace.info() << err.str() << std::endl;
            errors.push_back(err.str());
          };
          
          auto delta = cdh.getDelta(startCell, endCell);
          if (dim != delta.dim) {
            writeError("dim != delta.dim", delta);
          }

          if ((dir < 0 && delta.sign) || (dir > 0 && !delta.sign)) {
            writeError("sign didn't match dir", delta);
          }

          TCell startCopy = makeCell(startCell);
          kca.shift(startCopy, delta);
          if (startCopy != endCell) {
            std::stringstream err;
            err << "startCopy: " << startCopy;
            writeError("startCopy != endCell; " + err.str(), delta);
          }

          TCell endCopy = makeCell(endCell);
          kca.shift(endCopy, delta.opp());
          if (endCopy != startCell) {
            std::stringstream err;
            err << "endCopy: " << endCopy;
            writeError("endCopy != startCell; " + err.str(), delta);
          }
        }
      }
    }
  }
  return errors;
}


std::vector<std::string> testCdhDeltaBasics() {
  std::vector<std::string> allResults;

  auto appendResults = [&](const std::vector<std::string>& newResults) {
    if (newResults.size() == 0) DGtal::trace.info() << "  All passed!";
    allResults.insert(allResults.end(), newResults.begin(), newResults.end());
    DGtal::trace.info() << std::endl;
  };

  typedef DGtal::Z2i::Point Point;
  {
    typedef DGtal::Z2i::KPreSpace KSpace;
    KSpace ks;
    KCoordsAdjuster<KSpace> kca;
    DGtalTrace::detail::CellDeltaHelper<KSpace> cdh;
    DGtal::trace.info() << "testCdhDeltaBasics<PreCell>, " << ks << std::endl;
    appendResults(testCdhDeltaBasicsImpl<KSpace::Cell>(ks, kca, cdh));
    DGtal::trace.info() << "testCdhDeltaBasics<SPreCell>, " << ks << std::endl;
    appendResults(testCdhDeltaBasicsImpl<KSpace::SCell>(ks, kca, cdh));
  }
  {
    typedef DGtal::Z2i::KSpace KSpace;
    KSpace ks;
    KCoordsAdjuster<KSpace> kca(ks);
    DGtalTrace::detail::CellDeltaHelper<KSpace> cdh(ks);

    std::vector<std::array<KSpace::Closure, KSpace::dimension>> closures = {
      {KSpace::Closure::CLOSED, KSpace::Closure::CLOSED},
      {KSpace::Closure::PERIODIC, KSpace::Closure::CLOSED},
      {KSpace::Closure::PERIODIC, KSpace::Closure::PERIODIC}
    };
    for (auto closure : closures) {
      ks.init(Point(0,0), Point(5,5), closure);
      DGtal::trace.info() << "testCdhDeltaBasics<Cell>, " << ks << std::endl;
      appendResults(testCdhDeltaBasicsImpl<KSpace::Cell>(ks, kca, cdh));
      DGtal::trace.info() << "testCdhDeltaBasics<SCell>, " << ks << std::endl;
      appendResults(testCdhDeltaBasicsImpl<KSpace::SCell>(ks, kca, cdh));
    }
  }
  return allResults;
}



}