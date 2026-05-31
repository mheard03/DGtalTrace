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
struct MakeCellFromKCoordsPointFunctor {
 public:
  typedef TPoint Point;
  typedef TCell Value;
 private:
  typedef TCell Cell;
  typedef Cell::CellularGridSpace KSpace;
  typedef Cell::PreCellularGridSpace KPreSpace;
  typedef KPreSpace::Sign Sign;

 public:
  MakeCellFromKCoordsPointFunctor() requires CPreCell<TCell> {}
  MakeCellFromKCoordsPointFunctor(const Sign sign = true) requires CSPreCell<TCell> : mySign(sign) {}
  MakeCellFromKCoordsPointFunctor(DGtal::ConstAlias<KSpace> ks) requires CCell<TCell> : myKPtr(&ks) {}
  MakeCellFromKCoordsPointFunctor(DGtal::ConstAlias<KSpace> ks, const Sign sign = true) requires CSCell<TCell> : myKPtr(&ks), mySign(sign) {}
  
  TCell operator()(const Point &kCoords) const {
    if constexpr (CPreCell<TCell>) return KPreSpace::uCell(kCoords);
    if constexpr (CSPreCell<TCell>) return KPreSpace::sCell(kCoords, mySign);
    if constexpr (CXCell<TCell>) maybeThrow(kCoords);
    if constexpr (CCell<TCell>) return myKPtr->uCell(kCoords);
    if constexpr (CSCell<TCell>) return myKPtr->sCell(kCoords, mySign);
  }
    
 private:
  const KSpace* myKPtr = nullptr; 
  const Sign mySign = true;

  inline void maybeThrow(const Point& p) const requires CXCell<TCell> {
    if (!myKPtr->cIsInside(p)) throw std::invalid_argument("MakeCellFromKCoordsPointFunctor received a point outside the bounds of the KSpace");
  }
};

} // namespace functors
} // namespace DGtal