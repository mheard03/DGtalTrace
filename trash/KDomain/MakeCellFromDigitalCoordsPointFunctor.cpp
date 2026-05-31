#pragma once
#include <concepts>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/CPointFunctor.h>
#include <DGtal/topology/CCellFunctor.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include <stdexcept>
#include "../concepts/CCell.cpp"

namespace DGtal {
namespace functors {

using namespace DGtalTrace::concepts;

template <DGtalTrace::concepts::CXXCell TCell, typename TPoint = TCell::Point>
requires DGtalTrace::concepts::CXXCell<TCell> && DGtalTrace::HasSameDimension<TPoint, TCell>
struct MakeCellFromDigitalCoordsPointFunctor {
 public:
  typedef TPoint Point;
  typedef TCell Value;
 private:
  typedef TCell Cell;
  typedef Cell::CellularGridSpace KSpace;
  typedef Cell::PreCellularGridSpace KPreSpace;

 public:
  MakeCellFromDigitalCoordsPointFunctor(const TCell& cellTemplate) requires CXPreCell<TCell> : myCellTemplate(cellTemplate) {};
  MakeCellFromDigitalCoordsPointFunctor(DGtal::ConstAlias<KSpace> ks, const TCell& cellTemplate) requires CXCell<TCell> : myKPtr(&ks), myCellTemplate(cellTemplate) {}
  
  TCell operator()(const Point &digitalCoords) const {
    if constexpr (CPreCell<TCell>) return KPreSpace::uCell(digitalCoords, myCellTemplate);
    if constexpr (CSPreCell<TCell>) return KPreSpace::sCell(digitalCoords, myCellTemplate);
    if constexpr (CCell<TCell>) return maybeThrow(myKPtr->uCell(digitalCoords, myCellTemplate));
    if constexpr (CSCell<TCell>) return maybeThrow(myKPtr->sCell(digitalCoords, myCellTemplate));
  }
    
 private:
  const KSpace* myKPtr = nullptr; 
  const TCell myCellTemplate;

  inline TCell maybeThrow(const TCell& cell) const requires CXCell<TCell> {
    if (!myKPtr->cIsInside(cell)) throw std::invalid_argument("MakeCellFromCoordsPointFunctor received a point outside the bounds of the KSpace");
    return cell;
  }
};

} // namespace functors
} // namespace DGtal