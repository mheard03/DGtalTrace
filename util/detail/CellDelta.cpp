#pragma once
#include "../../concepts/CXCellularGridSpaceND.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace>
struct CellDelta {
  typedef TKSpace KSpace;
  typedef typename KSpace::Integer Integer;
  typedef typename KSpace::Sign Sign;

  enum Magnitude : Integer {
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

  void selfDisplay(std::ostream &out) const {
    out << "[Delta " << smag() << " on dim " << dim << "]";
  }
};

} // end namespace DGtalTrace