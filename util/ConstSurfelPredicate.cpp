#pragma once
#include <iostream>
#include "DGtal/base/Common.h"
#include "../concepts/CXCellularGridSpaceND.cpp"

namespace DGtalTrace {
namespace functors {

template <concepts::CXCellularGridSpaceND TKSpace, bool Value>
struct ConstSurfelPredicate {
  typedef TKSpace KSpace;
  typedef typename KSpace::Surfel Surfel;

  inline bool operator()(const Surfel& s) const {
    return Value;
  }

  void selfDisplay(std::ostream& out) const {
    out << "[ConstSurfelPredicate " << Value << "]";
  }
};

template <concepts::CXCellularGridSpaceND TKSpace>
struct TrueSurfelPredicate : ConstSurfelPredicate<TKSpace, true> {};

template <concepts::CXCellularGridSpaceND TKSpace>
struct FalseSurfelPredicate : ConstSurfelPredicate<TKSpace, false> {};

} // namespace functors
} // namespace DGtalTrace


template <DGtalTrace::concepts::CXCellularGridSpaceND TKSpace, bool Value>
inline std::ostream& operator<< (std::ostream & out, const DGtalTrace::functors::ConstSurfelPredicate<TKSpace,Value> & object) {
  object.selfDisplay ( out );
  return out;
}