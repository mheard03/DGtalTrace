#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/SCellsFunctors.h>
#include "../../concepts/CCell.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include "../CellTypeTraits.cpp"
#include "../KCoordsAdjuster.cpp"
#include "../KPreCellNavigator.cpp"
#include "../traceHelper.cpp"


typedef DGtal::Z2i::KPreSpace TKSpace;
// template<DGtalTrace::concepts::CXCellularGridSpace TKPreSpace>
std::vector<std::string> testSurfelPointelMethods(const TKSpace& ks) {
  typedef TKSpace KSpace;
  typedef DGtalTrace::KCoordsAdjuster<KSpace> KCA;
  typedef DGtalTrace::KPreCellNavigator<KSpace> KCN;
  typedef KSpace::Cell Cell;
  typedef KSpace::SCell SCell;  
  typedef typename KSpace::Point Point;

  KCA kca;
  KCN kcn;
  std::vector<std::string> errors;
  DGtal::functors::SCellToArrow<KSpace> fnArrow(ks);

  int i = -1;
  for (auto x : { 0, 3 }) {
    for (auto y : { 0, 3 }) {  
      for (auto sign : { KSpace::POS, KSpace::NEG }) {  
        const SCell surfel = kca.sCell(Point(x, y), sign);
        if (!KCA::isSurfel(surfel)) continue;

        i++;
        DGtal::Dimension dir = *KSpace::sDirs(surfel);
        Cell trueTail = kca.uCell(KSpace::sIndirectIncident(surfel, dir));
        Cell trueHead = kca.uCell(KSpace::sDirectIncident(surfel, dir));

        auto writeError = [&](std::string message, auto expected, auto actual) {
          std::stringstream err;
          err << "testSurfelPointelMethods" << std::endl;
          err << message << std::endl;
          err << "Loop " << i << ": x=" << x << ", y=" << y << ", sign =" << sign << std::endl;
          err << "  Surfel:   " << surfel << std::endl;
          err << "  Expected: " << expected << ", got " << actual << std::endl;
          DGtal::trace.info() << err.str() << std::endl;
          errors.push_back(err.str());
        };

        Cell surfelHeadResult = kcn.surfelHead(surfel);
        if (surfelHeadResult != trueHead) {
          DGtal::trace.info() << "arrow" << fnArrow(surfel) << std::endl;
          DGtal::trace.info() << "sDirect" << ks.sDirect(surfel, dir) << std::endl;
          writeError("surfelHead()", trueHead, surfelHeadResult);
        }
        Cell surfelTailResult = kcn.surfelTail(surfel);
        if (surfelTailResult != trueTail) {
          DGtal::trace.info() << "arrow" << fnArrow(surfel) << std::endl;
          DGtal::trace.info() << "sDirect" << ks.sDirect(surfel, dir) << std::endl;
          writeError("surfelTail()", trueTail, surfelTailResult);
        }
      }
    }
  }
  
  return errors;

}