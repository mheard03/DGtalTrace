#pragma once
#include "../traceHelper.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include <DGtal/base/Assert.h>


namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace>
struct CellDelta {
  typedef TKSpace KSpace;
  typedef typename KSpace::Integer Integer;
  typedef typename KSpace::Sign Sign;

  enum class Magnitude : Integer {
    ONE = 1,
    TWO = 2
  };
  DGtal::Dimension dim;
  Magnitude mag;
  Sign sign;

  Integer smag() const {
    if (sign) {
      return static_cast<Integer>(mag);
    }
    else {
      return -static_cast<Integer>(mag);
    }
  }

  CellDelta opp() const {
    return CellDelta<TKSpace>{dim, mag, !sign};
  }
  CellDelta toMag(Magnitude aMag) const {
    return CellDelta<TKSpace>{dim, aMag, sign};
  }

  void selfDisplay(std::ostream &out) const {
    out << "[Delta " << smag() << " on dim " << dim << "]";
  }
};

FN_STREAM_SELFDISPLAY_T(DGtalTrace::CellDelta<T>)

} // end namespace DGtalTrace