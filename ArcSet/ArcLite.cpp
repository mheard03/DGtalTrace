#pragma once
#include <ranges>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include "../util/concepts/CCell.cpp"
#include "../util/concepts/dgtal/CLabel.cpp"
#include "../util/concepts/CXCellularGridSpaceND.cpp"
#include "../util/traceHelper.cpp"

namespace DGtalTrace {

template <concepts::CCellularGridSpaceND TKSpace, concepts::CLabel TValue>
class ArcLite {
  typedef TKSpace KSpace;
  typedef KSpace::Surfel Surfel;
  typedef KSpace::Point Point;
  
 private:
  Surfel firstSurfel;
  Surfel lastSurfel;
  TValue insideColor;
  TValue outsideColor;
  
  std::optional<Surfel> next;
  std::optional<Surfel> prev;
};

}